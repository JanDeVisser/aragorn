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

struct FormattingOptions : public LSPObject {
    uint32_t            tabSize;
    bool                insertSpaces;
    std::optional<bool> trimTrailingWhitespace;
    std::optional<bool> insertFinalNewline;
    std::optional<bool> trimFinalNewlines;

    static Result<FormattingOptions, JSONError> decode(JSONValue const &json)
    {
        FormattingOptions ret;
        ret.tabSize = TRY_EVAL(json.try_get<uint32_t>(tabSize));
        ret.insertSpaces = TRY_EVAL(json.try_get<bool>(insertSpaces));
        if (json.has("trimTrailingWhitespace") {
            ret.trimTrailingWhitespace = TRY_EVAL(json.try_get<bool>(trimTrailingWhitespace));
        }
        if (json.has("insertFinalNewline") {
            ret.insertFinalNewline = TRY_EVAL(json.try_get<bool>(insertFinalNewline));
        }
        if (json.has("trimFinalNewlines") {
            ret.trimFinalNewlines = TRY_EVAL(json.try_get<bool>(trimFinalNewlines));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "tabSize", encode<uint32_t>(tabSize));
        set(ret, "insertSpaces", encode<bool>(insertSpaces));
        set(ret, "trimTrailingWhitespace", encode<std::optional<bool>>(trimTrailingWhitespace));
        set(ret, "insertFinalNewline", encode<std::optional<bool>>(insertFinalNewline));
        set(ret, "trimFinalNewlines", encode<std::optional<bool>>(trimFinalNewlines));
        JSONValue ret;
    };
};

} /* namespace LSP */
