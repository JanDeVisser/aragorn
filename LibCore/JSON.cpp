/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

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
}

constexpr static int JSONKeywordTrue = 0;
constexpr static int JSONKeywordFalse = 1;
constexpr static int JSONKeywordNull = 2;

Result<JSONValue, JSONError> JSONValue::deserialize(std::string_view const &str)
{
    JSONValue              current;
    std::vector<JSONValue> state {};

    Language json_language {};
    json_language.name = "JSON";
    json_language.keywords = {
        { "true", JSONKeywordTrue },
        { "false", JSONKeywordFalse },
        { "null", JSONKeywordNull },
    };

    Lexer lexer { json_language };
    lexer.push_source(str, "string");
    for (auto token = lexer.peek(); token != TokenKind::EndOfFile; token = lexer.peek()) {
        bool handled { false };
        current = {};
        switch (token.kind) {
        case TokenKind::Keyword: {
            switch (token.keyword_code.code) {
            case JSONKeywordFalse:
                current = JSONValue { false };
                lexer.lex();
                break;
            case JSONKeywordTrue:
                current = JSONValue { true };
                lexer.lex();
                break;
            case JSONKeywordNull:
                current = JSONValue {};
                lexer.lex();
                break;
            default:
                UNREACHABLE();
            }
        } break;
        case TokenKind::QuotedString: {
            if (!token.quoted_string.terminated) {
                return JSONError {
                    JSONError::Code::SyntaxError,
                    "Unterminated quoted string",
                    (int) token.location.line,
                    (int) token.location.column,
                };
            }
            switch (token.quoted_string.quote_type) {
            case QuoteType::DoubleQuote:
            case QuoteType::SingleQuote: {
                auto s = token.text.substr(1, token.text.length() - 2);
                replace_all(s, R"(\r)", "\r"sv);
                replace_all(s, R"(\n)", "\n"sv);
                replace_all(s, R"(\t)", "\t"sv);
                replace_all(s, R"(\")", R"(")");
                replace_all(s, R"(\')", "'"sv);
                current = JSONValue(s);
                lexer.lex();
            } break;
            case QuoteType::BackQuote:
                return JSONError {
                    JSONError::Code::SyntaxError,
                    "Backquoted ('`') strings are not allowed in JSON",
                    (int) token.location.line,
                    (int) token.location.column,
                };
            }
        } break;
        case TokenKind::Number: {
            switch (token.number_type) {
            case NumberType::Integer:
            case NumberType::HexNumber:
            case NumberType::BinaryNumber: {
                auto num = string_to_integer<int64_t>(token.text);
                if (!num) {
                    return JSONError {
                        JSONError::Code::SyntaxError,
                        "Unparseable integer number",
                        (int) token.location.line,
                        (int) token.location.column,
                    };
                }
                current = JSONValue { num.value() };
                lexer.lex();
            } break;
            case NumberType::Decimal: {
                auto num = string_to_double(token.text);
                if (!num) {
                    return JSONError {
                        JSONError::Code::SyntaxError,
                        "Unparseable floating point number",
                        (int) token.location.line,
                        (int) token.location.column,
                    };
                }
                current = JSONValue { num.value() };
                lexer.lex();
            } break;
            }
        } break;
        case TokenKind::Symbol: {
            switch (token.symbol_code) {
            case '[':
                state.emplace_back(JSONType::Array);
                lexer.lex();
                handled = true;
                break;
            case ']':
                if (state.empty() || !state.back().is_array())
                    return JSONError {
                        JSONError::Code::SyntaxError,
                        "Stray ']'",
                        (int) token.location.line,
                        (int) token.location.column,
                    };
                lexer.lex();
                current = state.back();
                state.pop_back();
                break;
            case '{':
                state.emplace_back(JSONType::Object);
                lexer.lex();
                handled = true;
                break;
            case '}':
                if (state.empty() || !state.back().is_object())
                    return JSONError {
                        JSONError::Code::SyntaxError,
                        "Stray '}'",
                        (int) token.location.line,
                        (int) token.location.column,
                    };
                lexer.lex();
                current = state.back();
                state.pop_back();
                break;
            case ',':
                if (state.empty() || (!state.back().is_object() && !state.back().is_array()))
                    return JSONError {
                        JSONError::Code::SyntaxError,
                        "Stray ','",
                        (int) token.location.line,
                        (int) token.location.column,
                    };
                lexer.lex();
                handled = true;
                break;
            case ':':
                if (state.empty() || !state.back().is_string())
                    return JSONError {
                        JSONError::Code::SyntaxError,
                        "Stray ':'",
                        (int) token.location.line,
                        (int) token.location.column,
                    };
                lexer.lex();
                handled = true;
                break;
            }
        } break;
        case TokenKind::EndOfFile:
            handled = true;
            break;
        case TokenKind::EndOfLine:
        case TokenKind::Whitespace:
            handled = true;
            lexer.lex();
            break;
        default:
            return JSONError {
                JSONError::Code::SyntaxError,
                std::format("Unexpected token '{}'", token.text),
                (int) token.location.line,
                (int) token.location.column,
            };
        }

        if (handled) {
            continue;
        }
        if (state.empty()) {
            state.push_back(current);
            continue;
        }

        auto &last = state.back();
        if (last.is_array()) {
            last.append(current);
            continue;
        }

        if (last.is_object()) {
            if (!current.is_string())
                return JSONError {
                    JSONError::Code::SyntaxError,
                    std::format("Expected object key name, got '{}'", current.to_string()),
                    (int) token.location.line,
                    (int) token.location.column,
                };
            state.push_back(current);
            continue;
        }

        if (last.is_string() && (state.size() >= 2) && state[state.size() - 2].is_object()) {
            auto &obj = state[state.size() - 2];
            obj.set(std::get<std::string>(last.m_value), current);
            state.pop_back();
            continue;
        }
        return JSONError {
            JSONError::Code::SyntaxError,
            std::format("Unexpected JSON value '{}'. This should not happen", current.to_string()),
            (int) token.location.line,
            (int) token.location.column,
        };
    }
    if (state.empty())
        return JSONValue();
    return state.back();
}

}
