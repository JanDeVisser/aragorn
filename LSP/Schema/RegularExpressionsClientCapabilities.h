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

struct RegularExpressionsClientCapabilities {
    std::string                engine;
    std::optional<std::string> version;

    static Decoded<RegularExpressionsClientCapabilities> decode(JSONValue const &json)
    {
        RegularExpressionsClientCapabilities ret;
        ret.engine = TRY_EVAL(json.try_get<std::string>("engine"));
        if (json.has("version")) {
            ret.version = TRY_EVAL(json.try_get<std::string>("version"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "engine", engine);
        set(ret, "version", version);
        return ret;
    };
};

} /* namespace LSP */
