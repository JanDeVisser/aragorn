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

enum class DiagnosticSeverity {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4,
};

inline std::optional<DiagnosticSeverity> DiagnosticSeverity_from_int(int i)
{
    if (i == 1)
        return DiagnosticSeverity::Error;
    if (i == 2)
        return DiagnosticSeverity::Warning;
    if (i == 3)
        return DiagnosticSeverity::Information;
    if (i == 4)
        return DiagnosticSeverity::Hint;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(DiagnosticSeverity const &obj)
{
    return JSONValue { static_cast<int>(obj) };
}

template<>
inline Decoded<DiagnosticSeverity> decode(JSONValue const &json)
{
    int int_val;
    TRY(json.convert(int_val));
    if (auto v = DiagnosticSeverity_from_int(int_val); !v) {
        return JSONError {
            JSONError::Code::UnexpectedValue,
            "Cannot convert JSON value of type 'DiagnosticSeverity' to integer",
        };
    } else {
        return *v;
    }
}

} /* namespace LibCore */
