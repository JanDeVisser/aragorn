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

struct SemanticTokensLegend {
    std::vector<std::string> tokenTypes;
    std::vector<std::string> tokenModifiers;

    static Decoded<SemanticTokensLegend> decode(JSONValue const &json)
    {
        SemanticTokensLegend ret;
        ret.tokenTypes = TRY_EVAL(json.try_get_array<std::string>("tokenTypes"));
        ret.tokenModifiers = TRY_EVAL(json.try_get_array<std::string>("tokenModifiers"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "tokenTypes", tokenTypes);
        set(ret, "tokenModifiers", tokenModifiers);
        return ret;
    };
};

} /* namespace LSP */
