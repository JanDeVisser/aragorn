/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/SemanticTokensClientCapabilities.h>
#include <LSP/Schema/TextDocumentSyncClientCapabilities.h>

namespace LSP {

struct TextDocumentClientCapabilities : public LSPObject {
    std::optional<TextDocumentSyncClientCapabilities> synchronization;
    std::optional<SemanticTokensClientCapabilities>   semanticTokens;

    static Result<TextDocumentClientCapabilities, JSONError> decode(JSONValue const &json)
    {
        TextDocumentClientCapabilities ret;
        if (json.has("synchronization") {
            ret.synchronization = TRY_EVAL(json.try_get<TextDocumentSyncClientCapabilities>(synchronization));
        }
        if (json.has("semanticTokens") {
            ret.semanticTokens = TRY_EVAL(json.try_get<SemanticTokensClientCapabilities>(semanticTokens));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "synchronization", encode<std::optional<TextDocumentSyncClientCapabilities>>(synchronization));
        set(ret, "semanticTokens", encode<std::optional<SemanticTokensClientCapabilities>>(semanticTokens));
        JSONValue ret;
    };
};

} /* namespace LSP */
