/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/Position.h>

namespace LSP {

struct Range : public LSPObject {
    Position start;
    Position end;

    static Result<Range, JSONError> decode(JSONValue const &json)
    {
        Range ret;
        ret.start = TRY_EVAL(json.try_get<Position>(start));
        ret.end = TRY_EVAL(json.try_get<Position>(end));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "start", encode<Position>(start));
        set(ret, "end", encode<Position>(end));
        JSONValue ret;
    };
};

} /* namespace LSP */
