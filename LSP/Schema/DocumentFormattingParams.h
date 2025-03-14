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

    static Decoded<DocumentFormattingParams> decode(JSONValue const &json)
    {
        DocumentFormattingParams ret;
        ret.textDocument = TRY_EVAL(json.try_get<TextDocumentIdentifier>("textDocument"));
        ret.options = TRY_EVAL(json.try_get<FormattingOptions>("options"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "textDocument", textDocument);
        set(ret, "options", options);
        return ret;
    };
};

} /* namespace LSP */
