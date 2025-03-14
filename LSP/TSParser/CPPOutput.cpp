/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "config.h"

#include <LSP/TSParser/TSParser.h>
#include <LibCore/Utf8.h>

namespace TSParser {

struct License {
    static constexpr auto text { LR"(/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

)" };
    static License        MIT;

    friend std::wostream &operator<<(std::wostream &os, License const &)
    {
        return os << License::text;
    }

private:
    License() = default;
};

License License::MIT;

void emit_string_enum_header(std::wostream &os, TypeDef const &type)
{
    auto &e = type.enumeration();

    os << License::MIT << LR"(#pragma once

#include <LSP/Schema/LSPBase.h>
)";
    for (auto const &dep : type.dependencies) {
        os << L"#include <LSP/Schema/" << dep << L".h>\n";
    }

    os << std::format(LR"(
namespace LSP {{

enum class {} {{
)",
        type.name);

    for (auto const &value : e.values) {
        os << std::format(L"    {},\n", capitalize(value.name));
    }
    os << std::format(LR"(}};

inline std::string {0}_as_string({0} obj)
{{
    switch (obj) {{
)",
        type.name);
    for (auto const &value : e.values) {
        os << std::format(LR"(    case {}::{}: return "{}";
)",
            type.name, capitalize(value.name), std::get<std::wstring>(value.value));
    }
    os << std::format(LR"(    default: return "unknown";
    }}
}}

inline std::optional<{0}> {0}_from_string(std::string_view const& s)
{{
)",
        type.name);
    for (auto const &value : e.values) {
        os << std::format(LR"(    if (s == "{}") return {}::{};
)",
            std::get<std::wstring>(value.value), type.name, capitalize(value.name));
    }
    os << std::format(LR"(    return {{ }};
}}

}} /* namespace LSP */

namespace LibCore {{

using namespace LSP;

template<>
inline JSONValue encode({0} const &obj)
{{
    return JSONValue {{ {0}_as_string(obj) }};
}}

template<>
inline Decoded<{0}> decode(JSONValue const &json)
{{
    return {0}_from_string(json.to_string());
}}

}} /* namespace LibCore */
)",
        type.name);
}

void emit_int_enum_header(std::wostream &os, TypeDef const &type)
{
    auto &e = type.enumeration();

    os << License::MIT << LR"(#pragma once

#include <LSP/Schema/LSPBase.h>
)";
    for (auto const &dep : type.dependencies) {
        os << L"#include <LSP/Schema/" << dep << L".h>\n";
    }

    os << std::format(LR"(
namespace LSP {{

enum class {} {{
)",
        type.name);

    for (auto const &value : e.values) {
        os << std::format(L"    {} = {},\n", capitalize(value.name), std::get<int>(value.value));
    }
    os << std::format(LR"(}};

inline std::optional<{0}> {0}_from_int(int i)
{{
)",
        type.name);
    for (auto const &value : e.values) {
        os << std::format(LR"(    if (i == {}) return {}::{};
)",
            std::get<int>(value.value), type.name, capitalize(value.name));
    }
    os << std::format(LR"(    return {{ }};
}}

}} /* namespace LSP */

namespace LibCore {{

using namespace LSP;

template<>
inline JSONValue encode({0} const &obj)
{{
    return JSONValue {{ static_cast<int>(obj) }};
}}

template<>
inline Decoded<{0}> decode(JSONValue const &json)
{{
    int int_val;
    TRY(json.convert(int_val));
    if (auto v = {0}_from_int(int_val); !v) {{
        return JSONError {{
            JSONError::Code::UnexpectedValue,
            "Cannot convert JSON value of type '{0}' to integer",
        }};
    }} else {{
        return *v;
    }}
}}

}} /* namespace LibCore */
)",
        type.name);
}

void emit_prop_def(std::wostream &os, Property const &prop);

void render_type(std::wostream &os, pType type, bool bare = false)
{
    if (type->array && !bare) {
        os << L"std::vector<";
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
        wchar_t sep = '<';
        if (!bare) {
            os << L"std::variant";
        }
        for (auto const &t : type->variant().options) {
            os << sep;
            render_type(os, t);
            sep = ',';
        }
        if (!bare) {
            os << L">";
        }
    } break;
    default:
        UNREACHABLE();
    }
    if (type->array && !bare) {
        os << L">";
    }
}

