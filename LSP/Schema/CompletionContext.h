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

struct CompletionContext {
    CompletionTriggerKind      triggerKind;
    std::optional<std::string> triggerCharacter;

    static Decoded<CompletionContext> decode(JSONValue const &json)
    {
        CompletionContext ret;
        ret.triggerKind = TRY_EVAL(json.try_get<CompletionTriggerKind>("triggerKind"));
        if (json.has("triggerCharacter")) {
            ret.triggerCharacter = TRY_EVAL(json.try_get<std::string>("triggerCharacter"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "triggerKind", triggerKind);
        set(ret, "triggerCharacter", triggerCharacter);
        return ret;
    };
};

} /* namespace LSP */
