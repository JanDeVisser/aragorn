/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TextDocumentIdentifier.h>

namespace LSP {

struct VersionedTextDocumentIdentifier : public TextDocumentIdentifier {
    int version;

    static Result<VersionedTextDocumentIdentifier, JSONError> decode(JSONValue const &json)
    {
        VersionedTextDocumentIdentifier ret;
        ret.version = TRY_EVAL(json.try_get<int>(version));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "version", encode<int>(version));
        JSONValue ret;
    };
};

} /* namespace LSP */
