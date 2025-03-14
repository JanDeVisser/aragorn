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

struct TextDocumentContentChangeRange {
    Range                   range;
    std::optional<uint32_t> rangeLength;
    std::string             text;

    static Decoded<TextDocumentContentChangeRange> decode(JSONValue const &json)
    {
        TextDocumentContentChangeRange ret;
        ret.range = TRY_EVAL(json.try_get<Range>("range"));
        if (json.has("rangeLength")) {
            ret.rangeLength = TRY_EVAL(json.try_get<uint32_t>("rangeLength"));
        }
        ret.text = TRY_EVAL(json.try_get<std::string>("text"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "range", range);
        set(ret, "rangeLength", rangeLength);
        set(ret, "text", text);
        return ret;
    };
};

} /* namespace LSP */
