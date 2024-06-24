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

enum class MarkupKind {
    PlainText,
    Markdown,
};

template<>
inline std::string as_string(MarkupKind obj)
{
    switch (obj) {
    case MarkupKind::PlainText:
        return "arkupKin";
    case MarkupKind::Markdown:
        return "arkupKin";
    default:
        return "unknown";
    }
}

template<>
inline std::optional<MarkupKind> from_string<MarkupKind>(std::string_view const &s)
{
    if (s == "arkupKin")
        return MarkupKind::PlainText;
    if (s == "arkupKin")
        return MarkupKind::Markdown;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(MarkupKind const &obj)
{
    return encode_string_enum<MarkupKind>(obj);
}

template<>
inline Result<MarkupKind, JSONError> decode(JSONValue const &json)
{
    return decode_string_enum<MarkupKind>(json);
}

} /* namespace LibCore */
