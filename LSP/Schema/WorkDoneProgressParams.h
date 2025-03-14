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

struct WorkDoneProgressParams {

    static Decoded<WorkDoneProgressParams> decode(JSONValue const &json)
    {
        WorkDoneProgressParams ret;
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        return ret;
    };
};

} /* namespace LSP */
