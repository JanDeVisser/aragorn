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

struct ServerCapabilities {
    std::optional<PositionEncodingKind>                                        positionEncoding;
    std::optional<std::variant<TextDocumentSyncOptions, TextDocumentSyncKind>> textDocumentSync;
    std::optional<SemanticTokensOptions>                                       semanticTokensProvider;

    static Decoded<ServerCapabilities> decode(JSONValue const &json)
    {
        ServerCapabilities ret;
        if (json.has("positionEncoding")) {
            ret.positionEncoding = TRY_EVAL(json.try_get<PositionEncodingKind>("positionEncoding"));
        }
        if (json.has("textDocumentSync")) {
            ret.textDocumentSync = TRY_EVAL(json.try_get_variant<TextDocumentSyncOptions, TextDocumentSyncKind>("textDocumentSync"));
        }
        if (json.has("semanticTokensProvider")) {
            ret.semanticTokensProvider = TRY_EVAL(json.try_get<SemanticTokensOptions>("semanticTokensProvider"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "positionEncoding", positionEncoding);
        set(ret, "textDocumentSync", textDocumentSync);
        set(ret, "semanticTokensProvider", semanticTokensProvider);
        return ret;
    };
};

} /* namespace LSP */
