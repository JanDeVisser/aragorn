/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TextDocumentIdentifier.h>

namespace LSP {

struct DidCloseTextDocumentParams {
    TextDocumentIdentifier textDocument;

    static Decoded<DidCloseTextDocumentParams> decode(JSONValue const &json)
    {
        DidCloseTextDocumentParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<TextDocumentIdentifier>("textDocument"));
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
