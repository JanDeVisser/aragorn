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

struct OptionalVersionedTextDocumentIdentifier : public TextDocumentIdentifier {
    std::variant<int, Null> version;

    static Decoded<OptionalVersionedTextDocumentIdentifier> decode(JSONValue const &json)
    {
        OptionalVersionedTextDocumentIdentifier ret;
        ret.version = TRY_EVAL(json.try_get_variant<int, Null>("version"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "version", version);
        return ret;
    };
};

} /* namespace LSP */
