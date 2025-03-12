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

struct TextEdit {
    Range       range;
    std::string newText;

    static Decoded<TextEdit> decode(JSONValue const &json)
    {
        TextEdit ret;
        ret.range = TRY_EVAL(json.try_get<Range>("range"));
        ret.newText = TRY_EVAL(json.try_get<std::string>("newText"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "range", range);
        set(ret, "newText", newText);
        return ret;
    };
};

} /* namespace LSP */
