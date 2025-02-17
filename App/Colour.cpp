/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <App/Colour.h>

namespace Aragorn {

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

    auto decode_object_item = [&decode_component, &json](std::string_view const &short_tag, std::string_view const &long_tag) -> Result<uint8_t, JSONError> {
        if (json.has(long_tag)) {
            return TRY_EVAL(decode_component(json[long_tag]));
        } else if (json.has(short_tag)) {
            return TRY_EVAL(decode_component(json[short_tag]));
        }
        return (short_tag == "a") ? 0xFF : 0;
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
        return Colour { res.value() };
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
        return Colour {
            TRY_EVAL(decode_component(json[0])),
            TRY_EVAL(decode_component(json[1])),
            TRY_EVAL(decode_component(json[2])),
            TRY_EVAL(decode_component(json[3]))
        };
    }
    case JSONType::Object: {
        return Colour {
            TRY_EVAL(decode_object_item("r", "red")),
            TRY_EVAL(decode_object_item("g", "green")),
            TRY_EVAL(decode_object_item("b", "blue")),
            TRY_EVAL(decode_object_item("a", "alpha"))
        };
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

Result<Colours, JSONError> Colours::decode(JSONValue const &json)
{
    CHECK_JSON_TYPE(json, Object);
    Colour bg { 0u };
    Colour fg { 0u };
    if (json.has("background")) {
        bg = TRY_EVAL(Colour::decode(json["background"]));
    } else if (json.has("bg")) {
        bg = TRY_EVAL(Colour::decode(json["bg"]));
    }
    if (json.has("foreground")) {
        fg = TRY_EVAL(Colour::decode(json["foreground"]));
    } else if (json.has("fg")) {
        fg = TRY_EVAL(Colour::decode(json["fg"]));
    }
    return Colours { bg, fg };
}

std::string Colours::to_string() const
{
    return std::format("bg: {} fg: {}", bg().to_hex(), fg().to_hex());
}

}
