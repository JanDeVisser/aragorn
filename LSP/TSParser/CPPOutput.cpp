/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>

#include "config.h"
#include <LSP/TSParser/TSParser.h>

namespace TSParser {

struct License {
    static constexpr auto text { R"(/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

)" };
    static License        MIT;

    friend std::ostream &operator<<(std::ostream &os, License const &)
    {
        return os << License::text;
    }

private:
    License() = default;
};

License License::MIT;

void emit_string_enum_header(std::ostream &os, TypeDef const &type)
{
    auto &e = type.enumeration();

    os << License::MIT << R"(#pragma once

#include <LSP/Schema/LSPBase.h>
)";
    for (auto const &dep : type.dependencies) {
        os << "#include <LSP/Schema/" << dep << ".h>\n";
    }

    os << std::format(R"(
namespace LSP {{

enum class {} {{
)",
        type.name);

    for (auto const &value : e.values) {
        os << std::format("    {},\n", capitalize(value.name));
    }
    os << std::format(R"(}};

template<>
inline std::string as_string({} obj)
{{
    switch (obj) {{
)",
        type.name);
    for (auto const &value : e.values) {
        os << std::format(R"(    case {}::{}: return "{}";
)",
            type.name, capitalize(value.name), std::get<std::string>(value.value));
    }
    os << std::format(R"(    default: return "unknown";
    }}
}}

template<>
inline std::optional<{}> from_string<{}>(std::string_view const& s)
{{
)",
        type.name, type.name);
    for (auto const &value : e.values) {
        os << std::format(R"(    if (s == "{}") return {}::{};
)",
            std::get<std::string>(value.value), type.name, capitalize(value.name));
    }
    os << std::format(R"(    return {{}};
}}

}} /* namespace LSP */

namespace LibCore {{

using namespace LSP;

template<>
inline JSONValue encode({0} const &obj)
{{
    return encode_string_enum<{0}>(obj);
}}

template<>
inline Result<{0}, JSONError> decode(JSONValue const &json)
{{
    return decode_string_enum<{0}>(json);
}}

}} /* namespace LibCore */
)",
        type.name);
}

void emit_int_enum_header(std::ostream &os, TypeDef const &type)
{
    auto &e = type.enumeration();

    os << License::MIT << R"(#pragma once

#include <LSP/Schema/LSPBase.h>
)";
    for (auto const &dep : type.dependencies) {
        os << "#include <LSP/Schema/" << dep << ".h>\n";
    }

    os << std::format(R"(
namespace LSP {{

enum class {} {{
)",
        type.name);

    for (auto const &value : e.values) {
        os << std::format("    {} = {},\n", capitalize(value.name), std::get<int>(value.value));
    }
    os << std::format(R"(}};

template<>
inline std::optional<{}> from_int<{}>(int i)
{{
)",
        type.name, type.name);
    for (auto const &value : e.values) {
        os << std::format(R"(    if (i == {}) return {}::{};
)",
            std::get<int>(value.value), type.name, capitalize(value.name));
    }
    os << std::format(R"(    return {{}};
}}

}} /* namespace LSP */

namespace LibCore {{

using namespace LSP;

template<>
inline JSONValue encode({0} const &obj)
{{
    return encode_int_enum(obj);
}}

template<>
inline Result<{0}, JSONError> decode(JSONValue const &json)
{{
    return decode_int_enum(json);
}}

}} /* namespace LibCore */
)",
        type.name);
}

void emit_prop_def(std::ostream &os, Property const &prop);

void render_type(std::ostream &os, pType type)
{
    if (type->array) {
        os << "std::vector<";
    }
    switch (type->kind) {
    case TypeKind::Type: {
        os << type->type_name();
    } break;
    case TypeKind::Basic: {
        os << BasicType_name(type->basic_type());
    } break;
    case TypeKind::Struct: {
        assert(!type->synthetic_name.empty());
        os << type->synthetic_name;
    } break;
    case TypeKind::Variant: {
        char sep = '<';
        os << "std::variant";
        for (auto const &t : type->variant().options) {
            os << sep;
            render_type(os, t);
            sep = ',';
        }
        os << ">";
    } break;
    default:
        std::cout << "render_type(" << TypeKind_name(type->kind) << ")\n";
        UNREACHABLE();
    }
    if (type->array) {
        os << ">";
    }
}

