/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/DocumentUri.h>
#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/Range.h>

namespace LSP {

struct Location {
    DocumentUri uri;
    Range       range;

    static Decoded<Location> decode(JSONValue const &json)
    {
        Location ret;
        ret.uri = TRY_EVAL(json.try_get<DocumentUri>("uri"));
        ret.range = TRY_EVAL(json.try_get<Range>("range"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "uri", uri);
        set(ret, "range", range);
        return ret;
    };
};

} /* namespace LSP */
