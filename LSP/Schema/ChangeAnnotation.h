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

struct ChangeAnnotation : public LSPObject {
    std::string                label;
    std::optional<bool>        needsConfirmation;
    std::optional<std::string> description;

    static Result<ChangeAnnotation, JSONError> decode(JSONValue const &json)
    {
        ChangeAnnotation ret;
        ret.label = TRY_EVAL(json.try_get<std::string>(label));
        if (json.has("needsConfirmation") {
            ret.needsConfirmation = TRY_EVAL(json.try_get<bool>(needsConfirmation));
        }
        if (json.has("description") {
            ret.description = TRY_EVAL(json.try_get<std::string>(description));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "label", encode<std::string>(label));
        set(ret, "needsConfirmation", encode<std::optional<bool>>(needsConfirmation));
        set(ret, "description", encode<std::optional<std::string>>(description));
        JSONValue ret;
    };
};

} /* namespace LSP */
