/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/MarkupKind.h>

namespace LSP {

struct MarkupContent {
    MarkupKind  kind;
    std::string value;

    static Decoded<MarkupContent> decode(JSONValue const &json)
    {
        MarkupContent ret;
        ret.kind = TRY_EVAL(json.try_get<MarkupKind>("kind"));
        ret.value = TRY_EVAL(json.try_get<std::string>("value"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "kind", kind);
        set(ret, "value", value);
        return ret;
    };
};

} /* namespace LSP */
