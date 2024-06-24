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

template<>
inline std::optional<DiagnosticSeverity> from_int<DiagnosticSeverity>(int i)
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
    return encode_int_enum(obj);
}

template<>
inline Result<DiagnosticSeverity, JSONError> decode(JSONValue const &json)
{
    return decode_int_enum(json);
}

} /* namespace LibCore */
