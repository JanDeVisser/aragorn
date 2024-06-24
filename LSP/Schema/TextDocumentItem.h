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

struct TextDocumentItem : public LSPObject {
    DocumentUri uri;
    std::string languageId;
    int         version;
    std::string text;

    static Result<TextDocumentItem, JSONError> decode(JSONValue const &json)
    {
        TextDocumentItem ret;
        ret.uri = TRY_EVAL(json.try_get<DocumentUri>(uri));
        ret.languageId = TRY_EVAL(json.try_get<std::string>(languageId));
        ret.version = TRY_EVAL(json.try_get<int>(version));
        ret.text = TRY_EVAL(json.try_get<std::string>(text));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "uri", encode<DocumentUri>(uri));
        set(ret, "languageId", encode<std::string>(languageId));
        set(ret, "version", encode<int>(version));
        set(ret, "text", encode<std::string>(text));
        JSONValue ret;
    };
};

} /* namespace LSP */
