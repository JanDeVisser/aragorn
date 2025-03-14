/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TokenFormat.h>

namespace LSP {

struct SemanticTokensClientCapabilities {
    std::optional<bool> dynamicRegistration;
    struct Requests {
        struct Range_1 {

            static Decoded<Range_1> decode(JSONValue const &json)
            {
                Range_1 ret;
                return std::move(ret);
            }

            JSONValue encode() const
            {
                JSONValue ret { JSONType::Object };
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
                JSONValue ret { JSONType::Object };
                set(ret, "delta", delta);
                return ret;
            };
        };
        std::optional<std::variant<bool, Full_1>> full;

        static Decoded<Requests> decode(JSONValue const &json)
        {
            Requests ret;
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
            JSONValue ret { JSONType::Object };
            set(ret, "range", range);
            set(ret, "full", full);
            return ret;
        };
    };
    Requests                 requests;
    std::vector<std::string> tokenTypes;
    std::vector<std::string> tokenModifiers;
    std::vector<TokenFormat> formats;
    std::optional<bool>      overlappingTokenSupport;
    std::optional<bool>      multilineTokenSupport;
    std::optional<bool>      serverCancelSupport;
    std::optional<bool>      augmentsSyntaxTokens;

    static Decoded<SemanticTokensClientCapabilities> decode(JSONValue const &json)
    {
        SemanticTokensClientCapabilities ret;
        if (json.has("dynamicRegistration")) {
            ret.dynamicRegistration = TRY_EVAL(json.try_get<bool>("dynamicRegistration"));
        }
        ret.requests = TRY_EVAL(json.try_get<Requests>("requests"));
        ret.tokenTypes = TRY_EVAL(json.try_get_array<std::string>("tokenTypes"));
        ret.tokenModifiers = TRY_EVAL(json.try_get_array<std::string>("tokenModifiers"));
        ret.formats = TRY_EVAL(json.try_get_array<TokenFormat>("formats"));
        if (json.has("overlappingTokenSupport")) {
            ret.overlappingTokenSupport = TRY_EVAL(json.try_get<bool>("overlappingTokenSupport"));
        }
        if (json.has("multilineTokenSupport")) {
            ret.multilineTokenSupport = TRY_EVAL(json.try_get<bool>("multilineTokenSupport"));
        }
        if (json.has("serverCancelSupport")) {
            ret.serverCancelSupport = TRY_EVAL(json.try_get<bool>("serverCancelSupport"));
        }
        if (json.has("augmentsSyntaxTokens")) {
            ret.augmentsSyntaxTokens = TRY_EVAL(json.try_get<bool>("augmentsSyntaxTokens"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "dynamicRegistration", dynamicRegistration);
        set(ret, "requests", requests);
        set(ret, "tokenTypes", tokenTypes);
        set(ret, "tokenModifiers", tokenModifiers);
        set(ret, "formats", formats);
        set(ret, "overlappingTokenSupport", overlappingTokenSupport);
        set(ret, "multilineTokenSupport", multilineTokenSupport);
        set(ret, "serverCancelSupport", serverCancelSupport);
        set(ret, "augmentsSyntaxTokens", augmentsSyntaxTokens);
        return ret;
    };
};

} /* namespace LSP */
