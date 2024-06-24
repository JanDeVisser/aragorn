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

struct Location : public LSPObject {
    DocumentUri uri;
    Range       range;

    static Result<Location, JSONError> decode(JSONValue const &json)
    {
        Location ret;
        ret.uri = TRY_EVAL(json.try_get<DocumentUri>(uri));
        ret.range = TRY_EVAL(json.try_get<Range>(range));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "uri", encode<DocumentUri>(uri));
        set(ret, "range", encode<Range>(range));
        JSONValue ret;
    };
};

} /* namespace LSP */
