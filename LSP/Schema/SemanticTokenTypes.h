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

enum SemanticTokenTypes {
    SemanticTokenTypesNamespace,
    SemanticTokenTypesType,
    SemanticTokenTypesClass,
    SemanticTokenTypesEnum,
    SemanticTokenTypesInterface,
    SemanticTokenTypesStruct,
    SemanticTokenTypesTypeParameter,
    SemanticTokenTypesParameter,
    SemanticTokenTypesVariable,
    SemanticTokenTypesProperty,
    SemanticTokenTypesEnumMember,
    SemanticTokenTypesEvent,
    SemanticTokenTypesFunction,
    SemanticTokenTypesMethod,
    SemanticTokenTypesMacro,
    SemanticTokenTypesKeyword,
    SemanticTokenTypesModifier,
    SemanticTokenTypesComment,
    SemanticTokenTypesString,
    SemanticTokenTypesNumber,
    SemanticTokenTypesRegexp,
    SemanticTokenTypesOperator,
    SemanticTokenTypesDecorator,
};

[[nodiscard]] std::string_view                  SemanticTokenTypes_to_string(SemanticTokenTypes value);
[[nodiscard]] std::optional<SemanticTokenTypes> SemanticTokenTypes_parse(std::string_view const &s);

}

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue to_json(SemanticTokenTypes const &obj)
{
    return JSONValue { SemanticTokenTypes_to_string(obj) };
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, SemanticTokenTypes &obj)
{
    if (!json.is_string()) {
        return JSONError { JSONError::Code::TypeMismatch, "SemanticTokenTypes" };
    }
    auto obj_maybe = SemanticTokenTypes_parse(json.to_string());
    if (!obj_maybe) {
        return JSONError { JSONError::Code::TypeMismatch, "SemanticTokenTypes" };
    }
    obj = *obj_maybe;
    return {};
}

}
