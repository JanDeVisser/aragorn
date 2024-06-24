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

struct PublishDiagnosticsClientCapabilities : public LSPObject {
    std::optional<bool> relatedInformation;
    struct TagSupport : public LSPObject {
        std::vector<DiagnosticTag> valueSet;

        static Result<TagSupport, JSONError> decode(JSONValue const &json)
        {
            TagSupport ret;
            ret.valueSet = TRY_EVAL(json.try_get<std::vector<DiagnosticTag>>(valueSet));
            return ret;
        }

        JSONValue encode()
        {
            set(ret, "valueSet", encode<std::vector<DiagnosticTag>>(valueSet));
            JSONValue ret;
        };
    };
    std::optional<TagSupport> tagSupport;
    std::optional<bool>       versionSupport;
    std::optional<bool>       codeDescriptionSupport;
    std::optional<bool>       dataSupport;

    static Result<PublishDiagnosticsClientCapabilities, JSONError> decode(JSONValue const &json)
    {
        PublishDiagnosticsClientCapabilities ret;
        if (json.has("relatedInformation") {
            ret.relatedInformation = TRY_EVAL(json.try_get<bool>(relatedInformation));
        }
        if (json.has("tagSupport") {
            ret.tagSupport = TRY_EVAL(json.try_get<TagSupport>(tagSupport));
        }
        if (json.has("versionSupport") {
            ret.versionSupport = TRY_EVAL(json.try_get<bool>(versionSupport));
        }
        if (json.has("codeDescriptionSupport") {
            ret.codeDescriptionSupport = TRY_EVAL(json.try_get<bool>(codeDescriptionSupport));
        }
        if (json.has("dataSupport") {
            ret.dataSupport = TRY_EVAL(json.try_get<bool>(dataSupport));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "relatedInformation", encode<std::optional<bool>>(relatedInformation));
        set(ret, "tagSupport", encode<std::optional<TagSupport>>(tagSupport));
        set(ret, "versionSupport", encode<std::optional<bool>>(versionSupport));
        set(ret, "codeDescriptionSupport", encode<std::optional<bool>>(codeDescriptionSupport));
        set(ret, "dataSupport", encode<std::optional<bool>>(dataSupport));
        JSONValue ret;
    };
};

} /* namespace LSP */
