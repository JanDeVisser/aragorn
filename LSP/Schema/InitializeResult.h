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

struct InitializeResult {
    ServerCapabilities capabilities;
    struct ServerInfo {
        std::string                name;
        std::optional<std::string> version;

        static Decoded<ServerInfo> decode(JSONValue const &json)
        {
            ServerInfo ret;
            ret.name = TRY_EVAL(json.try_get<std::string>("name"));
            if (json.has("version")) {
                ret.version = TRY_EVAL(json.try_get<std::string>("version"));
            }
            return std::move(ret);
        }

        JSONValue encode() const
        {
            JSONValue ret { JSONType::Object };
            set(ret, "name", name);
            set(ret, "version", version);
            return ret;
        };
    };
    std::optional<ServerInfo> serverInfo;

    static Decoded<InitializeResult> decode(JSONValue const &json)
    {
        InitializeResult ret;
        ret.capabilities = TRY_EVAL(json.try_get<ServerCapabilities>("capabilities"));
        if (json.has("serverInfo")) {
            ret.serverInfo = TRY_EVAL(json.try_get<ServerInfo>("serverInfo"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "capabilities", capabilities);
        set(ret, "serverInfo", serverInfo);
        return ret;
    };
};

} /* namespace LSP */
