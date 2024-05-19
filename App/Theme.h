/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <raylib.h>
#include <string>

#include <LibCore/Logging.h>
#include <LibCore/Token.h>
#include <LSP/Schema/SemanticTokenTypes.h>

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

    [[nodiscard]] std::string to_rgb() const;
    [[nodiscard]] std::string to_hex() const;
    [[nodiscard]] Color to_color() const { return color; }
    static Result<Colour,ColourParseError> Colour::parse_color(std::string_view const& color);
};

struct Colours {
    Colour bg;
    Colour fg;

    [[nodiscard]] std::string to_string() const;
};

struct TokenColour {
    std::string name;
    StringList  scope;
    Colours     colours;
};

struct SemanticTokenColour {
    SemanticTokenTypes token_type;
    Colours            colours;
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

struct Theme {
    Colours              editor;
    Colours              selection;
    Colours              linehighlight;
    Colours              gutter;
    std::vector<TokenColour>         token_colours;
    // SemanticTokenColours semantic_colours;
    std::vector<TokenThemeMapping>   token_mappings;
    std::vector<SemanticMapping>     semantic_mappings;

    static Result<Theme>  load(std::string_view const& name);
    [[nodiscard]] std::optional<int>     index_for_scope(std::string_view const& scope) const;
    [[nodiscard]] std::optional<Colours> colours(Token const& t) const;
    [[nodiscard]] std::optional<Colours> semantic_colours(int semantic_index) const;
    // void [[nodiscard]] map_semantic_type(int semantic_index, SemanticTokenTypes type) const;
};

}
