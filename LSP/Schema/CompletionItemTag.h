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

inline std::optional<CompletionItemTag> CompletionItemTag_from_int(int i)
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
    return JSONValue { static_cast<int>(obj) };
}

template<>
inline Decoded<CompletionItemTag> decode(JSONValue const &json)
{
    int int_val;
    TRY(json.convert(int_val));
    if (auto v = CompletionItemTag_from_int(int_val); !v) {
        return JSONError {
            JSONError::Code::UnexpectedValue,
            "Cannot convert JSON value of type 'CompletionItemTag' to integer",
        };
    } else {
        return *v;
    }
}

} /* namespace LibCore */
