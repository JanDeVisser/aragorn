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

inline std::string TraceValue_as_string(TraceValue obj)
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

inline std::optional<TraceValue> TraceValue_from_string(std::string_view const &s)
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
    return JSONValue { TraceValue_as_string(obj) };
}

template<>
inline Decoded<TraceValue> decode(JSONValue const &json)
{
    return TraceValue_from_string(json.to_string());
}

} /* namespace LibCore */
