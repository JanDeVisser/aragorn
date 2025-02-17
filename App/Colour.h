/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <raylib.h>
#include <string>

#include <LibCore/JSON.h>
#include <LibCore/Result.h>

namespace Aragorn {

using namespace LibCore;

class Colour {
public:
    struct ColourParseError {
        std::string message;
    };

    Colour(Colour const &) = default;
    Colour &operator=(Colour const &other) = default;

    explicit Colour(uint32_t rgba = 0x000000FF)
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

    Colour(unsigned char const components[4])
    {
        color.r = components[0];
        color.g = components[1];
        color.b = components[2];
        color.a = components[3];
    }

    operator Color() const { return color; }
    operator uint32_t() const { return rgba; }

    [[nodiscard]] std::string               to_rgb() const;
    [[nodiscard]] std::string               to_hex() const;
    static Result<Colour, ColourParseError> parse_color(std::string_view const &color);
    static Result<Colour, JSONError>        decode(JSONValue const &json);

private:
    union {
        Color         color;
        unsigned char components[4];
        uint32_t      rgba;
    };
};

class Colours {
public:
    explicit Colours(uint32_t bg = 0x000000FF, uint32_t fg = 0xFFFFFFFF)
        : m_bg(bg)
        , m_fg(fg)
    {
    }

    Colours(Colour bg, Colour fg)
        : m_bg(bg)
        , m_fg(fg)
    {
    }

    Colours(Colours const &) = default;
    static Result<Colours, JSONError> decode(JSONValue const &json);

    [[nodiscard]] Colour      bg() const { return m_bg; }
    [[nodiscard]] Colour      fg() const { return m_fg; }
    [[nodiscard]] std::string to_string() const;

private:
    Colour m_bg { 0u };
    Colour m_fg { 0u };
};

}
