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

enum class DiagnosticTag {
    Unnecessary = 1,
    Deprecated = 2,
};

inline std::optional<DiagnosticTag> DiagnosticTag_from_int(int i)
{
    if (i == 1)
        return DiagnosticTag::Unnecessary;
    if (i == 2)
        return DiagnosticTag::Deprecated;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(DiagnosticTag const &obj)
{
    return JSONValue { static_cast<int>(obj) };
}

template<>
inline Decoded<DiagnosticTag> decode(JSONValue const &json)
{
    int int_val;
    TRY(json.convert(int_val));
    if (auto v = DiagnosticTag_from_int(int_val); !v) {
        return JSONError {
            JSONError::Code::UnexpectedValue,
            "Cannot convert JSON value of type 'DiagnosticTag' to integer",
        };
    } else {
        return *v;
    }
}

} /* namespace LibCore */
