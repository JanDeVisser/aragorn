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

struct FormattingOptions {
    uint32_t            tabSize;
    bool                insertSpaces;
    std::optional<bool> trimTrailingWhitespace;
    std::optional<bool> insertFinalNewline;
    std::optional<bool> trimFinalNewlines;

    static Decoded<FormattingOptions> decode(JSONValue const &json)
    {
        FormattingOptions ret;
        ret.tabSize = TRY_EVAL(json.try_get<uint32_t>("tabSize"));
        ret.insertSpaces = TRY_EVAL(json.try_get<bool>("insertSpaces"));
        if (json.has("trimTrailingWhitespace")) {
            ret.trimTrailingWhitespace = TRY_EVAL(json.try_get<bool>("trimTrailingWhitespace"));
        }
        if (json.has("insertFinalNewline")) {
            ret.insertFinalNewline = TRY_EVAL(json.try_get<bool>("insertFinalNewline"));
        }
        if (json.has("trimFinalNewlines")) {
            ret.trimFinalNewlines = TRY_EVAL(json.try_get<bool>("trimFinalNewlines"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "tabSize", tabSize);
        set(ret, "insertSpaces", insertSpaces);
        set(ret, "trimTrailingWhitespace", trimTrailingWhitespace);
        set(ret, "insertFinalNewline", insertFinalNewline);
        set(ret, "trimFinalNewlines", trimFinalNewlines);
        return ret;
    };
};

} /* namespace LSP */
