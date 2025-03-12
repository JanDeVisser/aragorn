/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TextDocumentItem.h>

namespace LSP {

struct DidOpenTextDocumentParams {
    TextDocumentItem textDocument;

    static Decoded<DidOpenTextDocumentParams> decode(JSONValue const &json)
    {
        DidOpenTextDocumentParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<TextDocumentItem>("textDocument"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "textDocument", textDocument);
        return ret;
    };
};

} /* namespace LSP */
