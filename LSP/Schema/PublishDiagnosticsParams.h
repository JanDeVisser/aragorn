/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/Diagnostic.h>
#include <LSP/Schema/DocumentUri.h>
#include <LSP/Schema/LSPBase.h>

namespace LSP {

struct PublishDiagnosticsParams : public LSPObject {
    DocumentUri             uri;
    std::optional<int>      version;
    std::vector<Diagnostic> diagnostics;

    static Result<PublishDiagnosticsParams, JSONError> decode(JSONValue const &json)
    {
        PublishDiagnosticsParams ret;
        ret.uri = TRY_EVAL(json.try_get<DocumentUri>(uri));
        if (json.has("version") {
            ret.version = TRY_EVAL(json.try_get<int>(version));
        }
        ret.diagnostics = TRY_EVAL(json.try_get<std::vector<Diagnostic>>(diagnostics));
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "uri", encode<DocumentUri>(uri));
        set(ret, "version", encode<std::optional<int>>(version));
        set(ret, "diagnostics", encode<std::vector<Diagnostic>>(diagnostics));
        JSONValue ret;
    };
};

} /* namespace LSP */
