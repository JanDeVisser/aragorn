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

    static Decoded<VersionedTextDocumentIdentifier> decode(JSONValue const &json)
    {
        VersionedTextDocumentIdentifier ret;
        ret.version = TRY_EVAL(json.try_get<int>("version"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "version", version);
        return ret;
    };
};

} /* namespace LSP */
