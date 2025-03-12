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

struct Position {
    uint32_t line;
    uint32_t character;

    static Decoded<Position> decode(JSONValue const &json)
    {
        Position ret;
        ret.line = TRY_EVAL(json.try_get<uint32_t>("line"));
        ret.character = TRY_EVAL(json.try_get<uint32_t>("character"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "line", line);
        set(ret, "character", character);
        return ret;
    };
};

} /* namespace LSP */
