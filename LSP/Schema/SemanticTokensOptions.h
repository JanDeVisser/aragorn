/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/SemanticTokensLegend.h>

namespace LSP {

struct SemanticTokensOptions : public LSPObject {
    SemanticTokensLegend legend;
    struct Range_1 : public LSPObject {

        static Result<Range_1, JSONError> decode(JSONValue const &json)
        {
            Range_1 ret;
            return ret;
        }

        JSONValue encode()
        {
            JSONValue ret;
        };
    };
    std::optional<std::variant<bool, Range_1>> range;
    struct Full_1 : public LSPObject {
        std::optional<bool> delta;

        static Result<Full_1, JSONError> decode(JSONValue const &json)
        {
            Full_1 ret;
        if (json.has("delta") {
                ret.delta = TRY_EVAL(json.try_get<bool>(delta));
        }
         return ret;
        }

        JSONValue encode()
        {
            set(ret, "delta", encode<std::optional<bool>>(delta));
            JSONValue ret;
        };
    };
    std::optional<std::variant<bool, Full_1>> full;

    static Result<SemanticTokensOptions, JSONError> decode(JSONValue const &json)
    {
        SemanticTokensOptions ret;
        ret.legend = TRY_EVAL(json.try_get<SemanticTokensLegend>(legend));
        if (json.has("range") {
            ret.range = TRY_EVAL(json.try_get<std::variant<bool, Range_1>>(range));
        }
        if (json.has("full") {
            ret.full = TRY_EVAL(json.try_get<std::variant<bool, Full_1>>(full));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "legend", encode<SemanticTokensLegend>(legend));
        set(ret, "range", encode<std::optional<std::variant<bool, Range_1>>>(range));
        set(ret, "full", encode<std::optional<std::variant<bool, Full_1>>>(full));
        JSONValue ret;
    };
};

} /* namespace LSP */
