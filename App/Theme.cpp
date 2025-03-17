/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <filesystem>
#include <print>
#include <pwd.h>
#include <unistd.h>

#include <LibCore/IO.h>

#include <App/Aragorn.h>
#include <App/Theme.h>

namespace Aragorn {

using namespace LibCore;
using namespace std::literals::string_literals;

Theme &Theme::the()
{
    return Aragorn::the()->theme();
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
        if ((s == "editor.selectionBackground") || (s == "selectionHighlightBackground")) {
            ret.m_selection = Colours { colour, ret.m_selection.fg() };
        } else if (s == "editor.selectionForeground") {
            ret.m_selection = Colours { ret.m_selection.bg(), colour };
        }
    }
    if (static_cast<Color>(ret.m_selection.fg()).a == 0 && static_cast<Color>(ret.m_selection.bg()).a == 0) {
        ret.m_selection = Colours { ret.m_default_colours.fg(), ret.m_default_colours.bg() };
    } else {
        if (static_cast<Color>(ret.m_selection.fg()).a == 0) {
            ret.m_selection = Colours { ret.m_selection.bg(), ret.m_default_colours.fg() };
        }
        if (static_cast<Color>(ret.m_selection.bg()).a == 0) {
            ret.m_selection = Colours { ret.m_default_colours.bg(), ret.m_selection.fg() };
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
    auto file_name = fs::path(pw->pw_dir) / ".aragorn" / "themes" / n;
    if (!fs::exists(file_name)) {
        file_name = fs::path(ARAGORN_DATADIR) / "themes" / n;
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

Scope Theme::get_scope(SemanticTokenTypes type)
{
    if (m_semantic_token_type_to_scope_id.contains(type)) {
        return m_semantic_token_type_to_scope_id.at(type);
    }
    auto scope_name = std::format("SemanticTokenTypes.{}", SemanticTokenTypes_as_string(type));
    if (auto scope_id = get_scope(scope_name); scope_id > 0) {
        m_semantic_token_type_to_scope_id[type] = scope_id;
        return scope_id;
    }
    std::string scope;
    switch (type) {
    case SemanticTokenTypes::Type:
        scope = "entity.name.type";
        break;
    case SemanticTokenTypes::Class:
        scope = "entity.name.type.class";
        break;
    case SemanticTokenTypes::Enum:
        scope = "entity.name.type.enum";
        break;
    case SemanticTokenTypes::Parameter:
        scope = "variable.parameter";
        break;
    case SemanticTokenTypes::Variable:
        scope = "variable.other.readwrite";
        break;
    case SemanticTokenTypes::Property:
        scope = "variable.other.property";
        break;
    case SemanticTokenTypes::EnumMember:
        scope = "variable.other.enummember";
        break;
    case SemanticTokenTypes::Function:
        scope = "entity.name.function";
        break;
    case SemanticTokenTypes::Macro:
        scope = "entity.name.function.preprocessor";
        break;
    case SemanticTokenTypes::Keyword:
        scope = "keyword";
        break;
    case SemanticTokenTypes::Comment:
        scope = "comment";
        break;
    case SemanticTokenTypes::String:
        scope = "string";
        break;
    case SemanticTokenTypes::Number:
        scope = "constant.numeric";
        break;
    case SemanticTokenTypes::Operator:
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
    if (m_scope_ids.contains(name)) {
        Scope ret = m_scope_ids[name];
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
    m_scope_ids[name] = ss_match;
    return ss_match;
}

struct SemanticTypeToScopeMapping {
    SemanticTokenTypes semantic_type;
    std::string_view   scope;
};

SemanticTypeToScopeMapping semantic_scope_mapping[] = {
    { SemanticTokenTypes::Type, "entity.name.type" },
    { SemanticTokenTypes::Class, "entity.name.type.class" },
    { SemanticTokenTypes::Enum, "entity.name.type.enum" },
    { SemanticTokenTypes::Parameter, "variable.parameter" },
    { SemanticTokenTypes::Variable, "variable.other.readwrite" },
    { SemanticTokenTypes::Property, "variable.other.property" },
    { SemanticTokenTypes::EnumMember, "variable.other.enummember" },
    { SemanticTokenTypes::Function, "entity.name.function" },
    { SemanticTokenTypes::Macro, "entity.name.function.preprocessor" },
    { SemanticTokenTypes::Keyword, "keyword" },
    { SemanticTokenTypes::Comment, "comment" },
    { SemanticTokenTypes::String, "string" },
    { SemanticTokenTypes::Number, "constant.numeric" },
    { SemanticTokenTypes::Operator, "keyword.operator" },
};

void Theme::map_semantic_type(int semantic_index, SemanticTokenTypes type)
{
    for (auto ix = 0; ix < m_semantic_colours.size(); ++ix) {
        auto &semantic_colour = m_semantic_colours[ix];
        if (semantic_colour.token_type == type) {
            m_semantic_mappings.emplace_back(semantic_index, ix, -1);
            return;
        }
    }
    for (size_t ix = 0; ix < sizeof(semantic_scope_mapping) / sizeof(SemanticTypeToScopeMapping); ++ix) {
        if (semantic_scope_mapping[ix].semantic_type == type) {
            auto theme_ix = get_scope(semantic_scope_mapping[ix].scope);
            m_semantic_mappings.emplace_back(semantic_index, -1, theme_ix);
            return;
        }
    }
    std::println("SemanticTokenType {} = '{}' not mapped", semantic_index, SemanticTokenTypes_as_string(type));
}

}
