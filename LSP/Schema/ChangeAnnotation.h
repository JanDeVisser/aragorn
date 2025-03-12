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

struct ChangeAnnotation {
    std::string                label;
    std::optional<bool>        needsConfirmation;
    std::optional<std::string> description;

    static Decoded<ChangeAnnotation> decode(JSONValue const &json)
    {
        ChangeAnnotation ret;
        ret.label = TRY_EVAL(json.try_get<std::string>("label"));
        if (json.has("needsConfirmation")) {
            ret.needsConfirmation = TRY_EVAL(json.try_get<bool>("needsConfirmation"));
        }
        if (json.has("description")) {
            ret.description = TRY_EVAL(json.try_get<std::string>("description"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "label", label);
        set(ret, "needsConfirmation", needsConfirmation);
        set(ret, "description", description);
        return ret;
    };
};

} /* namespace LSP */
