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

struct TextDocumentClientCapabilities {
    std::optional<TextDocumentSyncClientCapabilities> synchronization;
    std::optional<SemanticTokensClientCapabilities>   semanticTokens;

    static Decoded<TextDocumentClientCapabilities> decode(JSONValue const &json)
    {
        TextDocumentClientCapabilities ret;
        if (json.has("synchronization")) {
            ret.synchronization = TRY_EVAL(json.try_get<TextDocumentSyncClientCapabilities>("synchronization"));
        }
        if (json.has("semanticTokens")) {
            ret.semanticTokens = TRY_EVAL(json.try_get<SemanticTokensClientCapabilities>("semanticTokens"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "synchronization", synchronization);
        set(ret, "semanticTokens", semanticTokens);
        return ret;
    };
};

} /* namespace LSP */
