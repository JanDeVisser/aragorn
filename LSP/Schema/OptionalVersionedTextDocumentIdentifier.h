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
    std::variant<int, LSP::Null> version;

    static Result<OptionalVersionedTextDocumentIdentifier, JSONError> decode(JSONValue const &json)
    {
        OptionalVersionedTextDocumentIdentifier ret;
        ret.version = TRY_EVAL(json.try_get<std::variant<int, LSP::Null>>(version));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "version", encode<std::variant<int, LSP::Null>>(version));
        JSONValue ret;
    };
};

} /* namespace LSP */
