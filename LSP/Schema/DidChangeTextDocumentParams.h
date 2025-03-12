/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TextDocumentContentChangeEvent.h>
#include <LSP/Schema/VersionedTextDocumentIdentifier.h>

namespace LSP {

struct DidChangeTextDocumentParams {
    VersionedTextDocumentIdentifier             textDocument;
    std::vector<TextDocumentContentChangeEvent> contentChanges;

    static Decoded<DidChangeTextDocumentParams> decode(JSONValue const &json)
    {
        DidChangeTextDocumentParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<VersionedTextDocumentIdentifier>("textDocument"));
        ret.contentChanges = TRY_EVAL(json.try_get_array<TextDocumentContentChangeEvent>("contentChanges"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "textDocument", textDocument);
        set(ret, "contentChanges", contentChanges);
        return ret;
    };
};

} /* namespace LSP */
