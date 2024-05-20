/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <raylib.h>
#include <string>

#include <LSP/Schema/SemanticTokenTypes.h>
#include <LibCore/JSON.h>
#include <LibCore/Token.h>

namespace Eddy {

using namespace LibCore;

struct Colour {
    struct ColourParseError {
        std::string message;
    };

    union {
        Color         color;
        unsigned char components[4];
        uint32_t      rgba;
    };

    Colour()
        : rgba(0)
    {
    }

    Colour(Colour const &) = default;

    explicit Colour(uint32_t rgba)
        : rgba(rgba)
    {
    }

    Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        color.r = r;
        color.g = g;
        color.b = b;
        color.a = a;
    }

    Colour(Color c)
        : color(c)
    {
    }

    [[nodiscard]] std::string               to_rgb() const;
    [[nodiscard]] std::string               to_hex() const;
    [[nodiscard]] Color                     to_color() const { return color; }
    static Result<Colour, ColourParseError> parse_color(std::string_view const &color);
    static Result<Colour, JSONError>        decode(JSONValue const &json);
};

struct Colours {
    Colour bg { 0 };
    Colour fg { 0 };

    [[nodiscard]] std::string to_string() const;
};

struct TokenColour {
    std::string                           name;
    StringList                            scope;
    Colours                               colours;
    static Result<TokenColour, JSONError> decode(JSONValue const &json);
};

struct SemanticTokenColour {
    SemanticTokenTypes                            token_type { SemanticTokenTypesVariable };
    Colours                                       colours;
    static Result<SemanticTokenColour, JSONError> decode(SemanticTokenTypes type, JSONValue const &json);
};

struct TokenThemeMapping {
    TokenKind kind;
    //    TokenCode code;
    int theme_index;
};

struct SemanticMapping {
    int semantic_index;
    int semantic_theme_index;
    int token_theme_index;
};

class Theme {
public:
    Colours                          editor;
    Colours                          selection;
    Colours                          linehighlight;
    Colours                          gutter;
    std::vector<TokenColour>         token_colours;
    std::vector<SemanticTokenColour> semantic_colours;
    std::vector<TokenThemeMapping>   token_mappings;
    std::vector<SemanticMapping>     semantic_mappings;

    [[nodiscard]] std::optional<int>     index_for_scope(std::string_view const &scope) const;
    [[nodiscard]] std::optional<Colours> colours(Token const &t) const;
    [[nodiscard]] std::optional<Colours> get_semantic_colours(int semantic_index) const;
    void                                 map_semantic_type(int semantic_index, SemanticTokenTypes type);
    static Result<Theme, JSONError>      load(std::string_view const &name);
    static Result<Theme, JSONError>      decode(JSONValue const &json);

private:
    void get_mapping(TokenKind kind, std::string_view const &scope);
    void build_theme_index_mappings();
};

}

namespace LibCore {

using namespace Eddy;

template<>
inline Error<JSONError> decode_value(JSONValue const &json, Colour &target)
{
    target = TRY_EVAL(Colour::decode(json));
    return {};
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, TokenColour &target)
{
    target = TRY_EVAL(TokenColour::decode(json));
    return {};
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, Theme &target)
{
    target = TRY_EVAL(Theme::decode(json));
    return {};
}

}
