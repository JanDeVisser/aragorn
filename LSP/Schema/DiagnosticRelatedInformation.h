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

struct DiagnosticRelatedInformation : public LSPObject {
    Location    location;
    std::string message;

    static Result<DiagnosticRelatedInformation, JSONError> decode(JSONValue const &json)
    {
        DiagnosticRelatedInformation ret;
        ret.location = TRY_EVAL(json.try_get<Location>(location));
        ret.message = TRY_EVAL(json.try_get<std::string>(message));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "location", encode<Location>(location));
        set(ret, "message", encode<std::string>(message));
        JSONValue ret;
    };
};

} /* namespace LSP */
