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

namespace fs = std::filesystem;

void error_message(LexerErrorMessage const &msg)
{
    fatal("{}:{}: {}", msg.location.line + 1, msg.location.column + 1, msg.message);
}

Parser::Parser(std::string_view const &fname)
    : file_name(fname)
    , m_module(Module::make(fs::path { fname }.stem().wstring()))
{
}

pType Parser::parse_type()
{
    pType ret;
    while (true) {
        auto       type = std::make_shared<Type>();
        auto const t = m_lexer.lex();
        auto       text = MUST_EVAL(to_utf8(m_lexer.text(t)));
        switch (t.kind) {
        case TokenKind::Symbol: {
            switch (t.symbol_code()) {
            case '{': {
                type->kind = TypeKind::Struct;
                type->definition.emplace<Interface>();
                parse_struct(std::get<Interface>(type->definition));
                break;
            };
            default: {
                fatal("Expected type specification, but got '{}'", text);
            } break;
            }
        } break;
        case TokenKind::Identifier: {
            // interface/enum type
            type->kind = TypeKind::Type;
            type->definition.emplace<std::wstring>(m_lexer.text(t));
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
            case TSKeyword::Empty:
                basic_type = BasicType::Empty;
                break;
            case TSKeyword::Integer:
                basic_type = BasicType::Int;
                break;
            case TSKeyword::String:
                basic_type = BasicType::String;
                break;
            case TSKeyword::Null:
                basic_type = BasicType::Null;
                ;
                break;
            case TSKeyword::UInteger:
                basic_type = BasicType::Unsigned;
                break;
            default: {
                fatal("Expected type specification, but got '{}'", text);
            } break;
            }
            if (basic_type != BasicType::None) {
                type->definition.emplace<BasicType>(basic_type);
            }
        } break;
        case TokenKind::Number: {
            type->kind = TypeKind::Constant;
            type->definition.emplace<ConstantType>(must_string_to_integer<int>(text));
        } break;
        case TokenKind::QuotedString: {
            type->kind = TypeKind::Constant;
            type->definition.emplace<ConstantType>(m_lexer.text(t).substr(1, m_lexer.text(t).length() - 2));
        } break;
        default:
            fatal("Expected type specification, but got '{}'", text);
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
            fatal("Expected '|' or ';', got '{}'", text);
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
        auto const name = MUST_EVAL(m_lexer.expect_identifier());
        auto      &prop = interface.properties.emplace_back(m_lexer.text(name));

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
    auto const name = MUST_EVAL(m_lexer.expect_identifier());
    auto      &type_def = TypeDef::make_interface(m_lexer.text(name));
    auto      &interface = type_def.interface();
    if (m_lexer.accept_keyword(TSKeyword::Extends)) {
        do {
            auto const base_interface = MUST_EVAL(m_lexer.expect_identifier());
            interface.extends.emplace_back(m_lexer.text(base_interface));
            type_def.dependencies.emplace(m_lexer.text(base_interface));
            if (m_lexer.accept_symbol('{')) {
                break;
            }
            MUST(m_lexer.expect_symbol(','));
        } while (true);
    } else {
        if (auto res = m_lexer.expect_symbol('{'); res.is_error()) {
            error_message(res.error());
        }
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
            if (!TypeDef::has(m_lexer.text(token))) {
                fatal("Unexpected identifier '{}' in type specification of enumeration value '{}::{}'",
                    m_lexer.text_utf8(token), m_lexer.text_utf8(name), m_lexer.text_utf8(name_token));
            }
            auto const &type_def = TypeDef::get(m_lexer.text(token));
            assert(type_def.kind == TypeDefKind::Alias);
            assert(type_def.alias_for()->kind == TypeKind::Basic);
            value_type = type_def.alias_for()->basic_type();
            break;
        }
        default:
            fatal("Unexpected enumeration value type specification for enumeration value '{}::{}': '{}'",
                m_lexer.text_utf8(name), m_lexer.text_utf8(name_token), m_lexer.text_utf8(token));
        }

        if (enumeration.value_type != BasicType::None && value_type != BasicType::None && value_type != enumeration.value_type) {
            fatal("Value type mismatch in enumeration '{}'", m_lexer.text_utf8(name));
        }

        MUST(m_lexer.expect_symbol('='));

        auto const &value_token = m_lexer.lex();
        switch (value_token.kind) {
        case TokenKind::Number: {
            if (value_type != BasicType::None && value_type != BasicType::Int) {
                fatal("Value type mismatch in enumeration '{}'", m_lexer.text_utf8(name));
            }
            value_type = BasicType::Int;
            auto const number_maybe = string_to_integer<int>(m_lexer.text_utf8(value_token));
            if (!number_maybe.has_value()) {
                fatal("Unparseable numeric constant '{}'", m_lexer.text_utf8(value_token));
            }
            enumeration.values.emplace_back(m_lexer.text(name_token), number_maybe.value());
        } break;
        case TokenKind::QuotedString: {
            if (value_type != BasicType::None && value_type != BasicType::String) {
                fatal("Value type mismatch in enumeration '{}'", m_lexer.text_utf8(name));
            }
            value_type = BasicType::String;
            enumeration.values.emplace_back(m_lexer.text(name_token), m_lexer.text(token).substr(1, m_lexer.text(token).length() - 2));
        } break;
        default:
            fatal("Unexpected value for enumeration value '{}::{}': '{}'",
                m_lexer.text_utf8(name), m_lexer.text_utf8(name_token), m_lexer.text_utf8(token));
        }
        if (enumeration.value_type != BasicType::None && value_type != enumeration.value_type) {
            fatal("Value type mismatch in enumeration '{}'", m_lexer.text_utf8(name));
        }
        if (enumeration.value_type == BasicType::None) {
            enumeration.value_type = value_type;
        }
        MUST(m_lexer.expect_symbol(';'));
        if (m_lexer.accept_symbol('}')) {
            break;
        }
    }

    if (TypeDef::has(m_lexer.text(name))) {
        auto &td = TypeDef::get(m_lexer.text(name));
        switch (td.kind) {
        case TypeDefKind::Alias: {
            if (td.alias_for()->kind != TypeKind::Basic) {
                fatal("Enumeration '{}' is typedef'ed to a non-basic type", m_lexer.text_utf8(name));
            }
            if (td.alias_for()->basic_type() != enumeration.value_type) {
                fatal("Enumeration '{}' is typedef'ed to a different basic type", m_lexer.text_utf8(name));
            }
            td.kind = TypeDefKind::Enumeration;
            td.definition = enumeration;
        } break;
        default:
            fatal("Duplicate type definition '{}'", m_lexer.text_utf8(name));
        }
    } else {
        TypeDef::make_enumeration(m_lexer.text(name), enumeration);
    }
}

