/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <filesystem>

#include "config.h"
#include <LSP/TSParser/TSParser.h>
#include <LibCore/IO.h>
#include <LibCore/Options.h>
#include <LibCore/StringUtil.h>

namespace TSParser {

char const *BasicType_name(BasicType basic_type)
{
    switch (basic_type) {
    case BasicType::Any:
        return "LSP::Any";
    case BasicType::Bool:
        return "bool";
    case BasicType::Empty:
        return "LSP::Empty";
    case BasicType::Int:
        return "int";
    case BasicType::Null:
        return "LSP::Null";
    case BasicType::String:
        return "std::string";
    case BasicType::Unsigned:
        return "uint32_t";
    default:
        UNREACHABLE();
    }
}

char const *TypeKind_name(TypeKind kind)
{
    switch (kind) {
#undef S
#define S(K)          \
    case TypeKind::K: \
        return #K;
        TYPEKINDS(S)
#undef S
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
    case BasicType::Empty:

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

    if (has_option("output-cpp")) {
        std::cout << "Generating C++ code" << std::endl;
        for (auto const &type_pair : module) {
            auto &type = type_pair.second;
            std::cout << type.name << "..." << std::endl;
            CPPOutputter outputter { type };
            outputter.output();
        }
    }

    if (has_option("output-json")) {
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

#if 0
        for (auto const &name : module.types) {
            std::cout << "Generating " << name << "\n";
            generate_typedef(module.name);
        }
#endif
    }
    return 0;
}
