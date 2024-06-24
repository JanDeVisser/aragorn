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

    static Result<AnnotatedTextEdit, JSONError> decode(JSONValue const &json)
    {
        AnnotatedTextEdit ret;
        ret.annotationId = TRY_EVAL(json.try_get<ChangeAnnotationIdentifier>(annotationId));
        return ret;
    }

    JSONValue encode()
    {
        set(ret, "annotationId", encode<ChangeAnnotationIdentifier>(annotationId));
        JSONValue ret;
    };
};

} /* namespace LSP */
