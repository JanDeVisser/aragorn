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

struct SemanticTokens {
    std::optional<std::string> resultId;
    std::vector<uint32_t>      data;

    static Decoded<SemanticTokens> decode(JSONValue const &json)
    {
        SemanticTokens ret;
        if (json.has("resultId")) {
            ret.resultId = TRY_EVAL(json.try_get<std::string>("resultId"));
        }
        ret.data = TRY_EVAL(json.try_get_array<uint32_t>("data"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "resultId", resultId);
        set(ret, "data", data);
        return ret;
    };
};

} /* namespace LSP */
