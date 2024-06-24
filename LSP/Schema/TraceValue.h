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

enum class TraceValue {
    Off,
    Messages,
    Verbose,
};

template<>
inline std::string as_string(TraceValue obj)
{
    switch (obj) {
    case TraceValue::Off:
        return "off";
    case TraceValue::Messages:
        return "messages";
    case TraceValue::Verbose:
        return "verbose";
    default:
        return "unknown";
    }
}

template<>
inline std::optional<TraceValue> from_string<TraceValue>(std::string_view const &s)
{
    if (s == "off")
        return TraceValue::Off;
    if (s == "messages")
        return TraceValue::Messages;
    if (s == "verbose")
        return TraceValue::Verbose;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(TraceValue const &obj)
{
    return encode_string_enum<TraceValue>(obj);
}

template<>
inline Result<TraceValue, JSONError> decode(JSONValue const &json)
{
    return decode_string_enum<TraceValue>(json);
}

} /* namespace LibCore */
