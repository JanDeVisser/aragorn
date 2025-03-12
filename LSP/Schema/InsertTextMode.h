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

enum class InsertTextMode {
    AsIs = 1,
    AdjustIndentation = 2,
};

inline std::optional<InsertTextMode> InsertTextMode_from_int(int i)
{
    if (i == 1)
        return InsertTextMode::AsIs;
    if (i == 2)
        return InsertTextMode::AdjustIndentation;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(InsertTextMode const &obj)
{
    return JSONValue { static_cast<int>(obj) };
}

template<>
inline Decoded<InsertTextMode> decode(JSONValue const &json)
{
    int int_val;
    TRY(json.convert(int_val));
    if (auto v = InsertTextMode_from_int(int_val); !v) {
        return JSONError {
            JSONError::Code::UnexpectedValue,
            "Cannot convert JSON value of type 'InsertTextMode' to integer",
        };
    } else {
        return *v;
    }
}

} /* namespace LibCore */
