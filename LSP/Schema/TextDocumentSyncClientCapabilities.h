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

struct TextDocumentSyncClientCapabilities {
    std::optional<bool> dynamicRegistration;
    std::optional<bool> willSave;
    std::optional<bool> willSaveWaitUntil;
    std::optional<bool> didSave;

    static Decoded<TextDocumentSyncClientCapabilities> decode(JSONValue const &json)
    {
        TextDocumentSyncClientCapabilities ret;
        if (json.has("dynamicRegistration")) {
            ret.dynamicRegistration = TRY_EVAL(json.try_get<bool>("dynamicRegistration"));
        }
        if (json.has("willSave")) {
            ret.willSave = TRY_EVAL(json.try_get<bool>("willSave"));
        }
        if (json.has("willSaveWaitUntil")) {
            ret.willSaveWaitUntil = TRY_EVAL(json.try_get<bool>("willSaveWaitUntil"));
        }
        if (json.has("didSave")) {
            ret.didSave = TRY_EVAL(json.try_get<bool>("didSave"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "dynamicRegistration", dynamicRegistration);
        set(ret, "willSave", willSave);
        set(ret, "willSaveWaitUntil", willSaveWaitUntil);
        set(ret, "didSave", didSave);
        return ret;
    };
};

} /* namespace LSP */
