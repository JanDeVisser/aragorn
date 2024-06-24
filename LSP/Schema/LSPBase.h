/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <LibCore/JSON.h>

namespace LSP {

using namespace LibCore;

using URI = std::string;
using DocumentUri = URI;

class LSPObject {
};

struct Empty {
};

struct Null {
};

template<typename T>
inline std::string as_string(T obj)
{
    fatal("Override LSP::as_string<{}>(obj)", typeid(T).name());
}

template<typename T>
inline std::optional<T> from_string(std::string_view const &s)
{
    return {};
}

template<typename T>
inline std::optional<T> from_int(int i)
{
    return {};
}

template<typename Enum>
inline JSONValue encode_string_enum(Enum const& obj)
{
    return JSONValue { as_string<Enum>(obj) };
}

template<typename Enum>
inline Result<Enum, JSONError> decode_string_enum(JSONValue const &json)
{
    if (!json.is_string()) {
        return JSONError { JSONError::Code::TypeMismatch, typeid(Enum).name() };
    }
    auto val_maybe = from_string<Enum>(json.to_string());
    if (!val_maybe) {
        return JSONError { JSONError::Code::TypeMismatch, typeid(Enum).name() };
    }
    return *val_maybe;
}

template<typename Enum>
inline JSONValue encode_int_enum(Enum const& obj)
{
    return JSONValue { static_cast<int>(obj) };
}

template<typename Enum>
inline Result<Enum, JSONError> decode_int_enum(JSONValue const &json)
{
    if (!json.is_integer()) {
        return JSONError { JSONError::Code::TypeMismatch, typeid(Enum).name() };
    }
    auto val_maybe = from_int<Enum>(value<int>(json).value());
    if (!val_maybe) {
        return JSONError { JSONError::Code::TypeMismatch, typeid(Enum).name() };
    }
    return *val_maybe;
}

}

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(Empty const &obj)
{
    return JSONValue::object();
}

template<>
inline Result<Empty,JSONError> decode(JSONValue const &json)
{
    if (!json.is_object() || !json.empty()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    return Empty {};
}

template<>
inline JSONValue encode(Null const &obj)
{
    return JSONValue {};
}

template<>
inline Result<Null,JSONError> decode(JSONValue const &json)
{
    if (!json.is_null()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    return Null {};
}

}
