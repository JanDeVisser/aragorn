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

struct SemanticTokensClientCapabilities : public LSPObject {
    std::optional<bool> dynamicRegistration;
    struct Requests : public LSPObject {
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

        static Result<Requests, JSONError> decode(JSONValue const &json)
        {
            Requests ret;
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
            set(ret, "range", encode<std::optional<std::variant<bool, Range_1>>>(range));
            set(ret, "full", encode<std::optional<std::variant<bool, Full_1>>>(full));
            JSONValue ret;
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

    static Result<SemanticTokensClientCapabilities, JSONError> decode(JSONValue const &json)
    {
        SemanticTokensClientCapabilities ret;
        if (json.has("dynamicRegistration") {
            ret.dynamicRegistration = TRY_EVAL(json.try_get<bool>(dynamicRegistration));
        }
        ret.requests = TRY_EVAL(json.try_get<Requests>(requests));
        ret.tokenTypes = TRY_EVAL(json.try_get<std::vector<std::string>>(tokenTypes));
        ret.tokenModifiers = TRY_EVAL(json.try_get<std::vector<std::string>>(tokenModifiers));
        ret.formats = TRY_EVAL(json.try_get<std::vector<TokenFormat>>(formats));
        if (json.has("overlappingTokenSupport") {
            ret.overlappingTokenSupport = TRY_EVAL(json.try_get<bool>(overlappingTokenSupport));
        }
        if (json.has("multilineTokenSupport") {
            ret.multilineTokenSupport = TRY_EVAL(json.try_get<bool>(multilineTokenSupport));
        }
        if (json.has("serverCancelSupport") {
            ret.serverCancelSupport = TRY_EVAL(json.try_get<bool>(serverCancelSupport));
        }
        if (json.has("augmentsSyntaxTokens") {
            ret.augmentsSyntaxTokens = TRY_EVAL(json.try_get<bool>(augmentsSyntaxTokens));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "dynamicRegistration", encode<std::optional<bool>>(dynamicRegistration));
        set(ret, "requests", encode<Requests>(requests));
        set(ret, "tokenTypes", encode<std::vector<std::string>>(tokenTypes));
        set(ret, "tokenModifiers", encode<std::vector<std::string>>(tokenModifiers));
        set(ret, "formats", encode<std::vector<TokenFormat>>(formats));
        set(ret, "overlappingTokenSupport", encode<std::optional<bool>>(overlappingTokenSupport));
        set(ret, "multilineTokenSupport", encode<std::optional<bool>>(multilineTokenSupport));
        set(ret, "serverCancelSupport", encode<std::optional<bool>>(serverCancelSupport));
        set(ret, "augmentsSyntaxTokens", encode<std::optional<bool>>(augmentsSyntaxTokens));
        JSONValue ret;
    };
};

} /* namespace LSP */
