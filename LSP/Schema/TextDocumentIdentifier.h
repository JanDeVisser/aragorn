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

struct TextDocumentIdentifier : public LSPObject {
    DocumentUri uri;

    static Result<TextDocumentIdentifier, JSONError> decode(JSONValue const &json)
    {
        TextDocumentIdentifier ret;
        ret.uri = TRY_EVAL(json.try_get<DocumentUri>(uri));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "uri", encode<DocumentUri>(uri));
        JSONValue ret;
    };
};

} /* namespace LSP */
