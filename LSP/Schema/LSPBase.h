/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <LibCore/JSON.h>

namespace LSP {

struct Empty {
};

struct Null {
};

}

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue to_json(Empty const &obj)
{
    return JSONValue::object();
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, Empty &)
{
    if (!json.is_object() || !json.empty()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    return {};
}

template<>
inline JSONValue to_json(Null const &obj)
{
    return JSONValue {};
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, Null &)
{
    if (!json.is_null()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    return {};
}

}
