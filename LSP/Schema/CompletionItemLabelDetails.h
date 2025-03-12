/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>

namespace LSP {

struct CompletionItemLabelDetails {
    std::optional<std::string> detail;
    std::optional<std::string> description;

    static Decoded<CompletionItemLabelDetails> decode(JSONValue const &json)
    {
        CompletionItemLabelDetails ret;
        if (json.has("detail")) {
            ret.detail = TRY_EVAL(json.try_get<std::string>("detail"));
        }
        if (json.has("description")) {
            ret.description = TRY_EVAL(json.try_get<std::string>("description"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "detail", detail);
        set(ret, "description", description);
        return ret;
    };
};

} /* namespace LSP */
