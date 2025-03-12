/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/DocumentUri.h>
#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/Range.h>

namespace LSP {

struct LocationLink {
    std::optional<Range> originSelectionRange;
    DocumentUri          targetUri;
    Range                targetRange;
    Range                targetSelectionRange;

    static Decoded<LocationLink> decode(JSONValue const &json)
    {
        LocationLink ret;
        if (json.has("originSelectionRange")) {
            ret.originSelectionRange = TRY_EVAL(json.try_get<Range>("originSelectionRange"));
        }
        ret.targetUri = TRY_EVAL(json.try_get<DocumentUri>("targetUri"));
        ret.targetRange = TRY_EVAL(json.try_get<Range>("targetRange"));
        ret.targetSelectionRange = TRY_EVAL(json.try_get<Range>("targetSelectionRange"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "originSelectionRange", originSelectionRange);
        set(ret, "targetUri", targetUri);
        set(ret, "targetRange", targetRange);
        set(ret, "targetSelectionRange", targetSelectionRange);
        return ret;
    };
};

} /* namespace LSP */
