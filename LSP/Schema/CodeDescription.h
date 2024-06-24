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

struct CodeDescription : public LSPObject {
    URI href;

    static Result<CodeDescription, JSONError> decode(JSONValue const &json)
    {
        CodeDescription ret;
        ret.href = TRY_EVAL(json.try_get<URI>(href));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "href", encode<URI>(href));
        JSONValue ret;
    };
};

} /* namespace LSP */
