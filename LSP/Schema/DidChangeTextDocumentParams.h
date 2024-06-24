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

struct DidChangeTextDocumentParams : public LSPObject {
    VersionedTextDocumentIdentifier             textDocument;
    std::vector<TextDocumentContentChangeEvent> contentChanges;

    static Result<DidChangeTextDocumentParams, JSONError> decode(JSONValue const &json)
    {
        DidChangeTextDocumentParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<VersionedTextDocumentIdentifier>(textDocument));
        ret.contentChanges = TRY_EVAL(json.try_get<std::vector<TextDocumentContentChangeEvent>>(contentChanges));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "textDocument", encode<VersionedTextDocumentIdentifier>(textDocument));
        set(ret, "contentChanges", encode<std::vector<TextDocumentContentChangeEvent>>(contentChanges));
        JSONValue ret;
    };
};

} /* namespace LSP */
