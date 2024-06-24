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

enum class CompletionItemTag {
    Deprecated = 1,
};

template<>
inline std::optional<CompletionItemTag> from_int<CompletionItemTag>(int i)
{
    if (i == 1)
        return CompletionItemTag::Deprecated;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(CompletionItemTag const &obj)
{
    return encode_int_enum(obj);
}

template<>
inline Result<CompletionItemTag, JSONError> decode(JSONValue const &json)
{
    return decode_int_enum(json);
}

} /* namespace LibCore */
