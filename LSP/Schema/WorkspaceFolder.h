/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/URI.h>

namespace LSP {

struct WorkspaceFolder {
    URI         uri;
    std::string name;

    static Decoded<WorkspaceFolder> decode(JSONValue const &json)
    {
        WorkspaceFolder ret;
        ret.uri = TRY_EVAL(json.try_get<URI>("uri"));
        ret.name = TRY_EVAL(json.try_get<std::string>("name"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "uri", uri);
        set(ret, "name", name);
        return ret;
    };
};

} /* namespace LSP */
