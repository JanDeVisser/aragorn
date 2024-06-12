/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <filesystem>

#include "LibCore/StringUtil.h"
#include "config.h"
#include <LSP/TSParser/TSParser.h>
#include <LibCore/IO.h>
#include <LibCore/Options.h>

namespace TSParser {

char const *BasicType_name(BasicType basic_type)
{
    switch (basic_type) {
    case BasicType::Any:
        return "LSPAny";
    case BasicType::Bool:
        return "boolean";
    case BasicType::Int:
        return "integer";
    case BasicType::Null:
        return "null";
    case BasicType::String:
        return "string";
    case BasicType::Unsigned:
        return "uinteger";
    default:
        UNREACHABLE();
    }
}

char const *TypeDefKind_name(TypeDefKind kind)
{
    switch (kind) {
    case TypeDefKind::Interface:
        return "interface";
    case TypeDefKind::Alias:
        return "alias";
    case TypeDefKind::Enumeration:
        return "enumeration";
    case TypeDefKind::Variant:
        return "variant";
    default:
        UNREACHABLE();
    }
}

/*
 * ---------------------------------------------------------------------------
 * -- ConstantType
 * ---------------------------------------------------------------------------
 */

JSONValue ConstantType::encode() const
{
    JSONValue ret = JSONValue::object();
    ret["type"] = BasicType_name(type);
    switch (type) {
    case BasicType::String:
        ret[BasicType_name(type)] = std::get<std::string>(value);
        ret["ctype"] = "std::string";
        break;
    case BasicType::Int:
        ret[BasicType_name(type)] = std::get<int>(value);
        ret["ctype"] = "int";
        break;
    case BasicType::Unsigned:
        ret[BasicType_name(type)] = std::get<int>(value);
        ret["ctype"] = "uint32_t";
        break;
    case BasicType::Bool:
        ret[BasicType_name(type)] = std::get<bool>(value);
        ret["ctype"] = "bool";
        break;
    case BasicType::Null:
        ret[BasicType_name(type)] = JSONValue();
        ret["ctype"] = "Null";
        break;
    default:
        fatal("Cannot serialize constants with basic type {}", BasicType_name(type));
    }
    return ret;
}

/*
 * ---------------------------------------------------------------------------
 * -- Type
 * ---------------------------------------------------------------------------
 */

JSONValue Type::encode() const
{
    auto basic_type_serialize = [this]() -> JSONValue {
        auto ret = JSONValue::object();
        auto basic_type = std::get<BasicType>(definition);
        ret["type"] = BasicType_name(basic_type);
        switch (basic_type) {
        case BasicType::Any:
            ret["ctype"] = "JSONValue";
            break;
        case BasicType::String:
            ret["ctype"] = "std::string";
            break;
        case BasicType::Int:
            ret["ctype"] = "int";
            break;
        case BasicType::Unsigned:
            ret["ctype"] = "uint32_t";
            break;
        case BasicType::Bool:
            ret["ctype"] = "bool";
            break;
        case BasicType::Null:
            ret["ctype"] = "Null";
            break;
        default:
            fatal("Cannot serialize objects with basic type {}", BasicType_name(basic_type));
        }
        return ret;
    };

    JSONValue ret = JSONValue::object();
    switch (kind) {
    case TypeKind::Basic:
        ret["kind"] = "basic_type";
        ret["basic_type"] = basic_type_serialize();
        break;
    case TypeKind::Type: {
        ret["kind"] = "typeref";
        auto typeref = JSONValue::object();
        typeref["type"] = std::get<std::string>(definition);
        typeref["ctype"] = typeref["type"];
        ret["typeref"] = typeref;
    } break;
    case TypeKind::Constant:
        ret["kind"] = "constant";
        ret["constant"] = std::get<ConstantType>(definition).encode();
        break;
    case TypeKind::Variant: {
        ret["kind"] = "variant";
        ret["options"] = std::get<Variant>(definition).encode();
    } break;
    case TypeKind::Struct: {
        ret["kind"] = "struct";
        ret["properties"] = std::get<Interface>(definition).encode_properties();
    } break;
    default:
        UNREACHABLE();
    }
    ret["array"] = array;
    return ret;
}

/*
 * ---------------------------------------------------------------------------
 * -- Variant
 * ---------------------------------------------------------------------------
 */

JSONValue Variant::encode() const
{
    auto ret = JSONValue::array();
    for (auto const &opt : options) {
        ret += opt->encode();
    }
    return ret;
}

/*
 * ---------------------------------------------------------------------------
 * -- Property
 * ---------------------------------------------------------------------------
 */

JSONValue Property::encode() const
{
    auto prop = JSONValue::object();
    prop["name"] = name;
    prop["optional"] = optional;
    prop["type"] = type->encode();
    return prop;
}

/*
 * ---------------------------------------------------------------------------
 * -- Interface
 * ---------------------------------------------------------------------------
 */

void Interface::add_properties(JSONValue &props) const
{
    for (auto const &e : extends) {
        auto const &base = TypeDef::get(e);
        assert(base.kind == TypeDefKind::Interface);
        std::get<Interface>(base.definition).add_properties(props);
    }
    for (auto const &p : properties) {
        props += p.encode();
    }
}

JSONValue Interface::encode_properties() const
{
    auto props = JSONValue::array();
    add_properties(props);
    return props;
}

JSONValue Interface::encode() const
{
    auto ret = JSONValue::object();
    auto extends = JSONValue::array();
    for (auto const &e : extends) {
        extends.append(JSONValue(e));
    }
    ret["extends"] = extends;
    ret["properties"] = encode_properties();
    return ret;
}

/*
 * ---------------------------------------------------------------------------
 * -- Enumeration
 * ---------------------------------------------------------------------------
 */

JSONValue Enumeration::encode() const
{
    auto ret = JSONValue::object();
    ret["type"] = BasicType_name(value_type);
    auto vals = JSONValue::array();
    for (auto const &value : values) {
        auto val = JSONValue::object();
        val["name"] = value.name;
        auto capitalized = value.name;
        capitalized[0] = toupper(capitalized[0]);
        val["capitalized"] = capitalized;
        val["type"] = BasicType_name(value_type);
        switch (value_type) {
        case BasicType::Int:
        case BasicType::Unsigned:
            val[BasicType_name(value_type)] = std::get<int>(value.value);
            break;
        case BasicType::String:
            val[BasicType_name(value_type)] = std::get<std::string>(value.value);
            break;
        default:
            UNREACHABLE();
        }
        vals.append(val);
    }
    ret["values"] = vals;
    return ret;
}

/*
 * ---------------------------------------------------------------------------
 * -- TypeDef
 * ---------------------------------------------------------------------------
 */

std::map<std::string, TypeDef> TypeDef::s_typedefs {};

JSONValue TypeDef::encode() const
{
    auto ret = JSONValue::object();
    ret["name"] = name;
    auto deps = JSONValue::array();
    for (auto const &dep : dependencies) {
        deps.append(dep);
    }
    ret["dependencies"] = deps;
    switch (kind) {
    case TypeDefKind::Alias: {
        ret["kind"] = "alias";
        ret["alias"] = std::get<pType>(definition)->encode();
    } break;
    case TypeDefKind::Interface: {
        ret["kind"] = "interface";
        ret["interface"] = std::get<Interface>(definition).encode();
    } break;
    case TypeDefKind::Enumeration: {
        ret["kind"] = "enumeration";
        ret["enumeration"] = std::get<Enumeration>(definition).encode();
    } break;
    case TypeDefKind::Variant: {
        ret["kind"] = "variant";
        ret["variant"] = std::get<Variant>(definition).encode();
    } break;
    default:
        UNREACHABLE();
    }
    return ret;
}

void TypeDef::get_dependencies(pType const &type)
{
    switch (type->kind) {
    case TypeKind::Type:
        dependencies.insert(type->type_name());
        break;
    case TypeKind::Struct: {
        for (auto const &prop : type->interface().properties) {
            get_dependencies(prop.type);
        }
    } break;
    case TypeKind::Variant: {
        for (auto const &option : type->variant().options) {
            get_dependencies(option);
        }
    } break;
    default:
        break;
    }
}

/*
 * ---------------------------------------------------------------------------
 * -- Module
 * ---------------------------------------------------------------------------
 */

std::vector<Module> Module::s_modules;

JSONValue Module::encode() const
{
    auto ret = JSONValue::object();
    ret["name"] = name;
    auto types_array = JSONValue::array();
    for (auto const &type_name : types) {
        auto const &type_def = TypeDef::get(name);
        types_array += type_def.encode();
    }
    ret["types"] = types_array;
    return ret;
}

Module &Module::make(std::string_view const &modname)
{
    return s_modules.emplace_back(Sentinel {}, modname);
}

/*
 * ---------------------------------------------------------------------------
 * -- Parser
 * ---------------------------------------------------------------------------
 */

namespace fs = std::filesystem;

Parser::Parser(std::string_view const &fname)
    : file_name(fname)
    , m_module(Module::make(fs::path { fname }.stem().string()))
{
}

pType Parser::parse_type()
{
    pType ret;
    while (true) {
        auto        type = std::make_shared<Type>();
        auto const &t = m_lexer.lex();
        switch (t.kind) {
        case TokenKind::Symbol: {
            switch (t.symbol_code()) {
            case '{': {
                type->kind = TypeKind::Struct;
                type->definition.emplace<Interface>();
                parse_struct(std::get<Interface>(type->definition));
                break;
            };
            default:
                fatal("Expected type specification, but got '{}'", t.text);
            }
        } break;
        case TokenKind::Identifier: {
            // interface/enum type
            type->kind = TypeKind::Type;
            type->definition.emplace<std::string>(t.text);
        } break;
        case TokenKind::Keyword: {
            type->kind = TypeKind::Basic;
            BasicType basic_type { BasicType::None };
            switch (t.keyword_code()) {
            case TSKeyword::LSPAny:
                basic_type = BasicType::Any;
                break;
            case TSKeyword::Boolean:
                basic_type = BasicType::Bool;
                break;
            case TSKeyword::Integer:
                basic_type = BasicType::Int;
                break;
            case TSKeyword::String:
                basic_type = BasicType::String;
                break;
            case TSKeyword::Null:
                type->kind = TypeKind::Constant;
                type->definition.emplace<ConstantType>(BasicType::Null);
                break;
            case TSKeyword::UInteger:
                basic_type = BasicType::Unsigned;
                break;
            default:
                fatal("Expected type specification, but got '{}'", t.text);
            }
            if (basic_type != BasicType::None) {
                type->definition.emplace<BasicType>(basic_type);
            }
        } break;
        case TokenKind::Number: {
            type->kind = TypeKind::Constant;
            type->definition.emplace<ConstantType>(must_string_to_integer<int>(t.text));
        } break;
        case TokenKind::QuotedString: {
            type->kind = TypeKind::Constant;
            type->definition.emplace<ConstantType>(t.text.substr(1, t.text.length() - 2));
        } break;
        default:
            fatal("Expected type specification, but got '{}'", t.text);
        }
        if (m_lexer.accept_symbol('[')) {
            MUST(m_lexer.expect_symbol(']'));
            type->array = true;
        }
        if (m_lexer.accept_symbol(';')) {
            if (ret != nullptr && ret->kind == TypeKind::Variant) {
                ret->variant().options.push_back(type);
            } else {
                ret = type;
            }
            return ret;
        } else if (m_lexer.accept_symbol('|')) {
            if (ret == nullptr) {
                ret = std::make_shared<Type>();
                ret->kind = TypeKind::Variant;
                ret->definition.emplace<Variant>();
            } else {
                assert(ret->kind == TypeKind::Variant);
            }
            ret->variant().options.push_back(type);
        } else {
            fatal("Expected '|' or ';', got '{}'", t.text);
        }
    }
}

void Parser::parse_struct(Interface &interface)
{
    if (m_lexer.accept(TokenKind::EndOfFile)) {
        fatal("Expected field definition or '}'");
    }
    if (m_lexer.accept_symbol('}')) {
        return;
    }
    while (true) {
        auto const &name = MUST_EVAL(m_lexer.expect_identifier());
        auto       &prop = interface.properties.emplace_back(name.text);

        if (m_lexer.accept_symbol('?')) {
            prop.optional = true;
        }
        MUST(m_lexer.expect_symbol(':'));
        prop.type = parse_type();
        if (m_lexer.accept_symbol('}')) {
            break;
        }
    }
}

void Parser::parse_interface()
{
    m_lexer.lex();
    auto const &name = MUST_EVAL(m_lexer.expect_identifier());
    auto       &type_def = TypeDef::make_interface(name.text);
    auto       &interface = type_def.interface();
    if (m_lexer.accept_keyword(TSKeyword::Extends)) {
        do {
            Token base_interface = MUST_EVAL(m_lexer.expect_identifier());
            interface.extends.push_back(base_interface.text);
            type_def.dependencies.insert(base_interface.text);
            if (m_lexer.accept_symbol('{')) {
                break;
            }
            MUST(m_lexer.expect_symbol(','));
        } while (true);
    } else {
        MUST(m_lexer.expect_symbol('{'));
    }
    parse_struct(interface);
    // Make dependencies std::set!
    // Then type.get_dependencies(type_def);
    for (auto const &prop : type_def.interface().properties) {
        type_def.get_dependencies(prop.type);
    }
}

void Parser::parse_namespace()
{
    m_lexer.lex();
    auto const &name = MUST_EVAL(m_lexer.expect_identifier());
    Enumeration enumeration {};
    MUST(m_lexer.expect_symbol('{'));
    while (true) {
        auto const &name_token = m_lexer.lex();
        if (name_token.matches_keyword(TSKeyword::Export) || name_token.matches_keyword(TSKeyword::Const)) {
            continue;
        }
        if (!name_token.is_identifier()) {
            fatal("Expected value name, got '{}'", name_token);
        }
        MUST(m_lexer.expect_symbol(':'));
        auto const &token = m_lexer.lex();
        BasicType   value_type = BasicType::None;
        switch (token.kind) {
        case TokenKind::Keyword: {
            switch (token.keyword_code()) {
            case TSKeyword::Integer:
                value_type = BasicType::Int;
                break;
            case TSKeyword::UInteger:
                value_type = BasicType::Unsigned;
                break;
            case TSKeyword::String:
                value_type = BasicType::String;
                break;
            default:
                fatal("Expected integer number, 'integer', or 'uinteger', got '{}'", token);
            }
        } break;
        case TokenKind::Number: {
            value_type = BasicType::Int;
        } break;
        case TokenKind::Identifier: {
            if (!TypeDef::has(token.text)) {
                fatal("Unexpected identifier '{}' in type specification of enumeration value '{}::{}'",
                    token.text, name.text, name_token.text);
            }
            auto const &type_def = TypeDef::get(token.text);
            assert(type_def.kind == TypeDefKind::Alias);
            assert(type_def.alias_for()->kind == TypeKind::Basic);
            value_type = type_def.alias_for()->basic_type();
            break;
        }
        default:
            fatal("Unexpected enumeration value type specification for enumeration value '{}::{}': '{}'",
                name.text, name_token.text, token.text);
        }

        if (enumeration.value_type != BasicType::None && value_type != BasicType::None && value_type != enumeration.value_type) {
            fatal("Value type mismatch in enumeration '{}'", name.text);
        }

        MUST(m_lexer.expect_symbol('='));

        auto const &value_token = m_lexer.lex();
        switch (value_token.kind) {
        case TokenKind::Number: {
            if (value_type != BasicType::None && value_type != BasicType::Int) {
                fatal("Value type mismatch in enumeration '{}'", name.text);
            }
            value_type = BasicType::Int;
            auto const number_maybe = string_to_integer<int>(value_token.text);
            if (!number_maybe.has_value()) {
                fatal("Unparseable numeric constant '{}'", value_token.text);
            }
            enumeration.values.emplace_back(name_token.text, number_maybe.value());
        } break;
        case TokenKind::QuotedString: {
            if (value_type != BasicType::None && value_type != BasicType::String) {
                fatal("Value type mismatch in enumeration '{}'", name.text);
            }
            value_type = BasicType::String;
            enumeration.values.emplace_back(name_token.text, token.text.substr(1, token.text.length() - 2));
        } break;
        default:
            fatal("Unexpected value for enumeration value '{}::{}': '{}'",
                name.text, name_token.text, token.text);
        }
        if (enumeration.value_type != BasicType::None && value_type != enumeration.value_type) {
            fatal("Value type mismatch in enumeration '{}'", name.text);
        }
        if (enumeration.value_type == BasicType::None) {
            enumeration.value_type = value_type;
        }
        MUST(m_lexer.expect_symbol(';'));
        if (m_lexer.accept_symbol('}')) {
            break;
        }
    }

    if (TypeDef::has(name.text)) {
        auto &td = TypeDef::get(name.text);
        switch (td.kind) {
        case TypeDefKind::Alias: {
            if (td.alias_for()->kind != TypeKind::Basic) {
                fatal("Enumeration '{}' is typedef'ed to a non-basic type", name.text);
            }
            if (td.alias_for()->basic_type() != enumeration.value_type) {
                fatal("Enumeration '{}' is typedef'ed to a different basic type", name.text);
            }
            td.kind = TypeDefKind::Enumeration;
            td.definition = enumeration;
        } break;
        default:
            fatal("Duplicate type definition '{}'", name.text);
        }
    } else {
        TypeDef::make_enumeration(name.text, enumeration);
    }
}

void Parser::parse_enumeration()
{
    m_lexer.lex();
    auto const &name = MUST_EVAL(m_lexer.expect_identifier());
    auto       &enumeration_def = TypeDef::make_enumeration(name.text);
    auto       &enumeration = enumeration_def.enumeration();
    MUST(m_lexer.expect_symbol('{'));
    while (true) {
        auto const &name_token = m_lexer.peek();
        if (!name_token.is_identifier() && name_token.kind != TokenKind::Keyword) {
            fatal("Expected enum value name, got '{}'", name_token.text);
        }
        m_lexer.lex();
        MUST(m_lexer.expect_symbol('='));

        BasicType   value_type;
        auto const &value_token = m_lexer.lex();
        switch (value_token.kind) {
        case TokenKind::Number: {
            value_type = BasicType::Int;
            auto const &number_maybe = string_to_integer<int>(value_token.text);
            assert(number_maybe.has_value());
            enumeration.values.emplace_back(name_token.text, number_maybe.value());
        } break;
        case TokenKind::QuotedString: {
            value_type = BasicType::String;
            enumeration.values.emplace_back(name_token.text, value_token.text.substr(1, value_token.text.length() - 2));
        } break;
        default:
            fatal("Unexpected value for enumeration value '{}::{}': '{}'",
                enumeration_def.name, name_token.text, value_token.text);
        }

        if (enumeration.value_type != BasicType::None && value_type != enumeration.value_type) {
            fatal("Value type mismatch in enumeration '{}'", enumeration_def.name);
        }
        if (enumeration.value_type == BasicType::None) {
            enumeration.value_type = value_type;
        }
        if (m_lexer.accept_symbol('}')) {
            break;
        }
        if (!m_lexer.expect_symbol(',')) {
            fatal("Expected ',' or '}'. Got '{}'", m_lexer.peek().text);
        }
        if (m_lexer.accept_symbol('}')) {
            break;
        }
    }
}

// FIXME This makes no sense
void Parser::parse_typedef()
{
    m_lexer.lex();
    bool        is_basic_type = false;
    auto const &name = m_lexer.lex();
    switch (name.kind) {
    case TokenKind::Identifier:
        break;
    case TokenKind::Keyword: {
        switch (name.keyword_code()) {
        case TSKeyword::Integer:
        case TSKeyword::UInteger:
        case TSKeyword::Decimal:
            is_basic_type = true;
            break;
        default:
            fatal("Expected type definition name, got '{}'", name.text);
        }
    } break;
    default:
        fatal("Expected type definition name, got '{}'", name.text);
    }

    MUST(m_lexer.expect_symbol('='));
    auto t = parse_type();
    if (TypeDef::has(name.text)) {
        auto const &td = TypeDef::get(name.text);
        switch (td.kind) {
        case TypeDefKind::Enumeration: {
            if (t->kind != TypeKind::Basic) {
                fatal("Enumeration '{}' is typedef'ed to a non-basic type", name.text);
            }
            Enumeration enumeration = std::get<Enumeration>(td.definition);
            if (enumeration.value_type != t->basic_type()) {
                fatal("Enumeration '{}' is typedef'ed to a different basic type", name.text);
            }
        } break;
        default:
            fatal("Duplicate type definition '{}'", name.text);
        }
    } else if (!is_basic_type) {
        auto &type_def = TypeDef::make_alias(name.text, t);
        type_def.get_dependencies(t);
    }
}

Module const &Parser::parse()
{
    auto const &buffer { MUST_EVAL(read_file_by_name(file_name)) };
    m_lexer.push_source(buffer, file_name);

    while (true) {
        auto &token = m_lexer.peek();
        if (token.kind == TokenKind::EndOfFile) {
            break;
        }
        if (token.matches_keyword(TSKeyword::Interface)) {
            parse_interface();
        } else if (token.matches_keyword(TSKeyword::Namespace)) {
            parse_namespace();
        } else if (token.matches_keyword(TSKeyword::Enum)) {
            parse_enumeration();
        } else if (token.matches_keyword(TSKeyword::Type)) {
            parse_typedef();
        } else {
            m_lexer.lex();
        }
    }
    return m_module;
}
}

using namespace LibCore;
using namespace TSParser;

int main(int argc, char const **argv)
{
    auto app_args = parse_options(argc, argv);

    if (app_args >= argc) {
        printf("No module specified\n");
        exit(1);
    }
    auto ts_module = argv[app_args];
    std::cout << "Reading " << ts_module << "\n";
    Parser parser { ts_module };
    Module module = parser.parse();

#if 0
    auto mod = JSONValue::array();
    for (auto const &name : module.types) {
        auto const &type_def = TypeDef::get(name);
        auto        typedescr = JSONValue::object();
        typedescr["name"] = name;
        typedescr["kind"] = TypeDefKind_name(type_def.kind);
        mod += typedescr;
    }
    auto const &json = mod.serialize(true);
    auto const &json_file = std::format("{}.json", module.name);

    MUST(write_file_by_name(json_file, json));

    for (auto const &name : module.types) {
        std::cout << "Generating " << name << "\n";
        generate_typedef(module.name);
    }
    return 0;
#endif
}
