/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <LibCore/IO.h>
#include <LibCore/JSON.h>
#include <LibCore/Lexer.h>

namespace LibCore {

using namespace std::literals;

[[nodiscard]] std::string JSONValue::to_string() const
{
    switch (type()) {
    case JSONType::Null:
        return "null";
    case JSONType::String:
        return std::get<std::string>(m_value);
    case JSONType::Integer:
        return std::to_string(std::get<int64_t>(m_value));
    case JSONType::Boolean:
        return std::get<bool>(m_value) ? "true" : "false";
    case JSONType::Double:
        return std::to_string(std::get<double>(m_value));
    case JSONType::Array: {
        auto const &array = std::get<Array>(m_value);
        if (array.empty())
            return "[]";
        std::string ret = "[ ";
        bool        first = true;
        for (auto const &v : array) {
            if (!first)
                ret += ", ";
            first = false;
            ret += v.serialize();
        }
        return ret + " ]";
    }
    case JSONType::Object: {
        auto const &object = std::get<Object>(m_value);
        if (object.empty())
            return "{}";
        std::string ret = "{ ";
        bool        first = true;
        for (auto const &v : object) {
            if (!first)
                ret += ", ";
            first = false;
            ret += "\"" + v.first + "\": " + v.second.serialize();
        }
        return ret + " }";
    }
    }
    UNREACHABLE();
}

[[nodiscard]] std::string JSONValue::serialize(bool pretty, int indent_width, int indent) const
{
    switch (type()) {
    case JSONType::Null:
    case JSONType::Integer:
    case JSONType::Double:
    case JSONType::Boolean:
        return to_string();
    case JSONType::String: {
        auto s = to_string();
        replace_all(s, "\r", R"(\r)");
        replace_all(s, "\n", R"(\n)");
        replace_all(s, "\t", R"(\t)");
        return "\"" + s + "\"";
    }
    case JSONType::Array: {
        auto const &array = std::get<Array>(m_value);
        if (array.empty())
            return "[]";
        std::string ret = "[";
        bool        first = true;
        for (auto const &v : array) {
            if (!first)
                ret += ",";
            if (pretty) {
                ret += "\n";
                for (auto i = 0u; i < indent + indent_width; ++i)
                    ret += ' ';
            } else {
                ret += ' ';
            }
            first = false;
            ret += v.serialize(pretty, indent_width, indent + indent_width);
        }
        if (pretty) {
            ret += "\n";
            for (auto i = 0u; i < indent; ++i)
                ret += ' ';
        } else {
            ret += ' ';
        }
        return ret + "]";
    }
    case JSONType::Object: {
        auto const &object = std::get<Object>(m_value);
        if (object.empty())
            return "{}";
        std::string ret = "{";
        bool        first = true;
        for (auto const &v : object) {
            if (!first)
                ret += ",";
            first = false;
            if (pretty) {
                ret += "\n";
                for (auto i = 0u; i < indent + indent_width; ++i)
                    ret += ' ';
            } else {
                ret += ' ';
            }
            ret += "\"" + v.first + "\": " + v.second.serialize();
        }
        if (pretty) {
            ret += "\n";
            for (auto i = 0u; i < indent; ++i)
                ret += ' ';
        } else {
            ret += ' ';
        }
        return ret + "}";
    }
    }
    UNREACHABLE();
}

#define JSONKEYWORD(S) \
    S(True, "true")    \
    S(False, "false")  \
    S(Null, "null")

enum class JSONKeyword {
#undef S
#define S(kw, str) kw,
    JSONKEYWORD(S)
#undef S
};

Result<JSONValue, JSONValue::ReadError> JSONValue::read_file(std::string_view const &file_name)
{
    trace(JSON, "Reading JSON file '{}'", file_name);
    auto json_text_maybe = read_file_by_name(file_name);
    if (json_text_maybe.is_error()) {
        log_error("Error reading JSON file '{}': {}", file_name, json_text_maybe.error().to_string());
        return ReadError { json_text_maybe.error() };
    }
    auto json_maybe = JSONValue::deserialize(json_text_maybe.value());
    if (json_maybe.is_error()) {
        log_error("Error parsing JSON file '{}': {}", file_name, json_maybe.error().to_string());
        return ReadError { json_maybe.error() };
    }
    return json_maybe.value();
}

using JSONLexer = Lexer<JSONKeyword>;
using JSONToken = Token<JSONKeyword>;

Result<std::string, JSONError> decode_string(JSONToken const &token)
{
    if (token != TokenKind::QuotedString) {
        return JSONError {
            JSONError::Code::SyntaxError,
            "Expected quoted string",
            static_cast<int>(token.location.line),
            static_cast<int>(token.location.column),
        };
    }
    if (!token.quoted_string().terminated) {
        return JSONError {
            JSONError::Code::SyntaxError,
            "Unterminated string",
            static_cast<int>(token.location.line),
            static_cast<int>(token.location.column),
        };
    }
    trace(JSON, "decode_string({})", token);
    if (token.text.length() > 2) {
        std::string qstr { token.text.substr(1, token.text.length() - 2) };
        replace_all(qstr, R"(\r)", "\r"sv);
        replace_all(qstr, R"(\n)", "\n"sv);
        replace_all(qstr, R"(\t)", "\t"sv);
        replace_all(qstr, R"(\")", R"(")");
        replace_all(qstr, R"(\')", "'"sv);
        return { qstr };
    }
    return token.text;
}

Result<JSONValue, JSONError> decode_value(JSONLexer &lexer)
{
    auto expect_symbol = [&lexer](int sym) -> Error<JSONError> {
        auto err = lexer.expect_symbol(sym);
        if (err.is_error()) {
            auto const &error_token = lexer.peek();
            return JSONError {
                JSONError::Code::SyntaxError,
                std::format("Expected '{:c}'", sym),
                static_cast<int>(error_token.location.line),
                static_cast<int>(error_token.location.column),
            };
        }
        return {};
    };

    auto const &token = lexer.lex();
    trace(JSON, "decode_value token: {}", token);
    switch (token.kind) {
    case TokenKind::Symbol: {
        switch (token.symbol_code()) {
        case '{': {
            auto result = JSONValue::object();
            if (!lexer.accept_symbol('}')) {
                while (true) {
                    auto const name_token = lexer.lex();
                    trace(JSON, "name_token: {}", name_token);
                    auto name = TRY_EVAL(decode_string(name_token));
                    trace(JSON, "Name: {}", name);
                    TRY(expect_symbol(':'));
                    auto value = TRY_EVAL(decode_value(lexer));
                    trace(JSON, "NVP: {}: {}", name, value.to_string());
                    result.set(name, value);
                    if (!lexer.accept_symbol(',')) {
                        TRY(expect_symbol('}'));
                        break;
                    }
                }
            }
            return result;
        }
        case '[': {
            auto result = JSONValue::array();
            if (!lexer.accept_symbol(']')) {
                while (true) {
                    auto value = TRY_EVAL(decode_value(lexer));
                    trace(JSON, "Array elem: {}", value.to_string());
                    result.append(value);
                    if (!lexer.accept_symbol(',')) {
                        TRY(expect_symbol(']'));
                        break;
                    }
                }
            }
            return result;
        }
        default:
            return JSONError {
                JSONError::Code::SyntaxError,
                std::format("Unexpected symbol '{:c}'", token.symbol_code()),
                static_cast<int>(token.location.line),
                static_cast<int>(token.location.column),
            };
        }
    }
    case TokenKind::QuotedString: {
        trace(JSON, "QuotedString: {}", token);
        return JSONValue { TRY_EVAL(decode_string(token)) };
    }
    case TokenKind::Number: {
        switch (token.number_type()) {
        case NumberType::Decimal: {
            auto dbl_maybe = string_to_double(token.text);
            assert(dbl_maybe.has_value());
            return JSONValue(dbl_maybe.value());
        }
        default: {
            auto int_maybe = string_to_integer<long>(token.text);
            assert(int_maybe.has_value());
            return JSONValue(int_maybe.value());
        }
        }
    }
    case TokenKind::Keyword: {
        switch (token.keyword_code()) {
        case JSONKeyword::False:
            return JSONValue(false);
        case JSONKeyword::True:
            return JSONValue(true);
        case JSONKeyword::Null:
            return JSONValue();
        default:
            UNREACHABLE();
        }
    }
    default:
        return JSONError {
            JSONError::Code::SyntaxError,
            std::format("Invalid token '{:}' ({:})", token.text, TokenKind_name(token.kind)),
            static_cast<int>(token.location.line),
            static_cast<int>(token.location.column),
        };
    }
}

template<>
std::map<std::string_view, JSONKeyword> get_keywords()
{
    return {
        { "true", JSONKeyword::True },
        { "false", JSONKeyword::False },
        { "null", JSONKeyword::Null },
    };
}

Result<JSONValue, JSONError> JSONValue::deserialize(std::string_view const &str)
{
    JSONLexer lexer;
    lexer.push_source(str, "string");
    return decode_value(lexer);
}

}
