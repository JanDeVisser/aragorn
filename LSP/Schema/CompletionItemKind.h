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

enum class CompletionItemKind {
    Text = 1,
    Method = 2,
    Function = 3,
    Constructor = 4,
    Field = 5,
    Variable = 6,
    Class = 7,
    Interface = 8,
    Module = 9,
    Property = 10,
    Unit = 11,
    Value = 12,
    Enum = 13,
    Keyword = 14,
    Snippet = 15,
    Color = 16,
    File = 17,
    Reference = 18,
    Folder = 19,
    EnumMember = 20,
    Constant = 21,
    Struct = 22,
    Event = 23,
    Operator = 24,
    TypeParameter = 25,
};

inline std::optional<CompletionItemKind> CompletionItemKind_from_int(int i)
{
    if (i == 1)
        return CompletionItemKind::Text;
    if (i == 2)
        return CompletionItemKind::Method;
    if (i == 3)
        return CompletionItemKind::Function;
    if (i == 4)
        return CompletionItemKind::Constructor;
    if (i == 5)
        return CompletionItemKind::Field;
    if (i == 6)
        return CompletionItemKind::Variable;
    if (i == 7)
        return CompletionItemKind::Class;
    if (i == 8)
        return CompletionItemKind::Interface;
    if (i == 9)
        return CompletionItemKind::Module;
    if (i == 10)
        return CompletionItemKind::Property;
    if (i == 11)
        return CompletionItemKind::Unit;
    if (i == 12)
        return CompletionItemKind::Value;
    if (i == 13)
        return CompletionItemKind::Enum;
    if (i == 14)
        return CompletionItemKind::Keyword;
    if (i == 15)
        return CompletionItemKind::Snippet;
    if (i == 16)
        return CompletionItemKind::Color;
    if (i == 17)
        return CompletionItemKind::File;
    if (i == 18)
        return CompletionItemKind::Reference;
    if (i == 19)
        return CompletionItemKind::Folder;
    if (i == 20)
        return CompletionItemKind::EnumMember;
    if (i == 21)
        return CompletionItemKind::Constant;
    if (i == 22)
        return CompletionItemKind::Struct;
    if (i == 23)
        return CompletionItemKind::Event;
    if (i == 24)
        return CompletionItemKind::Operator;
    if (i == 25)
        return CompletionItemKind::TypeParameter;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(CompletionItemKind const &obj)
{
    return JSONValue { static_cast<int>(obj) };
}

template<>
inline Decoded<CompletionItemKind> decode(JSONValue const &json)
{
    int int_val;
    TRY(json.convert(int_val));
    if (auto v = CompletionItemKind_from_int(int_val); !v) {
        return JSONError {
            JSONError::Code::UnexpectedValue,
            "Cannot convert JSON value of type 'CompletionItemKind' to integer",
        };
    } else {
        return *v;
    }
}

} /* namespace LibCore */
