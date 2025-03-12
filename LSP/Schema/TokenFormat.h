/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>

namespace LSP {

enum class TokenFormat {
    Relative,
};

inline std::string TokenFormat_as_string(TokenFormat obj)
{
    switch (obj) {
    case TokenFormat::Relative:
        return "relative";
    default:
        return "unknown";
    }
}

inline std::optional<TokenFormat> TokenFormat_from_string(std::string_view const &s)
{
    if (s == "relative")
        return TokenFormat::Relative;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(TokenFormat const &obj)
{
    return JSONValue { TokenFormat_as_string(obj) };
}

template<>
inline Decoded<TokenFormat> decode(JSONValue const &json)
{
    return TokenFormat_from_string(json.to_string());
}

} /* namespace LibCore */
