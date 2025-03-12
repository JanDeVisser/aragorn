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

inline std::string MarkupKind_as_string(MarkupKind obj)
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

inline std::optional<MarkupKind> MarkupKind_from_string(std::string_view const &s)
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
    return JSONValue { MarkupKind_as_string(obj) };
}

template<>
inline Decoded<MarkupKind> decode(JSONValue const &json)
{
    return MarkupKind_from_string(json.to_string());
}

} /* namespace LibCore */
