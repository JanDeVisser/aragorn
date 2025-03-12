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

struct DidSaveTextDocumentParams {
    TextDocumentIdentifier     textDocument;
    std::optional<std::string> text;

    static Decoded<DidSaveTextDocumentParams> decode(JSONValue const &json)
    {
        DidSaveTextDocumentParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<TextDocumentIdentifier>("textDocument"));
        if (json.has("text")) {
            ret.text = TRY_EVAL(json.try_get<std::string>("text"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "textDocument", textDocument);
        set(ret, "text", text);
        return ret;
    };
};

} /* namespace LSP */
