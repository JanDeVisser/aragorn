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

struct SemanticTokensLegend : public LSPObject {
    std::vector<std::string> tokenTypes;
    std::vector<std::string> tokenModifiers;

    static Result<SemanticTokensLegend, JSONError> decode(JSONValue const &json)
    {
        SemanticTokensLegend ret;
        ret.tokenTypes = TRY_EVAL(json.try_get<std::vector<std::string>>(tokenTypes));
        ret.tokenModifiers = TRY_EVAL(json.try_get<std::vector<std::string>>(tokenModifiers));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "tokenTypes", encode<std::vector<std::string>>(tokenTypes));
        set(ret, "tokenModifiers", encode<std::vector<std::string>>(tokenModifiers));
        JSONValue ret;
    };
};

} /* namespace LSP */
