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

struct PublishDiagnosticsParams {
    DocumentUri             uri;
    std::optional<int>      version;
    std::vector<Diagnostic> diagnostics;

    static Decoded<PublishDiagnosticsParams> decode(JSONValue const &json)
    {
        PublishDiagnosticsParams ret;
        ret.uri = TRY_EVAL(json.try_get<DocumentUri>("uri"));
        if (json.has("version")) {
            ret.version = TRY_EVAL(json.try_get<int>("version"));
        }
        ret.diagnostics = TRY_EVAL(json.try_get_array<Diagnostic>("diagnostics"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "uri", uri);
        set(ret, "version", version);
        set(ret, "diagnostics", diagnostics);
        return ret;
    };
};

} /* namespace LSP */
