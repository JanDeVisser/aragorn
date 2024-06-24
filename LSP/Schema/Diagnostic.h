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

struct Diagnostic : public LSPObject {
    Range                                                    range;
    std::optional<DiagnosticSeverity>                        severity;
    std::optional<std::variant<int, std::string>>            code;
    std::optional<CodeDescription>                           codeDescription;
    std::optional<std::string>                               source;
    std::string                                              message;
    std::optional<std::vector<DiagnosticTag>>                tags;
    std::optional<std::vector<DiagnosticRelatedInformation>> relatedInformation;
    std::optional<LSP::Any>                                  data;

    static Result<Diagnostic, JSONError> decode(JSONValue const &json)
    {
        Diagnostic ret;
        ret.range = TRY_EVAL(json.try_get<Range>(range));
        if (json.has("severity") {
            ret.severity = TRY_EVAL(json.try_get<DiagnosticSeverity>(severity));
        }
        if (json.has("code") {
            ret.code = TRY_EVAL(json.try_get<std::variant<int, std::string>>(code));
        }
        if (json.has("codeDescription") {
            ret.codeDescription = TRY_EVAL(json.try_get<CodeDescription>(codeDescription));
        }
        if (json.has("source") {
            ret.source = TRY_EVAL(json.try_get<std::string>(source));
        }
        ret.message = TRY_EVAL(json.try_get<std::string>(message));
        if (json.has("tags") {
            ret.tags = TRY_EVAL(json.try_get<std::vector<DiagnosticTag>>(tags));
        }
        if (json.has("relatedInformation") {
            ret.relatedInformation = TRY_EVAL(json.try_get<std::vector<DiagnosticRelatedInformation>>(relatedInformation));
        }
        if (json.has("data") {
            ret.data = TRY_EVAL(json.try_get<LSP::Any>(data));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "range", encode<Range>(range));
        set(ret, "severity", encode<std::optional<DiagnosticSeverity>>(severity));
        set(ret, "code", encode<std::optional<std::variant<int, std::string>>>(code));
        set(ret, "codeDescription", encode<std::optional<CodeDescription>>(codeDescription));
        set(ret, "source", encode<std::optional<std::string>>(source));
        set(ret, "message", encode<std::string>(message));
        set(ret, "tags", encode<std::optional<std::vector<DiagnosticTag>>>(tags));
        set(ret, "relatedInformation", encode<std::optional<std::vector<DiagnosticRelatedInformation>>>(relatedInformation));
        set(ret, "data", encode<std::optional<LSP::Any>>(data));
        JSONValue ret;
    };
};

} /* namespace LSP */
