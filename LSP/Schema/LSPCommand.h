/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>

namespace LSP {

struct LSPCommand : public LSPObject {
    std::string                          title;
    std::string                          command;
    std::optional<std::vector<LSP::Any>> arguments;

    static Result<LSPCommand, JSONError> decode(JSONValue const &json)
    {
        LSPCommand ret;
        ret.title = TRY_EVAL(json.try_get<std::string>(title));
        ret.command = TRY_EVAL(json.try_get<std::string>(command));
        if (json.has("arguments") {
            ret.arguments = TRY_EVAL(json.try_get<std::vector<LSP::Any>>(arguments));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "title", encode<std::string>(title));
        set(ret, "command", encode<std::string>(command));
        set(ret, "arguments", encode<std::optional<std::vector<LSP::Any>>>(arguments));
        JSONValue ret;
    };
};

} /* namespace LSP */
