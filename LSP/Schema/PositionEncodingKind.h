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

enum class PositionEncodingKind {
    UTF8,
    UTF16,
    UTF32,
};

template<>
inline std::string as_string(PositionEncodingKind obj)
{
    switch (obj) {
    case PositionEncodingKind::UTF8:
        return "ositionEncodingKin";
    case PositionEncodingKind::UTF16:
        return "ositionEncodingKin";
    case PositionEncodingKind::UTF32:
        return "ositionEncodingKin";
    default:
        return "unknown";
    }
}

template<>
inline std::optional<PositionEncodingKind> from_string<PositionEncodingKind>(std::string_view const &s)
{
    if (s == "ositionEncodingKin")
        return PositionEncodingKind::UTF8;
    if (s == "ositionEncodingKin")
        return PositionEncodingKind::UTF16;
    if (s == "ositionEncodingKin")
        return PositionEncodingKind::UTF32;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(PositionEncodingKind const &obj)
{
    return encode_string_enum<PositionEncodingKind>(obj);
}

template<>
inline Result<PositionEncodingKind, JSONError> decode(JSONValue const &json)
{
    return decode_string_enum<PositionEncodingKind>(json);
}

} /* namespace LibCore */
