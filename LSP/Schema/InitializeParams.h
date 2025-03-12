/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/ClientCapabilities.h>
#include <LSP/Schema/DocumentUri.h>
#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TraceValue.h>
#include <LSP/Schema/WorkspaceFolder.h>

namespace LSP {

struct InitializeParams {
    std::variant<int, Null> processId;
    struct ClientInfo {
        std::string                name;
        std::optional<std::string> version;

        static Decoded<ClientInfo> decode(JSONValue const &json)
        {
            ClientInfo ret;
            ret.name = TRY_EVAL(json.try_get<std::string>("name"));
            if (json.has("version")) {
                ret.version = TRY_EVAL(json.try_get<std::string>("version"));
            }
            return std::move(ret);
        }

        JSONValue encode() const
        {
            JSONValue ret;
            set(ret, "name", name);
            set(ret, "version", version);
            return ret;
        };
    };
    std::optional<ClientInfo>                                       clientInfo;
    std::optional<std::string>                                      locale;
    std::optional<std::variant<std::string, Null>>                  rootPath;
    std::variant<DocumentUri, Null>                                 rootUri;
    std::optional<Any>                                              initializationOptions;
    ClientCapabilities                                              capabilities;
    std::optional<TraceValue>                                       trace;
    std::optional<std::variant<std::vector<WorkspaceFolder>, Null>> workspaceFolders;

    static Decoded<InitializeParams> decode(JSONValue const &json)
    {
        InitializeParams ret;
        ret.processId = TRY_EVAL(json.try_get_variant<int, Null>("processId"));
        if (json.has("clientInfo")) {
            ret.clientInfo = TRY_EVAL(json.try_get<ClientInfo>("clientInfo"));
        }
        if (json.has("locale")) {
            ret.locale = TRY_EVAL(json.try_get<std::string>("locale"));
        }
        if (json.has("rootPath")) {
            ret.rootPath = TRY_EVAL(json.try_get_variant<std::string, Null>("rootPath"));
        }
        ret.rootUri = TRY_EVAL(json.try_get_variant<DocumentUri, Null>("rootUri"));
        if (json.has("initializationOptions")) {
            ret.initializationOptions = TRY_EVAL(json.try_get<Any>("initializationOptions"));
        }
        ret.capabilities = TRY_EVAL(json.try_get<ClientCapabilities>("capabilities"));
        if (json.has("trace")) {
            ret.trace = TRY_EVAL(json.try_get<TraceValue>("trace"));
        }
        if (json.has("workspaceFolders")) {
            ret.workspaceFolders = TRY_EVAL(json.try_get_variant<std::vector<WorkspaceFolder>, Null>("workspaceFolders"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "processId", processId);
        set(ret, "clientInfo", clientInfo);
        set(ret, "locale", locale);
        set(ret, "rootPath", rootPath);
        set(ret, "rootUri", rootUri);
        set(ret, "initializationOptions", initializationOptions);
        set(ret, "capabilities", capabilities);
        set(ret, "trace", trace);
        set(ret, "workspaceFolders", workspaceFolders);
        return ret;
    };
};

} /* namespace LSP */
