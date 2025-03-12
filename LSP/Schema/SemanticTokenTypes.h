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

enum class SemanticTokenTypes {
    Namespace,
    Type,
    Class,
    Enum,
    Interface,
    Struct,
    TypeParameter,
    Parameter,
    Variable,
    Property,
    EnumMember,
    Event,
    Function,
    Method,
    Macro,
    Keyword,
    Modifier,
    Comment,
    String,
    Number,
    Regexp,
    Operator,
    Decorator,
};

inline std::string SemanticTokenTypes_as_string(SemanticTokenTypes obj)
{
    switch (obj) {
    case SemanticTokenTypes::Namespace:
        return "namespace";
    case SemanticTokenTypes::Type:
        return "type";
    case SemanticTokenTypes::Class:
        return "class";
    case SemanticTokenTypes::Enum:
        return "enum";
    case SemanticTokenTypes::Interface:
        return "interface";
    case SemanticTokenTypes::Struct:
        return "struct";
    case SemanticTokenTypes::TypeParameter:
        return "typeParameter";
    case SemanticTokenTypes::Parameter:
        return "parameter";
    case SemanticTokenTypes::Variable:
        return "variable";
    case SemanticTokenTypes::Property:
        return "property";
    case SemanticTokenTypes::EnumMember:
        return "enumMember";
    case SemanticTokenTypes::Event:
        return "event";
    case SemanticTokenTypes::Function:
        return "function";
    case SemanticTokenTypes::Method:
        return "method";
    case SemanticTokenTypes::Macro:
        return "macro";
    case SemanticTokenTypes::Keyword:
        return "keyword";
    case SemanticTokenTypes::Modifier:
        return "modifier";
    case SemanticTokenTypes::Comment:
        return "comment";
    case SemanticTokenTypes::String:
        return "string";
    case SemanticTokenTypes::Number:
        return "number";
    case SemanticTokenTypes::Regexp:
        return "regexp";
    case SemanticTokenTypes::Operator:
        return "operator";
    case SemanticTokenTypes::Decorator:
        return "decorator";
    default:
        return "unknown";
    }
}

inline std::optional<SemanticTokenTypes> SemanticTokenTypes_from_string(std::string_view const &s)
{
    if (s == "namespace")
        return SemanticTokenTypes::Namespace;
    if (s == "type")
        return SemanticTokenTypes::Type;
    if (s == "class")
        return SemanticTokenTypes::Class;
    if (s == "enum")
        return SemanticTokenTypes::Enum;
    if (s == "interface")
        return SemanticTokenTypes::Interface;
    if (s == "struct")
        return SemanticTokenTypes::Struct;
    if (s == "typeParameter")
        return SemanticTokenTypes::TypeParameter;
    if (s == "parameter")
        return SemanticTokenTypes::Parameter;
    if (s == "variable")
        return SemanticTokenTypes::Variable;
    if (s == "property")
        return SemanticTokenTypes::Property;
    if (s == "enumMember")
        return SemanticTokenTypes::EnumMember;
    if (s == "event")
        return SemanticTokenTypes::Event;
    if (s == "function")
        return SemanticTokenTypes::Function;
    if (s == "method")
        return SemanticTokenTypes::Method;
    if (s == "macro")
        return SemanticTokenTypes::Macro;
    if (s == "keyword")
        return SemanticTokenTypes::Keyword;
    if (s == "modifier")
        return SemanticTokenTypes::Modifier;
    if (s == "comment")
        return SemanticTokenTypes::Comment;
    if (s == "string")
        return SemanticTokenTypes::String;
    if (s == "number")
        return SemanticTokenTypes::Number;
    if (s == "regexp")
        return SemanticTokenTypes::Regexp;
    if (s == "operator")
        return SemanticTokenTypes::Operator;
    if (s == "decorator")
        return SemanticTokenTypes::Decorator;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(SemanticTokenTypes const &obj)
{
    return JSONValue { SemanticTokenTypes_as_string(obj) };
}

template<>
inline Decoded<SemanticTokenTypes> decode(JSONValue const &json)
{
    return SemanticTokenTypes_from_string(json.to_string());
}

} /* namespace LibCore */
