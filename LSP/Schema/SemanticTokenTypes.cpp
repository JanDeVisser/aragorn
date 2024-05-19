/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#include <LSP/Schema/SemanticTokenTypes.h>

namespace LSP {

using namespace LibCore;

std::string_view SemanticTokenTypes_to_string(SemanticTokenTypes value)
{
    switch (value) {
    case SemanticTokenTypesNamespace:
        return "namespace";
    case SemanticTokenTypesType:
        return "type";
    case SemanticTokenTypesClass:
        return "class";
    case SemanticTokenTypesEnum:
        return "enum";
    case SemanticTokenTypesInterface:
        return "interface";
    case SemanticTokenTypesStruct:
        return "struct";
    case SemanticTokenTypesTypeParameter:
        return "typeParameter";
    case SemanticTokenTypesParameter:
        return "parameter";
    case SemanticTokenTypesVariable:
        return "variable";
    case SemanticTokenTypesProperty:
        return "property";
    case SemanticTokenTypesEnumMember:
        return "enumMember";
    case SemanticTokenTypesEvent:
        return "event";
    case SemanticTokenTypesFunction:
        return "function";
    case SemanticTokenTypesMethod:
        return "method";
    case SemanticTokenTypesMacro:
        return "macro";
    case SemanticTokenTypesKeyword:
        return "keyword";
    case SemanticTokenTypesModifier:
        return "modifier";
    case SemanticTokenTypesComment:
        return "comment";
    case SemanticTokenTypesString:
        return "string";
    case SemanticTokenTypesNumber:
        return "number";
    case SemanticTokenTypesRegexp:
        return "regexp";
    case SemanticTokenTypesOperator:
        return "operator";
    case SemanticTokenTypesDecorator:
        return "decorator";
    default:
        UNREACHABLE();
    }
}

std::optional<SemanticTokenTypes> SemanticTokenTypes_parse(std::string_view const& s)
{
    if (s == "namespace")
        return SemanticTokenTypesNamespace;
    if (s == "type")
        return SemanticTokenTypesType;
    if (s == "class")
        return SemanticTokenTypesClass;
    if (s == "enum")
        return SemanticTokenTypesEnum;
    if (s == "interface")
        return SemanticTokenTypesInterface;
    if (s == "struct")
        return SemanticTokenTypesStruct;
    if (s == "typeParameter")
        return SemanticTokenTypesTypeParameter;
    if (s == "parameter")
        return SemanticTokenTypesParameter;
    if (s == "variable")
        return SemanticTokenTypesVariable;
    if (s == "property")
        return SemanticTokenTypesProperty;
    if (s == "enumMember")
        return SemanticTokenTypesEnumMember;
    if (s == "event")
        return SemanticTokenTypesEvent;
    if (s == "function")
        return SemanticTokenTypesFunction;
    if (s == "method")
        return SemanticTokenTypesMethod;
    if (s == "macro")
        return SemanticTokenTypesMacro;
    if (s == "keyword")
        return SemanticTokenTypesKeyword;
    if (s == "modifier")
        return SemanticTokenTypesModifier;
    if (s == "comment")
        return SemanticTokenTypesComment;
    if (s == "string")
        return SemanticTokenTypesString;
    if (s == "number")
        return SemanticTokenTypesNumber;
    if (s == "regexp")
        return SemanticTokenTypesRegexp;
    if (s == "operator")
        return SemanticTokenTypesOperator;
    if (s == "decorator")
        return SemanticTokenTypesDecorator;
    return {};
}

}
