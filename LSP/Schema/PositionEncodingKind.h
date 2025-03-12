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

inline std::string PositionEncodingKind_as_string(PositionEncodingKind obj)
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

inline std::optional<PositionEncodingKind> PositionEncodingKind_from_string(std::string_view const &s)
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
    return JSONValue { PositionEncodingKind_as_string(obj) };
}

template<>
inline Decoded<PositionEncodingKind> decode(JSONValue const &json)
{
    return PositionEncodingKind_from_string(json.to_string());
}

} /* namespace LibCore */
