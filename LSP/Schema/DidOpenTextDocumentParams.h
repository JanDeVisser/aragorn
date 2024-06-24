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

struct DidOpenTextDocumentParams : public LSPObject {
    TextDocumentItem textDocument;

    static Result<DidOpenTextDocumentParams, JSONError> decode(JSONValue const &json)
    {
        DidOpenTextDocumentParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<TextDocumentItem>(textDocument));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "textDocument", encode<TextDocumentItem>(textDocument));
        JSONValue ret;
    };
};

} /* namespace LSP */
