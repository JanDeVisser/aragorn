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

struct Empty {
    JSONValue encode() const
    {
        return JSONValue::object();
    }

    static Decoded<Empty> decode(JSONValue const &json)
    {
        if (!json.is_object() || !json.empty()) {
            return JSONError { JSONError::Code::TypeMismatch, "" };
        }
        return Empty {};
    }
};

struct Null {
    JSONValue encode() const
    {
        return JSONValue {};
    }

    static Decoded<Null> decode(JSONValue const &json)
    {
        if (!json.is_null()) {
            return JSONError { JSONError::Code::TypeMismatch, "" };
        }
        return Null {};
    }
};

struct Any {
    JSONValue value;

    JSONValue encode() const
    {
        return value;
    }

    static Decoded<Any> decode(JSONValue const &json)
    {
        return Any { json };
    }
};

}
