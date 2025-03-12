/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TextDocumentClientCapabilities.h>

namespace LSP {

struct ClientCapabilities {
    std::optional<TextDocumentClientCapabilities> textDocument;

    static Decoded<ClientCapabilities> decode(JSONValue const &json)
    {
        ClientCapabilities ret;
        if (json.has("textDocument")) {
            ret.textDocument = TRY_EVAL(json.try_get<TextDocumentClientCapabilities>("textDocument"));
        }
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
