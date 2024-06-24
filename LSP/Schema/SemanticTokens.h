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

struct SemanticTokens : public LSPObject {
    std::optional<std::string> resultId;
    std::vector<uint32_t>      data;

    static Result<SemanticTokens, JSONError> decode(JSONValue const &json)
    {
        SemanticTokens ret;
        if (json.has("resultId") {
            ret.resultId = TRY_EVAL(json.try_get<std::string>(resultId));
        }
        ret.data = TRY_EVAL(json.try_get<std::vector<uint32_t>>(data));
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "resultId", encode<std::optional<std::string>>(resultId));
        set(ret, "data", encode<std::vector<uint32_t>>(data));
        JSONValue ret;
    };
};

} /* namespace LSP */
