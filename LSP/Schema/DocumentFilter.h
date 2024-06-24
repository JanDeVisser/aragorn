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

struct DocumentFilter : public LSPObject {
    std::optional<std::string> language;
    std::optional<std::string> scheme;
    std::optional<std::string> pattern;

    static Result<DocumentFilter, JSONError> decode(JSONValue const &json)
    {
        DocumentFilter ret;
        if (json.has("language") {
            ret.language = TRY_EVAL(json.try_get<std::string>(language));
        }
        if (json.has("scheme") {
            ret.scheme = TRY_EVAL(json.try_get<std::string>(scheme));
        }
        if (json.has("pattern") {
            ret.pattern = TRY_EVAL(json.try_get<std::string>(pattern));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "language", encode<std::optional<std::string>>(language));
        set(ret, "scheme", encode<std::optional<std::string>>(scheme));
        set(ret, "pattern", encode<std::optional<std::string>>(pattern));
        JSONValue ret;
    };
};

} /* namespace LSP */
