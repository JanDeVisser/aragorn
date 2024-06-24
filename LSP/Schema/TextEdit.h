/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/Range.h>

namespace LSP {

struct TextEdit : public LSPObject {
    Range       range;
    std::string newText;

    static Result<TextEdit, JSONError> decode(JSONValue const &json)
    {
        TextEdit ret;
        ret.range = TRY_EVAL(json.try_get<Range>(range));
        ret.newText = TRY_EVAL(json.try_get<std::string>(newText));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "range", encode<Range>(range));
        set(ret, "newText", encode<std::string>(newText));
        JSONValue ret;
    };
};

} /* namespace LSP */
