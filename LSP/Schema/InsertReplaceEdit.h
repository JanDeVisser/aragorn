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

struct InsertReplaceEdit {
    std::string newText;
    Range       insert;
    Range       replace;

    static Decoded<InsertReplaceEdit> decode(JSONValue const &json)
    {
        InsertReplaceEdit ret;
        ret.newText = TRY_EVAL(json.try_get<std::string>("newText"));
        ret.insert = TRY_EVAL(json.try_get<Range>("insert"));
        ret.replace = TRY_EVAL(json.try_get<Range>("replace"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "newText", newText);
        set(ret, "insert", insert);
        set(ret, "replace", replace);
        return ret;
    };
};

} /* namespace LSP */
