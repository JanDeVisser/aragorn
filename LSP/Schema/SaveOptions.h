/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>

namespace LSP {

struct SaveOptions : public LSPObject {
    std::optional<bool> includeText;

    static Result<SaveOptions, JSONError> decode(JSONValue const &json)
    {
        SaveOptions ret;
        if (json.has("includeText") {
            ret.includeText = TRY_EVAL(json.try_get<bool>(includeText));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "includeText", encode<std::optional<bool>>(includeText));
        JSONValue ret;
    };
};

} /* namespace LSP */
