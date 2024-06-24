/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/Position.h>
#include <LSP/Schema/TextDocumentIdentifier.h>

namespace LSP {

struct TextDocumentPositionParams : public LSPObject {
    TextDocumentIdentifier textDocument;
    Position               position;

    static Result<TextDocumentPositionParams, JSONError> decode(JSONValue const &json)
    {
        TextDocumentPositionParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<TextDocumentIdentifier>(textDocument));
        ret.position = TRY_EVAL(json.try_get<Position>(position));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "textDocument", encode<TextDocumentIdentifier>(textDocument));
        set(ret, "position", encode<Position>(position));
        JSONValue ret;
    };
};

} /* namespace LSP */
