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

struct InsertReplaceEdit : public LSPObject {
    std::string newText;
    Range       insert;
    Range       replace;

    static Result<InsertReplaceEdit, JSONError> decode(JSONValue const &json)
    {
        InsertReplaceEdit ret;
        ret.newText = TRY_EVAL(json.try_get<std::string>(newText));
        ret.insert = TRY_EVAL(json.try_get<Range>(insert));
        ret.replace = TRY_EVAL(json.try_get<Range>(replace));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "newText", encode<std::string>(newText));
        set(ret, "insert", encode<Range>(insert));
        set(ret, "replace", encode<Range>(replace));
        JSONValue ret;
    };
};

} /* namespace LSP */
