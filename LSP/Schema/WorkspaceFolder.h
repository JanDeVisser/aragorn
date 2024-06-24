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

struct WorkspaceFolder : public LSPObject {
    URI         uri;
    std::string name;

    static Result<WorkspaceFolder, JSONError> decode(JSONValue const &json)
    {
        WorkspaceFolder ret;
        ret.uri = TRY_EVAL(json.try_get<URI>(uri));
        ret.name = TRY_EVAL(json.try_get<std::string>(name));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "uri", encode<URI>(uri));
        set(ret, "name", encode<std::string>(name));
        JSONValue ret;
    };
};

} /* namespace LSP */
