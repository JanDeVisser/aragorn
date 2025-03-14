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

struct DocumentFilter {
    std::optional<std::string> language;
    std::optional<std::string> scheme;
    std::optional<std::string> pattern;

    static Decoded<DocumentFilter> decode(JSONValue const &json)
    {
        DocumentFilter ret;
        if (json.has("language")) {
            ret.language = TRY_EVAL(json.try_get<std::string>("language"));
        }
        if (json.has("scheme")) {
            ret.scheme = TRY_EVAL(json.try_get<std::string>("scheme"));
        }
        if (json.has("pattern")) {
            ret.pattern = TRY_EVAL(json.try_get<std::string>("pattern"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "language", language);
        set(ret, "scheme", scheme);
        set(ret, "pattern", pattern);
        return ret;
    };
};

} /* namespace LSP */
