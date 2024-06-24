/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/ServerCapabilities.h>

namespace LSP {

struct InitializeResult : public LSPObject {
    ServerCapabilities capabilities;
    struct ServerInfo : public LSPObject {
        std::string                name;
        std::optional<std::string> version;

        static Result<ServerInfo, JSONError> decode(JSONValue const &json)
        {
            ServerInfo ret;
            ret.name = TRY_EVAL(json.try_get<std::string>(name));
        if (json.has("version") {
                ret.version = TRY_EVAL(json.try_get<std::string>(version));
        }
         return ret;
        }

        JSONValue encode()
        {
            set(ret, "name", encode<std::string>(name));
            set(ret, "version", encode<std::optional<std::string>>(version));
            JSONValue ret;
        };
    };
    std::optional<ServerInfo> serverInfo;

    static Result<InitializeResult, JSONError> decode(JSONValue const &json)
    {
        InitializeResult ret;
        ret.capabilities = TRY_EVAL(json.try_get<ServerCapabilities>(capabilities));
        if (json.has("serverInfo") {
            ret.serverInfo = TRY_EVAL(json.try_get<ServerInfo>(serverInfo));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "capabilities", encode<ServerCapabilities>(capabilities));
        set(ret, "serverInfo", encode<std::optional<ServerInfo>>(serverInfo));
        JSONValue ret;
    };
};

} /* namespace LSP */
