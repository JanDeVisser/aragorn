/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/DocumentUri.h>
#include <LSP/Schema/LSPBase.h>

namespace LSP {

struct TextDocumentItem {
    DocumentUri uri;
    std::string languageId;
    int         version;
    std::string text;

    static Decoded<TextDocumentItem> decode(JSONValue const &json)
    {
        TextDocumentItem ret;
        ret.uri = TRY_EVAL(json.try_get<DocumentUri>("uri"));
        ret.languageId = TRY_EVAL(json.try_get<std::string>("languageId"));
        ret.version = TRY_EVAL(json.try_get<int>("version"));
        ret.text = TRY_EVAL(json.try_get<std::string>("text"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "uri", uri);
        set(ret, "languageId", languageId);
        set(ret, "version", version);
        set(ret, "text", text);
        return ret;
    };
};

} /* namespace LSP */
