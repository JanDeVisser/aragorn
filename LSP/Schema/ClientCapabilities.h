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

struct ClientCapabilities : public LSPObject {
    std::optional<TextDocumentClientCapabilities> textDocument;

    static Result<ClientCapabilities, JSONError> decode(JSONValue const &json)
    {
        ClientCapabilities ret;
        if (json.has("textDocument") {
            ret.textDocument = TRY_EVAL(json.try_get<TextDocumentClientCapabilities>(textDocument));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "textDocument", encode<std::optional<TextDocumentClientCapabilities>>(textDocument));
        JSONValue ret;
    };
};

} /* namespace LSP */
