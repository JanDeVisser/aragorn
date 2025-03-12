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

inline std::optional<InsertTextFormat> InsertTextFormat_from_int(int i)
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
    return JSONValue { static_cast<int>(obj) };
}

template<>
inline Decoded<InsertTextFormat> decode(JSONValue const &json)
{
    int int_val;
    TRY(json.convert(int_val));
    if (auto v = InsertTextFormat_from_int(int_val); !v) {
        return JSONError {
            JSONError::Code::UnexpectedValue,
            "Cannot convert JSON value of type 'InsertTextFormat' to integer",
        };
    } else {
        return *v;
    }
}

} /* namespace LibCore */
