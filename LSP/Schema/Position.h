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

struct Position : public LSPObject {
    uint32_t line;
    uint32_t character;

    static Result<Position, JSONError> decode(JSONValue const &json)
    {
        Position ret;
        ret.line = TRY_EVAL(json.try_get<uint32_t>(line));
        ret.character = TRY_EVAL(json.try_get<uint32_t>(character));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "line", encode<uint32_t>(line));
        set(ret, "character", encode<uint32_t>(character));
        JSONValue ret;
    };
};

} /* namespace LSP */
