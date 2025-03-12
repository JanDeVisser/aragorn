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

struct SaveOptions {
    std::optional<bool> includeText;

    static Decoded<SaveOptions> decode(JSONValue const &json)
    {
        SaveOptions ret;
        if (json.has("includeText")) {
            ret.includeText = TRY_EVAL(json.try_get<bool>("includeText"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "includeText", includeText);
        return ret;
    };
};

} /* namespace LSP */
