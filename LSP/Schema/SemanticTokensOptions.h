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

struct SemanticTokensOptions {
    SemanticTokensLegend legend;
    struct Range_1 {

        static Decoded<Range_1> decode(JSONValue const &json)
        {
            Range_1 ret;
            return std::move(ret);
        }

        JSONValue encode() const
        {
            JSONValue ret;
            return ret;
        };
    };
    std::optional<std::variant<bool, Range_1>> range;
    struct Full_1 {
        std::optional<bool> delta;

        static Decoded<Full_1> decode(JSONValue const &json)
        {
            Full_1 ret;
            if (json.has("delta")) {
                ret.delta = TRY_EVAL(json.try_get<bool>("delta"));
            }
            return std::move(ret);
        }

        JSONValue encode() const
        {
            JSONValue ret;
            set(ret, "delta", delta);
            return ret;
        };
    };
    std::optional<std::variant<bool, Full_1>> full;

    static Decoded<SemanticTokensOptions> decode(JSONValue const &json)
    {
        SemanticTokensOptions ret;
        ret.legend = TRY_EVAL(json.try_get<SemanticTokensLegend>("legend"));
        if (json.has("range")) {
            ret.range = TRY_EVAL(json.try_get_variant<bool, Range_1>("range"));
        }
        if (json.has("full")) {
            ret.full = TRY_EVAL(json.try_get_variant<bool, Full_1>("full"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "legend", legend);
        set(ret, "range", range);
        set(ret, "full", full);
        return ret;
    };
};

} /* namespace LSP */
