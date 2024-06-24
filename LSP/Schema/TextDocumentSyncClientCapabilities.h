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

struct TextDocumentSyncClientCapabilities : public LSPObject {
    std::optional<bool> dynamicRegistration;
    std::optional<bool> willSave;
    std::optional<bool> willSaveWaitUntil;
    std::optional<bool> didSave;

    static Result<TextDocumentSyncClientCapabilities, JSONError> decode(JSONValue const &json)
    {
        TextDocumentSyncClientCapabilities ret;
        if (json.has("dynamicRegistration") {
            ret.dynamicRegistration = TRY_EVAL(json.try_get<bool>(dynamicRegistration));
        }
        if (json.has("willSave") {
            ret.willSave = TRY_EVAL(json.try_get<bool>(willSave));
        }
        if (json.has("willSaveWaitUntil") {
            ret.willSaveWaitUntil = TRY_EVAL(json.try_get<bool>(willSaveWaitUntil));
        }
        if (json.has("didSave") {
            ret.didSave = TRY_EVAL(json.try_get<bool>(didSave));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "dynamicRegistration", encode<std::optional<bool>>(dynamicRegistration));
        set(ret, "willSave", encode<std::optional<bool>>(willSave));
        set(ret, "willSaveWaitUntil", encode<std::optional<bool>>(willSaveWaitUntil));
        set(ret, "didSave", encode<std::optional<bool>>(didSave));
        JSONValue ret;
    };
};

} /* namespace LSP */
