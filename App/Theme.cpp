/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <filesystem>
#include <pwd.h>
#include <unistd.h>

#include <LibCore/IO.h>

#include <App/Theme.h>

namespace Eddy {

Result<Colour, Colour::ColourParseError> Colour::parse_color(std::string_view const &color)
{
    auto parse_hex_color = [color](int prefixlen, int num_components) -> Result<Colour, ColourParseError> {
        Colour ret;
        ret.color.a = 0xFF;
        char buf[3];
        buf[2] = 0;
        assert(color.length() == prefixlen + num_components * 2);
        for (size_t ix = 0; ix < num_components; ++ix) {
            assert(color[2 * ix + prefixlen] != 0);
            memcpy(buf, color.data() + prefixlen + 2 * ix, 2);
            char         *endptr;
            unsigned long c = strtoul(buf, &endptr, 16);
            if (*endptr != 0) {
                return ColourParseError { std::format("Could not parse hex color string '{}'", color) };
            }
            assert(c < 256);
            ret.components[ix] = static_cast<unsigned char>(c);
        }
        return ret;
    };

    if (color.length() == 7 && color[0] == '#') {
        return parse_hex_color(1, 3);
    }
    if (color.length() == 9 && color[0] == '#') {
        return parse_hex_color(1, 4);
    }
    if (color.length() == 10 && (color.starts_with("0x") || color.starts_with("0X"))) {
        return parse_hex_color(2, 4);
    }
    auto s = strip(color);
    if (s.starts_with("rgb(") || s.starts_with("RGB(")) {
        s = s.substr(4);
        if (!s.ends_with(")")) {
            return ColourParseError { std::format("Color string '{}' should end with a ')'", color) };
        }
        s = s.substr(0, s.length() - 1);
        auto components = split_by_whitespace(s);
        if (components.size() != 3) {
            return ColourParseError { std::format("Color string '{}' should have 3 color components", color) };
        }
        Colour ret;
        ret.color.a = 0xFF;
        for (auto ix = 0; ix < 4; ++ix) {
            auto res = string_to_integer<uint8_t>(components[ix]);
            if (!res) {
                return ColourParseError { std::format("Could not parse RGB color component '{}'", components[ix]) };
            }
            ret.components[ix] = res.value();
        }
        return ret;
    }
    return ColourParseError { std::format("Color string '{}' has an unknown format", color) };
}

Result<Colour, JSONError> Colour::decode(JSONValue const &json)
{
    auto decode_component = [](JSONValue const &comp) -> Result<uint8_t, JSONError> {
        if (!comp.is_integer()) {
            return JSONError {
                JSONError::Code::TypeMismatch,
                std::format("Cannot convert JSON value {} to Colour component", comp.to_string()),
            };
        }
        auto res = value<uint8_t>(comp);
        if (!res.has_value()) {
            return JSONError {
                JSONError::Code::TypeMismatch,
                std::format("Integer color component value '{}' out of range", comp.to_string()),
            };
        }
        return res.value();
    };

    auto decode_object_item = [&decode_component, &json](std::string_view const &short_tag, std::string_view const &long_tag, uint8_t &target) -> Error<JSONError> {
        if (json.has(long_tag)) {
            target = TRY_EVAL(decode_component(json[long_tag]));
        } else if (json.has(short_tag)) {
            target = TRY_EVAL(decode_component(json[short_tag]));
        }
        return {};
    };

    switch (json.type()) {
    case JSONType::String: {
        auto res = Colour::parse_color(json.to_string());
        if (res.is_error()) {
            return JSONError {
                JSONError::Code::TypeMismatch,
                std::format("Invalid string color value '{}': {}", json.to_string(), res.error().message),
            };
        }
        return res.value();
    } break;
    case JSONType::Integer: {
        auto res = value<uint32_t>(json);
        if (!res.has_value()) {
            return JSONError {
                JSONError::Code::TypeMismatch,
                std::format("Invalid integer color value '{}'", json.to_string()),
            };
        }
        return Colour { res.value() };
    }
    case JSONType::Array: {
        if (json.size() != 4) {
            return JSONError {
                JSONError::Code::TypeMismatch,
                "Array color value must have 4 entries",
            };
        }
        Colour ret;
        for (size_t ix = 0; ix < 4; ++ix) {
            ret.components[ix] = TRY_EVAL(decode_component(json[ix]));
        }
        return ret;
    }
    case JSONType::Object: {
        Colour ret { 0 };
        TRY(decode_object_item("r", "red", ret.color.r));
        TRY(decode_object_item("g", "green", ret.color.g));
        TRY(decode_object_item("b", "blue", ret.color.b));
        ret.color.a = 0xFF;
        TRY(decode_object_item("a", "alpha", ret.color.a));
        return ret;
    }
    default:
        return JSONError {
            JSONError::Code::TypeMismatch,
            std::format("Cannot convert JSON value of type '{}' to color value", JSONType_name(json.type())),
        };
    }
}

std::string Colour::to_rgb() const
{
    if (color.a < 0xFF) {
        auto perc = static_cast<int>(static_cast<float>(color.a) / 255.0) * 100.0;
        return std::format("rgb({} {} {} / {}%)", color.r, color.g, color.b, perc);
    }
    return std::format("rgb({} {} {})", color.r, color.g, color.b);
}

std::string Colour::to_hex() const
{
    return std::format("#{:02x}{:02x}{:02x}{:02x}", color.r, color.g, color.b, color.a);
}

std::string Colours::to_string() const
{
    return std::format("bg: {} fg: {}", bg.to_hex(), fg.to_hex());
}

Result<TokenColour, JSONError> TokenColour::decode(JSONValue const &json)
{
    CHECK_JSON_TYPE(json, Object);
    TokenColour ret;
    auto        name_maybe = json.get("name");
    if (name_maybe.has_value()) {
        CHECK_JSON_TYPE(name_maybe.value(), String);
        ret.name = name_maybe.value().to_string();
    }
    auto scope_maybe = json.get("scope");
    if (scope_maybe.has_value()) {
        auto scope = scope_maybe.value();
        if (!scope.is_string() && !scope.is_array()) {
            return JSONError {
                JSONError::Code::TypeMismatch,
                "'tokenColor' entry scope value must be an array or a string",
            };
        }
        if (ret.name.empty()) {
            if (scope.is_string()) {
                ret.name = scope.to_string();
            } else if (!scope.empty()) {
                ret.name = scope[0].to_string();
            }
        }
        if (scope.is_string()) {
            auto scopes = split(scope.to_string(), ',');
            for (auto const &s : scopes) {
                ret.scope.emplace_back(strip(s));
            }
        } else {
            for (auto const &entry : scope) {
                CHECK_JSON_TYPE(entry, String);
                ret.scope.emplace_back(entry.to_string());
            }
        }
    }
    auto settings_maybe = json.get("settings");
    if (!settings_maybe.has_value()) {
        return JSONError {
            JSONError::Code::TypeMismatch,
            "'tokenColor' entry must specify settings",
        };
    }
    auto settings = settings_maybe.value();
    CHECK_JSON_TYPE(settings, Object);
    ret.colours.fg = TRY_EVAL(Colour::decode(settings["foreground"]));
    ret.colours.bg = TRY_EVAL(Colour::decode(settings["background"]));
    return Result<TokenColour, JSONError> { ret };
}

Result<SemanticTokenColour, JSONError> SemanticTokenColour::decode(SemanticTokenTypes type, JSONValue const &json)
{
    CHECK_JSON_TYPE(json, Object);
    SemanticTokenColour ret;
    ret.token_type = type;
    ret.colours.fg = TRY_EVAL(Colour::decode(json["foreground"]));
    ret.colours.bg = TRY_EVAL(Colour::decode(json["background"]));
    return ret;
}

void Theme::get_mapping(TokenKind kind, std::string_view const &scope)
{
    auto index = index_for_scope(scope);
    if (index.has_value()) {
        token_mappings.push_back(TokenThemeMapping { kind, index.value() });
        return;
    }
}

void Theme::build_theme_index_mappings()
{
    get_mapping(TokenKind::Comment, "comment");
    get_mapping(TokenKind::Keyword, "keyword");
    get_mapping(TokenKind::Identifier, "identifier");
    get_mapping(TokenKind::Number, "constant.numeric");
    get_mapping(TokenKind::Symbol, "punctuation");
    get_mapping(TokenKind::QuotedString, "string");
    get_mapping(TokenKind::Directive, "keyword.control.directive");
    get_mapping(TokenKind::DirectiveArg, "string");
}

Result<Theme, JSONError> Theme::decode(JSONValue const &json)
{
    Theme ret;
    auto  colors = json.get_with_default("colors");
    CHECK_JSON_TYPE(colors, Object);
    ret.editor.bg = TRY_EVAL(Colour::decode(colors.get_with_default("editor.background", JSONType::Integer)));
    if (ret.editor.bg.rgba == 0) {
        ret.editor.bg = Colour { 0x000000FF };
    }
    ret.editor.fg = TRY_EVAL(Colour::decode(colors.get_with_default("editor.foreground", JSONType::Integer)));
    if (ret.editor.fg.rgba == 0) {
        ret.editor.fg = Colour { 0xFFFFFFFF };
    }
    ret.selection.bg = TRY_EVAL(Colour::decode(colors.get_with_default("editor.selectionBackground", JSONType::Integer)));
    ret.selection.fg = TRY_EVAL(Colour::decode(colors.get_with_default("editor.selectionForeground", JSONType::Integer)));
    if (ret.selection.bg.rgba == 0 && ret.selection.fg.rgba == 0) {
        ret.selection.bg = ret.editor.fg;
        ret.selection.fg = ret.editor.bg;
    }
    if (ret.selection.bg.rgba == 0) {
        ret.selection.bg = ret.editor.bg;
    }
    if (ret.selection.fg.rgba == 0) {
        ret.selection.fg = ret.editor.fg;
    }
    ret.linehighlight.bg = TRY_EVAL(Colour::decode(colors.get_with_default("editor.lineHighlightBackground", JSONType::Integer)));
    ret.linehighlight.fg = TRY_EVAL(Colour::decode(colors.get_with_default("editor.lineHighlightForeground", JSONType::Integer)));
    if (ret.linehighlight.bg.rgba == 0) {
        ret.linehighlight.bg = ret.editor.bg;
    }
    if (ret.linehighlight.fg.rgba == 0) {
        ret.linehighlight.fg = ret.editor.fg;
    }
    ret.gutter.bg = TRY_EVAL(Colour::decode(colors.get_with_default("editorGutter.background", JSONType::Integer)));
    ret.gutter.fg = TRY_EVAL(Colour::decode(colors.get_with_default("editor.activeLineNumber.foreground", JSONType::Integer)));
    if (ret.gutter.bg.rgba == 0) {
        ret.gutter.bg = ret.editor.bg;
    }
    if (ret.gutter.fg.rgba == 0) {
        ret.gutter.fg = ret.editor.fg;
    }
    auto token_colors = json.get_with_default("tokenColors", JSONType::Array);
    CHECK_JSON_TYPE(token_colors, Array);
    for (auto c : token_colors) {
        TokenColour tc = TRY_EVAL(TokenColour::decode(c));
        if (tc.colours.bg.rgba == 0) {
            tc.colours.bg = ret.editor.bg;
        }
        if (tc.colours.fg.rgba == 0) {
            tc.colours.fg = ret.editor.fg;
        }
        ret.token_colours.push_back(tc);
    }

    auto semantic_token_colors = json.get_with_default("semanticTokenColors");
    CHECK_JSON_TYPE(semantic_token_colors, Object);
    for (auto it = semantic_token_colors.obj_begin(); it != semantic_token_colors.obj_end(); ++it) {
        auto const &p = *it;
        auto        type_maybe = SemanticTokenTypes_parse(p.first);
        if (!type_maybe.has_value()) {
            continue;
        }
        auto type = type_maybe.value();
        auto semantic_token_colour = TRY_EVAL(SemanticTokenColour::decode(type, p.second));
        ret.semantic_colours.push_back(semantic_token_colour);
    }

    ret.build_theme_index_mappings();
    return ret;
}

Result<Theme, JSONError> Theme::load(std::string_view const &name)
{
    namespace fs = std::filesystem;
    Theme          ret;
    struct passwd *pw = getpwuid(getuid());
    std::string n { name };
    n += ".json";
    auto           file_name = fs::path(pw->pw_dir) / ".eddy" / "themes" / n;
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

std::optional<int> Theme::index_for_scope(std::string_view const &scope) const
{
    int    tc_match = -1;
    size_t matchlen = 0;
    for (auto tc_ix = 0; tc_ix < token_colours.size(); ++tc_ix) {
        auto &tc = token_colours[tc_ix];
        if (tc.scope.empty() && tc_match == -1) {
            assert(matchlen == 0);
            tc_match = tc_ix;
            continue;
        }
        for (auto &s : tc.scope) {
            if (scope.starts_with(s) && s.length() > matchlen) {
                tc_match = tc_ix;
                matchlen = s.length();
            }
        }
    }
    if (tc_match >= 0) {
        return tc_match;
    }
    return {};
}

std::optional<Colours> Theme::colours(Token const &t) const
{
    for (auto &mapping : token_mappings) {
        if (mapping.kind == t.kind) {
            return token_colours[mapping.theme_index].colours;
        }
    }
    return {};
}

std::optional<Colours> Theme::get_semantic_colours(int semantic_index) const
{
    for (auto &mapping : semantic_mappings) {
        if (mapping.semantic_index == semantic_index) {
            if (mapping.semantic_theme_index < 0 && mapping.token_theme_index >= 0) {
                return token_colours[mapping.token_theme_index].colours;
            }
            if (mapping.semantic_theme_index >= 0) {
                return semantic_colours[mapping.semantic_theme_index].colours;
            }
            return {};
        }
    }
    return {};
}

typedef struct {
    SemanticTokenTypes semantic_type;
    char const        *scope;
} SemanticTypeToScopeMapping;

SemanticTypeToScopeMapping semantic_scope_mapping[] = {
    { SemanticTokenTypesType, "entity.name.type" },
    { SemanticTokenTypesClass, "entity.name.type.class" },
    { SemanticTokenTypesEnum, "entity.name.type.enum" },
    { SemanticTokenTypesParameter, "variable.parameter" },
    { SemanticTokenTypesVariable, "variable.other.readwrite" },
    { SemanticTokenTypesProperty, "variable.other.property" },
    { SemanticTokenTypesEnumMember, "variable.other.enummember" },
    { SemanticTokenTypesFunction, "entity.name.function" },
    { SemanticTokenTypesMacro, "entity.name.function.preprocessor" },
    { SemanticTokenTypesKeyword, "keyword" },
    { SemanticTokenTypesComment, "comment" },
    { SemanticTokenTypesString, "string" },
    { SemanticTokenTypesNumber, "constant.numeric" },
    { SemanticTokenTypesOperator, "keyword.operator" },
};

void Theme::map_semantic_type(int semantic_index, SemanticTokenTypes type)
{
    for (auto ix = 0; auto const &colour : semantic_colours) {
        if (colour.token_type == type) {
            semantic_mappings.push_back(SemanticMapping { semantic_index, ix, -1 });
            return;
        }
        ++ix;
    }

    for (auto const&mapping : semantic_scope_mapping) {
        if (mapping.semantic_type == type) {
            auto theme_ix_maybe = index_for_scope(mapping.scope);
            int         theme_ix = -1;
            if (theme_ix_maybe.has_value()) {
                theme_ix = theme_ix_maybe.value();
            }
            semantic_mappings.push_back(SemanticMapping { semantic_index, -1, theme_ix });
            return;
        }
    }
}

}
