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

inline std::optional<TextDocumentSyncKind> TextDocumentSyncKind_from_int(int i)
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
    return JSONValue { static_cast<int>(obj) };
}

template<>
inline Decoded<TextDocumentSyncKind> decode(JSONValue const &json)
{
    int int_val;
    TRY(json.convert(int_val));
    if (auto v = TextDocumentSyncKind_from_int(int_val); !v) {
        return JSONError {
            JSONError::Code::UnexpectedValue,
            "Cannot convert JSON value of type 'TextDocumentSyncKind' to integer",
        };
    } else {
        return *v;
    }
}

} /* namespace LibCore */
