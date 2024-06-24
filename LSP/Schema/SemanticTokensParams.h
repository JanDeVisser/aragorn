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

struct SemanticTokensParams : public LSPObject {
    TextDocumentIdentifier textDocument;

    static Result<SemanticTokensParams, JSONError> decode(JSONValue const &json)
    {
        SemanticTokensParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<TextDocumentIdentifier>(textDocument));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "textDocument", encode<TextDocumentIdentifier>(textDocument));
        JSONValue ret;
    };
};

} /* namespace LSP */
