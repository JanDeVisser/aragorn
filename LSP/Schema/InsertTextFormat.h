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

enum class InsertTextFormat {
    PlainText = 1,
    Snippet = 2,
};

template<>
inline std::optional<InsertTextFormat> from_int<InsertTextFormat>(int i)
{
    if (i == 1)
        return InsertTextFormat::PlainText;
    if (i == 2)
        return InsertTextFormat::Snippet;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(InsertTextFormat const &obj)
{
    return encode_int_enum(obj);
}

template<>
inline Result<InsertTextFormat, JSONError> decode(JSONValue const &json)
{
    return decode_int_enum(json);
}

} /* namespace LibCore */
