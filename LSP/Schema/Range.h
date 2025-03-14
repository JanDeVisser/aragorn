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

struct Range {
    Position start;
    Position end;

    static Decoded<Range> decode(JSONValue const &json)
    {
        Range ret;
        ret.start = TRY_EVAL(json.try_get<Position>("start"));
        ret.end = TRY_EVAL(json.try_get<Position>("end"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "start", start);
        set(ret, "end", end);
        return ret;
    };
};

} /* namespace LSP */
