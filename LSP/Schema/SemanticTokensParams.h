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

struct SemanticTokensParams {
    TextDocumentIdentifier textDocument;

    static Decoded<SemanticTokensParams> decode(JSONValue const &json)
    {
        SemanticTokensParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<TextDocumentIdentifier>("textDocument"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "textDocument", textDocument);
        return ret;
    };
};

} /* namespace LSP */
