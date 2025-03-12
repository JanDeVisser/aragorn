/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "LibCore/Utf8.h"

#include <LibCore/JSON.h>
#include <LibCore/Lexer.h>

namespace TSParser {

using namespace LibCore;

#define TS_KEYWORDS(S)        \
    S(Boolean, "boolean")     \
    S(Const, "const")         \
    S(Decimal, "decimal")     \
    S(Empty, "{}")            \
    S(Enum, "enum")           \
    S(Export, "export")       \
    S(Extends, "extends")     \
    S(Integer, "integer")     \
    S(Interface, "interface") \
    S(LSPAny, "LSPAny")       \
    S(Namespace, "namespace") \
    S(Null, "null")           \
    S(String, "string")       \
    S(Type, "type")           \
    S(UInteger, "uinteger")

enum class TSKeyword {
#undef S
#define S(kw, s) kw,
    TS_KEYWORDS(S)
#undef S
};

#define TYPEKINDS(S) \
    S(None)          \
    S(Basic)         \
    S(Constant)      \
    S(Type)          \
    S(Variant)       \
    S(Struct)

enum class TypeKind {
#undef S
#define S(kind) kind,
    TYPEKINDS(S)
#undef S
};

enum class BasicType {
    None,
    Any,
    Bool,
    Empty,
    Int,
    Null,
    String,
    Unsigned,
};

struct EnumerationValue {
    std::wstring                    name;
    std::variant<std::wstring, int> value;

    EnumerationValue(std::wstring_view const &name, std::wstring_view const &value)
        : name(name)
        , value(std::wstring { value })
    {
    }

    EnumerationValue(std::wstring_view const &name, int value)
        : name(name)
        , value(value)
    {
    }
};

struct Enumeration {
    BasicType                     value_type { BasicType::None };
    std::vector<EnumerationValue> values;

    JSONValue encode() const;
};

using pEnumeration = std::shared_ptr<Enumeration>;

struct ConstantType {
    std::wstring                          name;
    BasicType                             type;
    std::variant<std::wstring, int, bool> value;

    ConstantType(BasicType t)
        : type(t)
    {
    }

    ConstantType(int v)
        : type(BasicType::Int)
    {
        value.emplace<int>(v);
    }

    ConstantType(uint32_t v)
        : type(BasicType::Unsigned)
    {
        value.emplace<int>(v);
    }

    ConstantType(bool v)
        : type(BasicType::Bool)
    {
        value.emplace<bool>(v);
    }

    ConstantType(std::wstring_view const &v)
        : type(BasicType::String)
    {
        value.emplace<std::wstring>(std::wstring { v });
    }

    JSONValue encode() const;
};

using pType = std::shared_ptr<struct Type>;

struct Variant {
    std::vector<pType> options;

    JSONValue encode() const;
};

struct Property {
    std::wstring name;
    bool         optional { false };
    pType        type { nullptr };

    Property(std::wstring_view const &n)
        : name(n)
    {
    }

    JSONValue encode() const;
};

struct Interface {
    WStringList           extends;
    std::vector<Property> properties;
    Interface() = default;

    JSONValue encode_properties() const;
    JSONValue encode() const;

private:
    void add_properties(JSONValue &props) const;
};
using pInterface = std::shared_ptr<Interface>;

struct Type {
    using TypeDefinition = std::variant<std::wstring,
        BasicType,
        ConstantType,
        Enumeration,
        Interface,
        Variant>;

    TypeKind       kind { TypeKind::None };
    bool           array { false };
    TypeDefinition definition;
    std::wstring   synthetic_name;

    Type() = default;

    BasicType &basic_type()
    {
        assert(kind == TypeKind::Basic);
        return std::get<BasicType>(definition);
    }

    BasicType const &basic_type() const
    {
        assert(kind == TypeKind::Basic);
        return std::get<BasicType>(definition);
    }

    ConstantType &constant_type()
    {
        assert(kind == TypeKind::Constant);
        return std::get<ConstantType>(definition);
    }

    ConstantType const &constant_type() const
    {
        assert(kind == TypeKind::Constant);
        return std::get<ConstantType>(definition);
    }

    Variant &variant()
    {
        assert(kind == TypeKind::Variant);
        return std::get<Variant>(definition);
    }

    Variant const &variant() const
    {
        assert(kind == TypeKind::Variant);
        return std::get<Variant>(definition);
    }

    Interface &interface()
    {
        assert(kind == TypeKind::Struct);
        return std::get<Interface>(definition);
    }

    Interface const &interface() const
    {
        assert(kind == TypeKind::Struct);
        return std::get<Interface>(definition);
    }

    std::wstring &type_name()
    {
        assert(kind == TypeKind::Type);
        return std::get<std::wstring>(definition);
    }

    std::wstring const &type_name() const
    {
        assert(kind == TypeKind::Type);
        return std::get<std::wstring>(definition);
    }

    JSONValue encode() const;
};

enum class TypeDefKind {
    None,
    Alias,
    Interface,
    Enumeration,
    Variant,
};

using pTypeDef = std::shared_ptr<struct TypeDef>;
class TypeDef {
public:
    using TypeDefinition = std::variant<pType, Interface, Enumeration, Variant>;
    TypeDefKind            kind { TypeDefKind::None };
    std::wstring           name {};
    std::set<std::wstring> dependencies;
    TypeDefinition         definition;