void inline_struct(std::wostream &os, std::wstring_view const &name, Interface const &iface)
{
    os << L"struct " << name << L" ";
    if (!iface.extends.empty()) {
        char sep = ':';
        for (auto const &ext : iface.extends) {
            os << std::format(L"{:c} public {} ", sep, ext);
            sep = ',';
        }
    }
    os << L"{\n";
    for (auto const &prop : iface.properties) {
        emit_prop_def(os, prop);
    }
    os << L"\nstatic Decoded<" << name << L"> decode(JSONValue const& json) {\n"
       << name << L" ret;\n";
    for (auto const &prop : iface.properties) {
        if (prop.optional) {
            os << L"        if (json.has(\"" << prop.name << L"\")) {\n";
        }
        if (prop.type->array) {
            os << L"        ret." << prop.name << " = TRY_EVAL(json.try_get_array<";
        } else if (prop.type->kind == TypeKind::Variant) {
            os << L"        ret." << prop.name << " = TRY_EVAL(json.try_get_variant";
        } else {
            os << L"        ret." << prop.name << " = TRY_EVAL(json.try_get<";
        }
        render_type(os, prop.type, true);
        os << L">(\"" << prop.name << L"\"));\n";
        if (prop.optional) {
            os << L"        }\n";
        }
    }
    os << L"         return std::move(ret);\n";
    os << L"    }\n\n";

    os << L"JSONValue encode() const {\n";
    os << L"JSONValue ret { JSONType::Object };\n";
    for (auto const &prop : iface.properties) {
        auto n = MUST_EVAL(to_utf8(prop.name));
        os << L"set(ret, \"" << prop.name << L"\", ";
        os << prop.name << L");\n";
    }
    os << L"return ret;\n";
    os << L"};\n";
    os << L"};\n";
}

void emit_prop_def(std::wostream &os, Property const &prop)
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
                option->synthetic_name = std::format(L"{}_{}", capitalize(prop.name), ix);
                inline_struct(os, option->synthetic_name, option->interface());
            }
            ++ix;
        }
    }
    os << L"    ";
    if (prop.optional) {
        os << L"std::optional<";
    }
    render_type(os, prop.type);
    if (prop.optional) {
        os << L">";
    }
    os << L" " << prop.name << L";\n";
}

void emit_interface_header(std::wostream &os, TypeDef const &type)
{
    auto &iface = type.interface();

    os << License::MIT << LR"(#pragma once

#include <LSP/Schema/LSPBase.h>
)";
    for (auto const &dep : type.dependencies) {
        os << L"#include <LSP/Schema/" << dep << L".h>\n";
    }

    os << std::format(LR"(
namespace LSP {{

)");
    inline_struct(os, type.name, iface);
    os << LR"(

} /* namespace LSP */
)";
}

void emit_variant_header(std::wostream &os, TypeDef const &type)
{
    auto &variant = type.variant();

    os << License::MIT << LR"(#pragma once

#include <LSP/Schema/LSPBase.h>
)";
    for (auto const &dep : type.dependencies) {
        os << L"#include <LSP/Schema/" << dep << L".h>\n";
    }

    os << LR"(
namespace LSP {

)";

    auto ix = 0u;
    for (auto const &option : variant.options) {
        if (option->kind == TypeKind::Struct) {
            option->synthetic_name = std::format(L"{}_{}", capitalize(type.name), ix);
            inline_struct(os, option->synthetic_name, option->interface());
        }
        ++ix;
    }
    os << std::format(L"using {} = std::variant", type.name);
    ix = 0;
    wchar_t sep = '<';
    for (auto const &option : variant.options) {
        os << sep;
        render_type(os, option);
        sep = ',';
    }
    os << LR"(>;

} /* namespace LSP */
)";
}

struct Header {
    TypeDef const &type;

    friend std::wostream &operator<<(std::wostream &os, Header const &self)
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
    Header      h { m_type };
    std::string n { as_utf8(m_type.name) };
    {
        std::wofstream os { std::format("{}.h", n), std::ios::trunc | std::ios::out };
        os << h;
        os.close();
    }
    auto cmd = std::format("clang-format -i {}.h", n);
    std::system(cmd.c_str());
}

}
