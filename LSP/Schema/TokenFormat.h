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

template<>
inline std::string as_string(TokenFormat obj)
{
    switch (obj) {
    case TokenFormat::Relative:
        return "relative";
    default:
        return "unknown";
    }
}

template<>
inline std::optional<TokenFormat> from_string<TokenFormat>(std::string_view const &s)
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
    return encode_string_enum<TokenFormat>(obj);
}

template<>
inline Result<TokenFormat, JSONError> decode(JSONValue const &json)
{
    return decode_string_enum<TokenFormat>(json);
}

} /* namespace LibCore */
