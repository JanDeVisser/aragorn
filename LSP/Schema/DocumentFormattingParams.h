/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/FormattingOptions.h>
#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TextDocumentIdentifier.h>
#include <LSP/Schema/WorkDoneProgressParams.h>

namespace LSP {

struct DocumentFormattingParams : public WorkDoneProgressParams {
    TextDocumentIdentifier textDocument;
    FormattingOptions      options;

    static Result<DocumentFormattingParams, JSONError> decode(JSONValue const &json)
    {
        DocumentFormattingParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<TextDocumentIdentifier>(textDocument));
        ret.options = TRY_EVAL(json.try_get<FormattingOptions>(options));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "textDocument", encode<TextDocumentIdentifier>(textDocument));
        set(ret, "options", encode<FormattingOptions>(options));
        JSONValue ret;
    };
};

} /* namespace LSP */