void Parser::parse_enumeration()
{
    m_lexer.lex();
    auto const name = MUST_EVAL(m_lexer.expect_identifier());
    auto      &enumeration_def = TypeDef::make_enumeration(m_lexer.text(name));
    auto      &enumeration = enumeration_def.enumeration();
    MUST(m_lexer.expect_symbol('{'));
    while (true) {
        auto const name_token = m_lexer.peek();
        if (!name_token.is_identifier() && name_token.kind != TokenKind::Keyword) {
            fatal("Expected enum value name, got '{}'", m_lexer.text_utf8(name_token));
        }
        m_lexer.lex();
        MUST(m_lexer.expect_symbol('='));

        BasicType  value_type;
        auto const value_token = m_lexer.lex();
        switch (value_token.kind) {
        case TokenKind::Number: {
            value_type = BasicType::Int;
            auto const &number_maybe = string_to_integer<int>(m_lexer.text_utf8(value_token));
            assert(number_maybe.has_value());
            enumeration.values.emplace_back(m_lexer.text(name_token), number_maybe.value());
        } break;
        case TokenKind::QuotedString: {
            value_type = BasicType::String;
            std::wcout << "name: " << m_lexer.text(name_token) << " value: " << m_lexer.text(value_token) << std::endl;
            enumeration.values.emplace_back(m_lexer.text(name_token), m_lexer.text(value_token).substr(1, m_lexer.text(value_token).length() - 2));
        } break;
        default: {
            auto enum_name = MUST_EVAL(to_utf8(enumeration_def.name));
            fatal("Unexpected value for enumeration value '{}::{}': '{}'",
                enum_name, m_lexer.text_utf8(name_token), m_lexer.text_utf8(value_token));
        } break;
        }

        if (enumeration.value_type != BasicType::None && value_type != enumeration.value_type) {
            auto enum_name = MUST_EVAL(to_utf8(enumeration_def.name));
            fatal("Value type mismatch in enumeration '{}'", enum_name);
        }
        if (enumeration.value_type == BasicType::None) {
            enumeration.value_type = value_type;
        }
        if (m_lexer.accept_symbol('}')) {
            break;
        }
        if (!m_lexer.expect_symbol(',')) {
            fatal("Expected ',' or '}'. Got '{}'", m_lexer.text_utf8(m_lexer.peek()));
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
            fatal("Expected type definition name, got '{}'", m_lexer.text_utf8(name));
        }
    } break;
    default:
        fatal("Expected type definition name, got '{}'", m_lexer.text_utf8(name));
    }

    MUST(m_lexer.expect_symbol('='));
    auto t = parse_type();
    if (TypeDef::has(m_lexer.text(name))) {
        auto const &td = TypeDef::get(m_lexer.text(name));
        switch (td.kind) {
        case TypeDefKind::Enumeration: {
            if (t->kind != TypeKind::Basic) {
                fatal("Enumeration '{}' is typedef'ed to a non-basic type", m_lexer.text_utf8(name));
            }
            Enumeration enumeration = std::get<Enumeration>(td.definition);
            if (enumeration.value_type != t->basic_type()) {
                fatal("Enumeration '{}' is typedef'ed to a different basic type", m_lexer.text_utf8(name));
            }
        } break;
        default:
            fatal("Duplicate type definition '{}'", m_lexer.text_utf8(name));
        }
    } else if (t->kind == TypeKind::Variant) {
        auto &type_def = TypeDef::make_variant(m_lexer.text(name), t);
        type_def.get_dependencies(t);
    } else if (!is_basic_type) {
        auto &type_def = TypeDef::make_alias(m_lexer.text(name), t);
        type_def.get_dependencies(t);
    }
}

Module const &Parser::parse()
{
    auto const &buffer { MUST_EVAL(read_file_by_name<wchar_t>(file_name)) };
    m_lexer.push_source(buffer);

    while (true) {
        auto &token = m_lexer.peek();
        if (token.kind == TokenKind::EndOfFile) {
            break;
        }
        if (token.matches_keyword(SimpleKeywordCategory::Keyword, TSKeyword::Interface)) {
            parse_interface();
        } else if (token.matches_keyword(SimpleKeywordCategory::Keyword, TSKeyword::Namespace)) {
            parse_namespace();
        } else if (token.matches_keyword(SimpleKeywordCategory::Keyword, TSKeyword::Enum)) {
            parse_enumeration();
        } else if (token.matches_keyword(SimpleKeywordCategory::Keyword, TSKeyword::Type)) {
            parse_typedef();
        } else {
            m_lexer.lex();
        }
    }
    return m_module;
}

}
