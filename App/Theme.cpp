/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <filesystem>
#include <pwd.h>
#include <unistd.h>

#include <LibCore/IO.h>

#include <App/Eddy.h>
#include <App/Theme.h>

namespace Eddy {

using namespace LibCore;

Theme& Theme::the()
{
    return Eddy::the()->theme();
}

Result<Theme, JSONError> Theme::decode(JSONValue const &json)
{
    Theme ret;

    auto add_scope = [&ret](std::string_view const &scope, Colours colours) {
        ret.m_colours.emplace_back(scope, colours);
        ret.m_scope_ids[std::string { scope }] = ret.m_colours.size() - 1;
    };

    auto token_color_decode = [&ret, &add_scope](JSONValue const &json) -> Error<JSONError> {
        CHECK_JSON_TYPE(json, Object);
        auto settings_maybe = json.get("settings");
        if (!settings_maybe.has_value()) {
            return JSONError {
                JSONError::Code::TypeMismatch,
                "'tokenColor' entry must specify settings",
            };
        }
        auto settings = settings_maybe.value();
        CHECK_JSON_TYPE(settings, Object);
        auto colours = TRY_EVAL(Colours::decode(settings));

        auto scope_maybe = json.get("scope");
        if (scope_maybe.has_value()) {
            auto scope = scope_maybe.value();
            if (!scope.is_string() && !scope.is_array()) {
                return JSONError {
                    JSONError::Code::TypeMismatch,
                    "'tokenColor' entry scope value must be an array or a string",
                };
            }
            if (scope.is_string()) {
                auto scopes = split(scope.to_string(), ',');
                for (auto const &s : scopes) {
                    add_scope(strip(s), colours);
                }
            } else {
                for (auto const &entry : scope) {
                    CHECK_JSON_TYPE(entry, String);
                    add_scope(entry.to_string(), colours);
                }
            }
        }
        return {};
    };

    auto colors = json.get_with_default("colors");
    CHECK_JSON_TYPE(colors, Object);
    for (auto it = colors.obj_begin(); it != colors.obj_end(); ++it) {
        auto       &entry = *it;
        auto const &s = entry.first;
        auto const &c = entry.second;
        auto        colour = TRY_EVAL(Colour::decode(c));
        add_scope(s, Colours { 0u, (uint32_t) colour });
        if (s == "editor.background") {
            ret.m_default_colours = Colours { colour, ret.m_default_colours.fg() };
        } else if (s == "editor.foreground") {
            ret.m_default_colours = Colours { ret.m_default_colours.bg(), colour };
        }
        if (s == "editor.selectionBackground") {
            ret.m_selection = Colours { colour, ret.m_selection.fg() };
        } else if (s == "editor.selectionForeground") {
            ret.m_selection = Colours { ret.m_selection.bg(), colour };
        }
    }
    add_scope("", ret.m_default_colours);

    auto token_colors = json.get_with_default("tokenColors", JSONType::Array);
    CHECK_JSON_TYPE(token_colors, Array);
    for (auto c : token_colors) {
        TRY(token_color_decode(c));
    }

    auto semantic_token_colors = json.get_with_default("semanticTokenColors");
    CHECK_JSON_TYPE(semantic_token_colors, Object);
    for (auto it = semantic_token_colors.obj_begin(); it != semantic_token_colors.obj_end(); ++it) {
        auto const &p = *it;
        auto        scope = std::format("semanticTokenColors.{}", p.first);
        auto        colours = TRY_EVAL(Colours::decode(p.second));
        add_scope(scope, colours);
    }
    return ret;
}

Result<Theme, JSONError> Theme::load(std::string_view const &name)
{
    namespace fs = std::filesystem;
    Theme          ret;
    struct passwd *pw = getpwuid(getuid());
    std::string    n { name };
    n += ".json";
    auto file_name = fs::path(pw->pw_dir) / ".eddy" / "themes" / n;
    if (!fs::exists(file_name)) {
        file_name = fs::path(EDDY_DATADIR) / "themes" / n;
        if (!fs::exists(file_name)) {
            return JSONError {
                JSONError::Code::ProtocolError,
                std::format("Theme file '{}' not found", n),
            };
        }
    }
    auto json_maybe = JSONValue::read_file(file_name.string());
    if (json_maybe.is_error()) {
        if (std::holds_alternative<LibCError>(json_maybe.error())) {
            return JSONError {
                JSONError::Code::ProtocolError,
                std::get<LibCError>(json_maybe.error()).description,
            };
        } else {
            return std::get<JSONError>(json_maybe.error());
        }
    }
    return TRY_EVAL(Theme::decode(json_maybe.value()));
}

Scope Theme::get_scope(TokenKind kind)
{
    if (m_token_kind_to_scope_id.contains(kind)) {
        return m_token_kind_to_scope_id.at(kind);
    }
    trace(THEME, "get_scope(TokenKind::{})", TokenKind_name(kind));
    std::string_view scope;
    switch (kind) {
    case TokenKind::Comment:
        scope = "comment";
        break;
    case TokenKind::Keyword:
        scope = "keyword";
        break;
    case TokenKind::Identifier:
        scope = "identifier";
        break;
    case TokenKind::Number:
        scope = "constant.numeric";
        break;
    case TokenKind::Symbol:
        scope = "punctuation";
        break;
    case TokenKind::QuotedString:
        scope = "string";
        break;
    case TokenKind::Directive:
        scope = "keyword.control.directive";
        break;
    case TokenKind::DirectiveArg:
        scope = "string";
        break;
    default:
        scope = "identifier";
        break;
    }
    trace(THEME, "get_scope(TokenKind::{}) -> {}", TokenKind_name(kind), scope);
    auto scope_id = get_scope(scope);
    m_token_kind_to_scope_id[kind] = scope_id;
    return scope_id;
}

Scope Theme::get_scope(SemanticTokenTypes type)
{
    if (m_semantic_token_type_to_scope_id.contains(type)) {
        return m_semantic_token_type_to_scope_id.at(type);
    }
    auto scope_name = std::format("SemanticTokenTypes.{}", SemanticTokenTypes_to_string(type));
    if (auto scope_id = get_scope(scope_name); scope_id > 0) {
        m_semantic_token_type_to_scope_id[type] = scope_id;
	return scope_id;
    }
    std::string scope;
    switch (type) {
    case SemanticTokenTypesType:
        scope = "entity.name.type";
        break;
    case SemanticTokenTypesClass:
        scope = "entity.name.type.class";
        break;
    case SemanticTokenTypesEnum:
        scope = "entity.name.type.enum";
        break;
    case SemanticTokenTypesParameter:
        scope = "variable.parameter";
        break;
    case SemanticTokenTypesVariable:
        scope = "variable.other.readwrite";
        break;
    case SemanticTokenTypesProperty:
        scope = "variable.other.property";
        break;
    case SemanticTokenTypesEnumMember:
        scope = "variable.other.enummember";
        break;
    case SemanticTokenTypesFunction:
        scope = "entity.name.function";
        break;
    case SemanticTokenTypesMacro:
        scope = "entity.name.function.preprocessor";
        break;
    case SemanticTokenTypesKeyword:
        scope = "keyword";
        break;
    case SemanticTokenTypesComment:
        scope = "comment";
        break;
    case SemanticTokenTypesString:
        scope = "string";
        break;
    case SemanticTokenTypesNumber:
        scope = "constant.numeric";
        break;
    case SemanticTokenTypesOperator:
        scope = "keyword.operator";
        break;
    default:
        scope = "identifier";
        break;
    }
    auto scope_id = get_scope(scope);
    m_colours.emplace_back(scope_name, m_colours[scope_id].colours);
    scope_id = m_colours.size() - 1;
    m_scope_ids[scope_name] = scope_id;
    m_semantic_token_type_to_scope_id[type] = scope_id;
    return scope_id;
}

Scope Theme::get_scope(std::string_view const &name)
{
    std::string n { name };
    if (m_scope_ids.contains(n)) {
	Scope ret = m_scope_ids[n];
	trace(THEME, "get_scope({}) mapped to scope_id {}: {}", name, ret, m_colours[ret].colours.to_string());
        return ret;
    }

    trace(THEME, "get_scope({})", name);
    int    ss_match = -1;
    size_t matchlen = 0;
    for (auto ss_ix = 0; ss_ix < m_colours.size(); ++ss_ix) {
        auto &ss = m_colours[ss_ix];
        if (ss.scope.empty() && ss_match < 0) {
            ss_match = ss_ix;
	    continue;
	}
        if (name.starts_with(ss.scope) && ss.scope.length() > matchlen) {
	    trace(THEME, "match: {}", ss.scope);
            ss_match = ss_ix;
            matchlen = ss.scope.length();
        }
    }
    assert(ss_match >= 0);
    trace(THEME, "get_scope({}) matches '{}' ({}): {}", name, m_colours[ss_match].scope, ss_match, m_colours[ss_match].colours.to_string());
    m_scope_ids[n] = ss_match;
    return ss_match;
}

}

