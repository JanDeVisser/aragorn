/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/SaveOptions.h>
#include <LSP/Schema/TextDocumentSyncKind.h>

namespace LSP {

struct TextDocumentSyncOptions : public LSPObject {
    std::optional<bool>                            openClose;
    std::optional<TextDocumentSyncKind>            change;
    std::optional<bool>                            willSave;
    std::optional<bool>                            willSaveWaitUntil;
    std::optional<std::variant<bool, SaveOptions>> save;

    static Result<TextDocumentSyncOptions, JSONError> decode(JSONValue const &json)
    {
        TextDocumentSyncOptions ret;
        if (json.has("openClose") {
            ret.openClose = TRY_EVAL(json.try_get<bool>(openClose));
        }
        if (json.has("change") {
            ret.change = TRY_EVAL(json.try_get<TextDocumentSyncKind>(change));
        }
        if (json.has("willSave") {
            ret.willSave = TRY_EVAL(json.try_get<bool>(willSave));
        }
        if (json.has("willSaveWaitUntil") {
            ret.willSaveWaitUntil = TRY_EVAL(json.try_get<bool>(willSaveWaitUntil));
        }
        if (json.has("save") {
            ret.save = TRY_EVAL(json.try_get<std::variant<bool, SaveOptions>>(save));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "openClose", encode<std::optional<bool>>(openClose));
        set(ret, "change", encode<std::optional<TextDocumentSyncKind>>(change));
        set(ret, "willSave", encode<std::optional<bool>>(willSave));
        set(ret, "willSaveWaitUntil", encode<std::optional<bool>>(willSaveWaitUntil));
        set(ret, "save", encode<std::optional<std::variant<bool, SaveOptions>>>(save));
        JSONValue ret;
    };
};

} /* namespace LSP */
