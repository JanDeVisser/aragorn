/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/CompletionTriggerKind.h>
#include <LSP/Schema/LSPBase.h>

namespace LSP {

struct CompletionContext : public LSPObject {
    CompletionTriggerKind      triggerKind;
    std::optional<std::string> triggerCharacter;

    static Result<CompletionContext, JSONError> decode(JSONValue const &json)
    {
        CompletionContext ret;
        ret.triggerKind = TRY_EVAL(json.try_get<CompletionTriggerKind>(triggerKind));
        if (json.has("triggerCharacter") {
            ret.triggerCharacter = TRY_EVAL(json.try_get<std::string>(triggerCharacter));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "triggerKind", encode<CompletionTriggerKind>(triggerKind));
        set(ret, "triggerCharacter", encode<std::optional<std::string>>(triggerCharacter));
        JSONValue ret;
    };
};

} /* namespace LSP */
