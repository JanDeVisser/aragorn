/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/URI.h>

namespace LSP {

struct CodeDescription {
    URI href;

    static Decoded<CodeDescription> decode(JSONValue const &json)
    {
        CodeDescription ret;
        ret.href = TRY_EVAL(json.try_get<URI>("href"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "href", href);
        return ret;
    };
};

} /* namespace LSP */
