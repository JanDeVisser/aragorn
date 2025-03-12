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

enum class CompletionTriggerKind {
    Invoked = 1,
    TriggerCharacter = 2,
    TriggerForIncompleteCompletions = 3,
};

inline std::optional<CompletionTriggerKind> CompletionTriggerKind_from_int(int i)
{
    if (i == 1)
        return CompletionTriggerKind::Invoked;
    if (i == 2)
        return CompletionTriggerKind::TriggerCharacter;
    if (i == 3)
        return CompletionTriggerKind::TriggerForIncompleteCompletions;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(CompletionTriggerKind const &obj)
{
    return JSONValue { static_cast<int>(obj) };
}

template<>
inline Decoded<CompletionTriggerKind> decode(JSONValue const &json)
{
    int int_val;
    TRY(json.convert(int_val));
    if (auto v = CompletionTriggerKind_from_int(int_val); !v) {
        return JSONError {
            JSONError::Code::UnexpectedValue,
            "Cannot convert JSON value of type 'CompletionTriggerKind' to integer",
        };
    } else {
        return *v;
    }
}

} /* namespace LibCore */
