/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/DiagnosticTag.h>
#include <LSP/Schema/LSPBase.h>

namespace LSP {

struct PublishDiagnosticsClientCapabilities {
    std::optional<bool> relatedInformation;
    struct TagSupport {
        std::vector<DiagnosticTag> valueSet;

        static Decoded<TagSupport> decode(JSONValue const &json)
        {
            TagSupport ret;
            ret.valueSet = TRY_EVAL(json.try_get_array<DiagnosticTag>("valueSet"));
            return std::move(ret);
        }

        JSONValue encode() const
        {
            JSONValue ret { JSONType::Object };
            set(ret, "valueSet", valueSet);
            return ret;
        };
    };
    std::optional<TagSupport> tagSupport;
    std::optional<bool>       versionSupport;
    std::optional<bool>       codeDescriptionSupport;
    std::optional<bool>       dataSupport;

    static Decoded<PublishDiagnosticsClientCapabilities> decode(JSONValue const &json)
    {
        PublishDiagnosticsClientCapabilities ret;
        if (json.has("relatedInformation")) {
            ret.relatedInformation = TRY_EVAL(json.try_get<bool>("relatedInformation"));
        }
        if (json.has("tagSupport")) {
            ret.tagSupport = TRY_EVAL(json.try_get<TagSupport>("tagSupport"));
        }
        if (json.has("versionSupport")) {
            ret.versionSupport = TRY_EVAL(json.try_get<bool>("versionSupport"));
        }
        if (json.has("codeDescriptionSupport")) {
            ret.codeDescriptionSupport = TRY_EVAL(json.try_get<bool>("codeDescriptionSupport"));
        }
        if (json.has("dataSupport")) {
            ret.dataSupport = TRY_EVAL(json.try_get<bool>("dataSupport"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "relatedInformation", relatedInformation);
        set(ret, "tagSupport", tagSupport);
        set(ret, "versionSupport", versionSupport);
        set(ret, "codeDescriptionSupport", codeDescriptionSupport);
        set(ret, "dataSupport", dataSupport);
        return ret;
    };
};

} /* namespace LSP */
