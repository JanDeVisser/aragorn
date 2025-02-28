/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

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
    std::string                    name;
    std::variant<std::string, int> value;

    EnumerationValue(std::string_view const &name, std::string_view const &value)
        : name(name)
        , value(std::string { value })
    {
    }

    EnumerationValue(std::string_view const &name, int value)
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
    std::string                          name;
    BasicType                            type;
    std::variant<std::string, int, bool> value;

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

    ConstantType(std::string_view const &v)
        : type(BasicType::String)
    {
        value.emplace<std::string>(std::string { v });
    }

    JSONValue encode() const;
};

using pType = std::shared_ptr<struct Type>;

struct Variant {
    std::vector<pType> options;

    JSONValue encode() const;
};

struct Property {
    std::string name;
    bool        optional { false };
    pType       type { nullptr };

    Property(std::string_view const &n)
        : name(n)
    {
    }

    JSONValue encode() const;
};

struct Interface {
    StringList            extends;
    std::vector<Property> properties;
    Interface() = default;

    JSONValue encode_properties() const;
    JSONValue encode() const;

private:
    void add_properties(JSONValue &props) const;
};
using pInterface = std::shared_ptr<Interface>;

struct Type {
    using TypeDefinition = std::variant<std::string,
        BasicType,
        ConstantType,
        Enumeration,
        Interface,
        Variant>;

    TypeKind       kind { TypeKind::None };
    bool           array { false };
    TypeDefinition definition;
    std::string    synthetic_name;

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

    std::string &type_name()
    {
        assert(kind == TypeKind::Type);
        return std::get<std::string>(definition);
    }

    std::string const &type_name() const
    {
        assert(kind == TypeKind::Type);
        return std::get<std::string>(definition);
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
    TypeDefKind           kind { TypeDefKind::None };
    std::string           name {};
    std::set<std::string> dependencies;
    TypeDefinition        definition;

    TypeDef(TypeDefKind kind, std::string_view const &name)
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

    static TypeDef &get(std::string_view const &name)
    {
        std::string n { name };
        assert(s_typedefs.contains(n));
        return s_typedefs.at(n);
    }

    static bool has(std::string_view const &name)
    {
        return s_typedefs.contains(std::string { name });
    }

    static TypeDef &make_interface(std::string_view const &name)
    {
        auto &td = make(TypeDefKind::Interface, name);
        td.definition.emplace<Interface>();
        return td;
    }

    static TypeDef &make_alias(std::string_view const &name, pType type)
    {
        auto &td = make(TypeDefKind::Alias, name);
        td.definition = std::move(type);
        return td;
    }

    static TypeDef &make_enumeration(std::string_view const &name)
    {
        auto &td = make(TypeDefKind::Enumeration, name);
        td.definition.emplace<Enumeration>();
        return td;
    }

    static TypeDef &make_enumeration(std::string_view const &name, Enumeration e)
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
    static TypeDef &make(TypeDefKind kind, std::string_view const &name)
    {
        std::string n { name };
        if (s_typedefs.contains(n)) {
            fatal("Type '{}' already registered", n);
        }
        auto const &p = s_typedefs.try_emplace(std::string { name }, kind, name);
        assert(p.second);
        return (*(p.first)).second;
    }

    static std::map<std::string, TypeDef> s_typedefs;
};

class Module {
private:
    class Sentinel { };

public:
    std::string name;
    StringList  types;

    Module(Sentinel, std::string_view const &modname)
        : name(modname)
    {
    }

    JSONValue encode() const;
    auto      cbegin() { return TypeDef::cbegin(); }
    auto      cend() { return TypeDef::cend(); }
    auto      begin() { return TypeDef::cbegin(); }
    auto      end() { return TypeDef::cend(); }

    static Module &make(std::string_view const &modname);

private:
    static std::vector<Module> s_modules;
};

class Parser {
public:
    using TSLexer = LibCore::Lexer<std::string_view, TSKeyword>;
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

extern void generate_typedef(std::string_view const &name);

}

namespace LibCore {

using namespace TSParser;

template<>
inline std::map<std::string_view, TSKeyword> get_keywords()
{
    return std::map<std::string_view, TSKeyword> {
#undef S
#define S(kw, str) { str, TSKeyword::kw },
        TS_KEYWORDS(S)
#undef S
    };
}

}
