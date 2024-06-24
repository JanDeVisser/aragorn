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

enum class InsertTextMode {
    AsIs = 1,
    AdjustIndentation = 2,
};

template<>
inline std::optional<InsertTextMode> from_int<InsertTextMode>(int i)
{
    if (i == 1)
        return InsertTextMode::AsIs;
    if (i == 2)
        return InsertTextMode::AdjustIndentation;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(InsertTextMode const &obj)
{
    return encode_int_enum(obj);
}

template<>
inline Result<InsertTextMode, JSONError> decode(JSONValue const &json)
{
    return decode_int_enum(json);
}

} /* namespace LibCore */
