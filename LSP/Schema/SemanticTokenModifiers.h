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

enum class SemanticTokenModifiers {
    Declaration,
    Definition,
    Readonly,
    Static,
    Deprecated,
    Abstract,
    Async,
    Modification,
    Documentation,
    DefaultLibrary,
};

template<>
inline std::string as_string(SemanticTokenModifiers obj)
{
    switch (obj) {
    case SemanticTokenModifiers::Declaration:
        return "declaration";
    case SemanticTokenModifiers::Definition:
        return "definition";
    case SemanticTokenModifiers::Readonly:
        return "readonly";
    case SemanticTokenModifiers::Static:
        return "static";
    case SemanticTokenModifiers::Deprecated:
        return "deprecated";
    case SemanticTokenModifiers::Abstract:
        return "abstract";
    case SemanticTokenModifiers::Async:
        return "async";
    case SemanticTokenModifiers::Modification:
        return "modification";
    case SemanticTokenModifiers::Documentation:
        return "documentation";
    case SemanticTokenModifiers::DefaultLibrary:
        return "defaultLibrary";
    default:
        return "unknown";
    }
}

template<>
inline std::optional<SemanticTokenModifiers> from_string<SemanticTokenModifiers>(std::string_view const &s)
{
    if (s == "declaration")
        return SemanticTokenModifiers::Declaration;
    if (s == "definition")
        return SemanticTokenModifiers::Definition;
    if (s == "readonly")
        return SemanticTokenModifiers::Readonly;
    if (s == "static")
        return SemanticTokenModifiers::Static;
    if (s == "deprecated")
        return SemanticTokenModifiers::Deprecated;
    if (s == "abstract")
        return SemanticTokenModifiers::Abstract;
    if (s == "async")
        return SemanticTokenModifiers::Async;
    if (s == "modification")
        return SemanticTokenModifiers::Modification;
    if (s == "documentation")
        return SemanticTokenModifiers::Documentation;
    if (s == "defaultLibrary")
        return SemanticTokenModifiers::DefaultLibrary;
    return {};
}

} /* namespace LSP */

namespace LibCore {

using namespace LSP;

template<>
inline JSONValue encode(SemanticTokenModifiers const &obj)
{
    return encode_string_enum<SemanticTokenModifiers>(obj);
}

template<>
inline Result<SemanticTokenModifiers, JSONError> decode(JSONValue const &json)
{
    return decode_string_enum<SemanticTokenModifiers>(json);
}

} /* namespace LibCore */
