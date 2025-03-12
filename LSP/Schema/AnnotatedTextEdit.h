/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/ChangeAnnotationIdentifier.h>
#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TextEdit.h>

namespace LSP {

struct AnnotatedTextEdit : public TextEdit {
    ChangeAnnotationIdentifier annotationId;

    static Decoded<AnnotatedTextEdit> decode(JSONValue const &json)
    {
        AnnotatedTextEdit ret;
        ret.annotationId = TRY_EVAL(json.try_get<ChangeAnnotationIdentifier>("annotationId"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "annotationId", annotationId);
        return ret;
    };
};

} /* namespace LSP */
