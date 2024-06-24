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

template<>
inline std::optional<CompletionTriggerKind> from_int<CompletionTriggerKind>(int i)
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
    return encode_int_enum(obj);
}

template<>
inline Result<CompletionTriggerKind, JSONError> decode(JSONValue const &json)
{
    return decode_int_enum(json);
}

} /* namespace LibCore */
