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

struct TextDocumentIdentifier {
    DocumentUri uri;

    static Decoded<TextDocumentIdentifier> decode(JSONValue const &json)
    {
        TextDocumentIdentifier ret;
        ret.uri = TRY_EVAL(json.try_get<DocumentUri>("uri"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "uri", uri);
        return ret;
    };
};

} /* namespace LSP */
