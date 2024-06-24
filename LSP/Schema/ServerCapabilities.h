/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/PositionEncodingKind.h>
#include <LSP/Schema/SemanticTokensOptions.h>
#include <LSP/Schema/TextDocumentSyncKind.h>
#include <LSP/Schema/TextDocumentSyncOptions.h>

namespace LSP {

struct ServerCapabilities : public LSPObject {
    std::optional<PositionEncodingKind>                                        positionEncoding;
    std::optional<std::variant<TextDocumentSyncOptions, TextDocumentSyncKind>> textDocumentSync;
    std::optional<SemanticTokensOptions>                                       semanticTokensProvider;

    static Result<ServerCapabilities, JSONError> decode(JSONValue const &json)
    {
        ServerCapabilities ret;
        if (json.has("positionEncoding") {
            ret.positionEncoding = TRY_EVAL(json.try_get<PositionEncodingKind>(positionEncoding));
        }
        if (json.has("textDocumentSync") {
            ret.textDocumentSync = TRY_EVAL(json.try_get<std::variant<TextDocumentSyncOptions, TextDocumentSyncKind>>(textDocumentSync));
        }
        if (json.has("semanticTokensProvider") {
            ret.semanticTokensProvider = TRY_EVAL(json.try_get<SemanticTokensOptions>(semanticTokensProvider));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "positionEncoding", encode<std::optional<PositionEncodingKind>>(positionEncoding));
        set(ret, "textDocumentSync", encode<std::optional<std::variant<TextDocumentSyncOptions, TextDocumentSyncKind>>>(textDocumentSync));
        set(ret, "semanticTokensProvider", encode<std::optional<SemanticTokensOptions>>(semanticTokensProvider));
        JSONValue ret;
    };
};

} /* namespace LSP */
