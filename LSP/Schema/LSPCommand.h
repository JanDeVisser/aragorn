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

struct LSPCommand {
    std::string                     title;
    std::string                     command;
    std::optional<std::vector<Any>> arguments;

    static Decoded<LSPCommand> decode(JSONValue const &json)
    {
        LSPCommand ret;
        ret.title = TRY_EVAL(json.try_get<std::string>("title"));
        ret.command = TRY_EVAL(json.try_get<std::string>("command"));
        if (json.has("arguments")) {
            ret.arguments = TRY_EVAL(json.try_get_array<Any>("arguments"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "title", title);
        set(ret, "command", command);
        set(ret, "arguments", arguments);
        return ret;
    };
};

} /* namespace LSP */
