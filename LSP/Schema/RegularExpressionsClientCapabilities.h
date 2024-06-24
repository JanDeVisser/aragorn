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

struct RegularExpressionsClientCapabilities : public LSPObject {
    std::string                engine;
    std::optional<std::string> version;

    static Result<RegularExpressionsClientCapabilities, JSONError> decode(JSONValue const &json)
    {
        RegularExpressionsClientCapabilities ret;
        ret.engine = TRY_EVAL(json.try_get<std::string>(engine));
        if (json.has("version") {
            ret.version = TRY_EVAL(json.try_get<std::string>(version));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "engine", encode<std::string>(engine));
        set(ret, "version", encode<std::optional<std::string>>(version));
        JSONValue ret;
    };
};

} /* namespace LSP */