void inline_struct(std::ostream &os, std::string_view const &name, Interface const &iface)
{
    os << "struct " << name << " ";
    if (iface.extends.empty()) {
        os << ": public LSPObject ";
    } else {
        char sep = ':';
        for (auto const &ext : iface.extends) {
            os << std::format("{:c} public {} ", sep, ext);
            sep = ',';
        }
    }
    os << "{\n";
    for (auto const &prop : iface.properties) {
        emit_prop_def(os, prop);
    }
    os << "\nstatic Result<" << name << ", JSONError> decode(JSONValue const& json) {\n"
       << name << " ret;\n";
    for (auto const &prop : iface.properties) {
	if (prop.optional) {
            os << "        if (json.has(\"" << prop.name << "\") {\n";
	}
        os << "        ret." << prop.name << " = TRY_EVAL(json.try_get<";
	render_type(os, prop.type);
        os << ">(" << prop.name << "));\n";
	if (prop.optional) {
            os << "        }\n";
	}
    }
    os << "         return ret;\n";
    os << "    }\n\n";

    os << "JSONValue encode() {\n";
    for (auto const &prop : iface.properties) {
        os << "set(ret, \"" << prop.name << "\", ";
        os << "encode<";
        if (prop.optional) {
	    os << "std::optional<";
	}
        render_type(os, prop.type);
        if (prop.optional) {
	    os << ">";
	}
	os << ">(" << prop.name << "));\n";
    }
    os << "JSONValue ret;\n";  
    os << "};\n";
    os << "};\n";
}

void emit_prop_def(std::ostream &os, Property const &prop)
{
    if (prop.type->kind == TypeKind::Struct) {
        prop.type->synthetic_name = capitalize(prop.name);
        inline_struct(os, prop.type->synthetic_name, prop.type->interface());
    }
    if (prop.type->kind == TypeKind::Variant) {
        auto const &variant = prop.type->variant();
        auto        ix = 0u;
        for (auto const &option : variant.options) {
            if (option->kind == TypeKind::Struct) {
                option->synthetic_name = std::format("{}_{}", capitalize(prop.name), ix);
                inline_struct(os, option->synthetic_name, option->interface());
            }
            ++ix;
        }
    }
    os << "    ";
    if (prop.optional) {
        os << "std::optional<";
    }
    render_type(os, prop.type);
    if (prop.optional) {
        os << ">";
    }
    os << " " << prop.name << ";\n";
}

void emit_interface_header(std::ostream &os, TypeDef const &type)
{
    auto &iface = type.interface();

    os << License::MIT << R"(#pragma once

#include <LSP/Schema/LSPBase.h>
)";
    for (auto const &dep : type.dependencies) {
        os << "#include <LSP/Schema/" << dep << ".h>\n";
    }

    os << std::format(R"(
namespace LSP {{

)");
    inline_struct(os, type.name, iface);
    os << R"(

} /* namespace LSP */
)";
}

void emit_variant_header(std::ostream &os, TypeDef const &type)
{
    auto &variant = type.variant();

    os << License::MIT << R"(#pragma once

#include <LSP/Schema/LSPBase.h>
)";
    for (auto const &dep : type.dependencies) {
        os << "#include <LSP/Schema/" << dep << ".h>\n";
    }

    os << R"(
namespace LSP {{

)";

    auto        ix = 0u;
    for (auto const &option : variant.options) {
	if (option->kind == TypeKind::Struct) {
	    option->synthetic_name = std::format("{}_{}", capitalize(type.name), ix);
	    inline_struct(os, option->synthetic_name, option->interface());
	}
	++ix;
    }
    os << std::format("using {} = std::variant", type.name);
    ix = 0;
    auto sep = '<';
    for (auto const &option : variant.options) {
        os << sep;
	render_type(os, option);
	sep = ',';
    }
    os << R"(>;

} /* namespace LSP */
)";
}

struct Header {
    TypeDef const &type;

    friend std::ostream &operator<<(std::ostream &os, Header const &self)
    {
        switch (self.type.kind) {
        case TypeDefKind::Enumeration: {
            switch (self.type.enumeration().value_type) {
            case BasicType::String:
                emit_string_enum_header(os, self.type);
                break;
            case BasicType::Int:
                emit_int_enum_header(os, self.type);
                break;
            default:
                UNREACHABLE();
            }
        } break;
        case TypeDefKind::Interface:
            emit_interface_header(os, self.type);
            break;
        case TypeDefKind::Variant: {
	    emit_variant_header(os, self.type);
	} break;
        case TypeDefKind::Alias:
            break;
        default:
            UNREACHABLE();
        }
        return os;
    }
};

void CPPOutputter::output()
{
    Header        h { m_type };
    std::ofstream os { std::format("{}.h", m_type.name), std::ios::trunc | std::ios::out };
    os << h;
    os.close();
    std::system(std::format("clang-format -i {}.h", m_type.name).c_str());
}

}
