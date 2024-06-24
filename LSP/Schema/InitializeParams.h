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

struct InitializeParams : public LSPObject {
    std::variant<int, LSP::Null> processId;
    struct ClientInfo : public LSPObject {
        std::string                name;
        std::optional<std::string> version;

        static Result<ClientInfo, JSONError> decode(JSONValue const &json)
        {
            ClientInfo ret;
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
    std::optional<ClientInfo>                                            clientInfo;
    std::optional<std::string>                                           locale;
    std::optional<std::variant<std::string, LSP::Null>>                  rootPath;
    std::variant<DocumentUri, LSP::Null>                                 rootUri;
    std::optional<LSP::Any>                                              initializationOptions;
    ClientCapabilities                                                   capabilities;
    std::optional<TraceValue>                                            trace;
    std::optional<std::variant<std::vector<WorkspaceFolder>, LSP::Null>> workspaceFolders;

    static Result<InitializeParams, JSONError> decode(JSONValue const &json)
    {
        InitializeParams ret;
        ret.processId = TRY_EVAL(json.try_get<std::variant<int, LSP::Null>>(processId));
        if (json.has("clientInfo") {
            ret.clientInfo = TRY_EVAL(json.try_get<ClientInfo>(clientInfo));
        }
        if (json.has("locale") {
            ret.locale = TRY_EVAL(json.try_get<std::string>(locale));
        }
        if (json.has("rootPath") {
            ret.rootPath = TRY_EVAL(json.try_get<std::variant<std::string, LSP::Null>>(rootPath));
        }
        ret.rootUri = TRY_EVAL(json.try_get<std::variant<DocumentUri,LSP::Null>>(rootUri));
        if (json.has("initializationOptions") {
            ret.initializationOptions = TRY_EVAL(json.try_get<LSP::Any>(initializationOptions));
        }
        ret.capabilities = TRY_EVAL(json.try_get<ClientCapabilities>(capabilities));
        if (json.has("trace") {
            ret.trace = TRY_EVAL(json.try_get<TraceValue>(trace));
        }
        if (json.has("workspaceFolders") {
            ret.workspaceFolders = TRY_EVAL(json.try_get<std::variant<std::vector<WorkspaceFolder>, LSP::Null>>(workspaceFolders));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "processId", encode<std::variant<int, LSP::Null>>(processId));
        set(ret, "clientInfo", encode<std::optional<ClientInfo>>(clientInfo));
        set(ret, "locale", encode<std::optional<std::string>>(locale));
        set(ret, "rootPath", encode<std::optional<std::variant<std::string, LSP::Null>>>(rootPath));
        set(ret, "rootUri", encode<std::variant<DocumentUri, LSP::Null>>(rootUri));
        set(ret, "initializationOptions", encode<std::optional<LSP::Any>>(initializationOptions));
        set(ret, "capabilities", encode<ClientCapabilities>(capabilities));
        set(ret, "trace", encode<std::optional<TraceValue>>(trace));
        set(ret, "workspaceFolders", encode<std::optional<std::variant<std::vector<WorkspaceFolder>, LSP::Null>>>(workspaceFolders));
        JSONValue ret;
    };
};

} /* namespace LSP */
