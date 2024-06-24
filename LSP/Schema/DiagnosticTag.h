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

template<>
inline std::optional<DiagnosticTag> from_int<DiagnosticTag>(int i)
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
    return encode_int_enum(obj);
}

template<>
inline Result<DiagnosticTag, JSONError> decode(JSONValue const &json)
{
    return decode_int_enum(json);
}

} /* namespace LibCore */
