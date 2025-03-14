/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/CompletionContext.h>
#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TextDocumentPositionParams.h>
#include <LSP/Schema/WorkDoneProgressParams.h>

namespace LSP {

struct CompletionParams : public TextDocumentPositionParams
    , public WorkDoneProgressParams {
    std::optional<CompletionContext> context;

    static Decoded<CompletionParams> decode(JSONValue const &json)
    {
        CompletionParams ret;
        if (json.has("context")) {
            ret.context = TRY_EVAL(json.try_get<CompletionContext>("context"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "context", context);
        return ret;
    };
};

} /* namespace LSP */
