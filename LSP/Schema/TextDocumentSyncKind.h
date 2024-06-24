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

enum class TextDocumentSyncKind {
    None = 0,
    Full = 1,
    Incremental = 2,
};

template<>
inline std::optional<TextDocumentSyncKind> from_int<TextDocumentSyncKind>(int i)
{
    if (i == 0)
        return TextDocumentSyncKind::None;
    if (i == 1)
        return TextDocumentSyncKind::Full;
    if (i == 2)
        return TextDocumentSyncKind::Incremental;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(TextDocumentSyncKind const &obj)
{
    return encode_int_enum(obj);
}

template<>
inline Result<TextDocumentSyncKind, JSONError> decode(JSONValue const &json)
{
    return decode_int_enum(json);
}

} /* namespace LibCore */
