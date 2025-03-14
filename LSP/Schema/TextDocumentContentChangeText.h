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

struct TextDocumentContentChangeText {
    std::string text;

    static Decoded<TextDocumentContentChangeText> decode(JSONValue const &json)
    {
        TextDocumentContentChangeText ret;
        ret.text = TRY_EVAL(json.try_get<std::string>("text"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "text", text);
        return ret;
    };
};

} /* namespace LSP */