    TypeDef(TypeDefKind kind, std::wstring_view const &name)
        : kind(kind)
        , name(name)
    {
    }

    pType &alias_for()
    {
        assert(kind == TypeDefKind::Alias);
        return std::get<pType>(definition);
    }

    pType const &alias_for() const
    {
        assert(kind == TypeDefKind::Alias);
        return std::get<pType>(definition);
    }

    Enumeration &enumeration()
    {
        assert(kind == TypeDefKind::Enumeration);
        return std::get<Enumeration>(definition);
    }

    Enumeration const &enumeration() const
    {
        assert(kind == TypeDefKind::Enumeration);
        return std::get<Enumeration>(definition);
    }

    Interface &interface()
    {
        assert(kind == TypeDefKind::Interface);
        return std::get<Interface>(definition);
    }

    Interface const &interface() const
    {
        assert(kind == TypeDefKind::Interface);
        return std::get<Interface>(definition);
    }

    Variant &variant()
    {
        assert(kind == TypeDefKind::Variant);
        return std::get<Variant>(definition);
    }

    Variant const &variant() const
    {
        assert(kind == TypeDefKind::Variant);
        return std::get<Variant>(definition);
    }

    static TypeDef &get(std::wstring_view const &name)
    {
        std::wstring n { name };
        assert(s_typedefs.contains(n));
        return s_typedefs.at(n);
    }

    static bool has(std::wstring_view const &name)
    {
        return s_typedefs.contains(std::wstring { name });
    }

    static TypeDef &make_interface(std::wstring_view const &name)
    {
        auto &td = make(TypeDefKind::Interface, name);
        td.definition.emplace<Interface>();
        return td;
    }

    static TypeDef &make_alias(std::wstring_view const &name, pType type)
    {
        auto &td = make(TypeDefKind::Alias, name);
        td.definition = std::move(type);
        return td;
    }

    static TypeDef &make_enumeration(std::wstring_view const &name)
    {
        auto &td = make(TypeDefKind::Enumeration, name);
        td.definition.emplace<Enumeration>();
        return td;
    }

    static TypeDef &make_enumeration(std::wstring_view const &name, Enumeration e)
    {
        auto &td = make_enumeration(name);
        td.definition = std::move(e);
        return td;
    }

    void        get_dependencies(pType const &type);
    JSONValue   encode() const;
    static auto cbegin() { return s_typedefs.cbegin(); }
    static auto cend() { return s_typedefs.cend(); }

private:
    static TypeDef &make(TypeDefKind kind, std::wstring_view const &name)
    {
        std::wstring n { name };
        if (s_typedefs.contains(n)) {
            auto n_utf8 = MUST_EVAL(to_utf8(n));
            fatal("Type '{}' already registered", n_utf8);
        }
        auto const &p = s_typedefs.try_emplace(std::wstring { name }, kind, name);
        assert(p.second);
        return (*(p.first)).second;
    }

    static std::map<std::wstring, TypeDef> s_typedefs;
};

class Module {
private:
    class Sentinel { };

public:
    std::wstring name;
    WStringList  types;

    Module(Sentinel, std::wstring_view const &modname)
        : name(modname)
    {
    }

    JSONValue encode() const;
    auto      cbegin() { return TypeDef::cbegin(); }
    auto      cend() { return TypeDef::cend(); }
    auto      begin() { return TypeDef::cbegin(); }
    auto      end() { return TypeDef::cend(); }

    static Module &make(std::wstring_view const &modname);

private:
    static std::vector<Module> s_modules;
};

class Parser {
public:
    using TSLexer = Lexer<std::wstring_view, EnumKeywords<std::wstring_view, SimpleKeywordCategory, TSKeyword>>;
    Parser(std::string_view const &fname);
    Module const &parse();

private:
    pType parse_type();
    void  parse_struct(Interface &interface);
    void  parse_interface();
    void  parse_namespace();
    void  parse_enumeration();
    void  parse_typedef();

    Module     &m_module;
    std::string file_name;
    TSLexer     m_lexer {};
};

class CPPOutputter {
public:
    CPPOutputter(TypeDef const &type)
        : m_type(type)
    {
    }

    void output();

private:
    TypeDef const &m_type;
};

extern char const *BasicType_name(BasicType basic_type);
extern char const *TypeKind_name(TypeKind kind);
extern char const *TypeDefKind_name(TypeDefKind kind);

extern void generate_typedef(std::wstring_view const &name);

}

namespace LibCore {

using namespace TSParser;

template<>
[[nodiscard]] inline std::optional<std::tuple<SimpleKeywordCategory, TSKeyword, MatchType>> match_keyword(std::string const &str)
{
#undef S
#define S(KW, STR)                                                       \
    if (std::string_view(STR).starts_with(str)) {                        \
        return std::tuple {                                              \
            SimpleKeywordCategory::Keyword,                              \
            TSKeyword::KW,                                               \
            (str == STR) ? MatchType::FullMatch : MatchType::PrefixMatch \
        };                                                               \
    }
    TS_KEYWORDS(S)
#undef S
    return {};
}

}
