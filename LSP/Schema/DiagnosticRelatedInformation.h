/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/Location.h>

namespace LSP {

struct DiagnosticRelatedInformation {
    Location    location;
    std::string message;

    static Decoded<DiagnosticRelatedInformation> decode(JSONValue const &json)
    {
        DiagnosticRelatedInformation ret;
        ret.location = TRY_EVAL(json.try_get<Location>("location"));
        ret.message = TRY_EVAL(json.try_get<std::string>("message"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "location", location);
        set(ret, "message", message);
        return ret;
    };
};

} /* namespace LSP */
