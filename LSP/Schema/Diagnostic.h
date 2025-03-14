/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/CodeDescription.h>
#include <LSP/Schema/DiagnosticRelatedInformation.h>
#include <LSP/Schema/DiagnosticSeverity.h>
#include <LSP/Schema/DiagnosticTag.h>
#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/Range.h>

namespace LSP {

struct Diagnostic {
    Range                                                    range;
    std::optional<DiagnosticSeverity>                        severity;
    std::optional<std::variant<int, std::string>>            code;
    std::optional<CodeDescription>                           codeDescription;
    std::optional<std::string>                               source;
    std::string                                              message;
    std::optional<std::vector<DiagnosticTag>>                tags;
    std::optional<std::vector<DiagnosticRelatedInformation>> relatedInformation;
    std::optional<Any>                                       data;

    static Decoded<Diagnostic> decode(JSONValue const &json)
    {
        Diagnostic ret;
        ret.range = TRY_EVAL(json.try_get<Range>("range"));
        if (json.has("severity")) {
            ret.severity = TRY_EVAL(json.try_get<DiagnosticSeverity>("severity"));
        }
        if (json.has("code")) {
            ret.code = TRY_EVAL(json.try_get_variant<int, std::string>("code"));
        }
        if (json.has("codeDescription")) {
            ret.codeDescription = TRY_EVAL(json.try_get<CodeDescription>("codeDescription"));
        }
        if (json.has("source")) {
            ret.source = TRY_EVAL(json.try_get<std::string>("source"));
        }
        ret.message = TRY_EVAL(json.try_get<std::string>("message"));
        if (json.has("tags")) {
            ret.tags = TRY_EVAL(json.try_get_array<DiagnosticTag>("tags"));
        }
        if (json.has("relatedInformation")) {
            ret.relatedInformation = TRY_EVAL(json.try_get_array<DiagnosticRelatedInformation>("relatedInformation"));
        }
        if (json.has("data")) {
            ret.data = TRY_EVAL(json.try_get<Any>("data"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "range", range);
        set(ret, "severity", severity);
        set(ret, "code", code);
        set(ret, "codeDescription", codeDescription);
        set(ret, "source", source);
        set(ret, "message", message);
        set(ret, "tags", tags);
        set(ret, "relatedInformation", relatedInformation);
        set(ret, "data", data);
        return ret;
    };
};

} /* namespace LSP */
