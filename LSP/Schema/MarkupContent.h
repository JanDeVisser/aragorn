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

struct MarkupContent : public LSPObject {
    MarkupKind  kind;
    std::string value;

    static Result<MarkupContent, JSONError> decode(JSONValue const &json)
    {
        MarkupContent ret;
        ret.kind = TRY_EVAL(json.try_get<MarkupKind>(kind));
        ret.value = TRY_EVAL(json.try_get<std::string>(value));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "kind", encode<MarkupKind>(kind));
        set(ret, "value", encode<std::string>(value));
        JSONValue ret;
    };
};

} /* namespace LSP */
