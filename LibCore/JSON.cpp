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

Result<std::string, JSONError> JSONValue::decode_string(Token const &t)
{
    assert(ss_peek(&decoder->ss) == '\"');
    StringBuilder sb = { 0 };
    ss_skip_one(&decoder->ss);
    ss_reset(&decoder->ss);
    while (true) {
        int ch = ss_peek(&decoder->ss);
        if (ch == '\\') {
            ss_skip_one(&decoder->ss);
            ch = ss_peek(&decoder->ss);
            switch (ch) {
            case 0:
                ERROR(StringView, JSONError, decoder->ss.point.line, "Bad escape");
            case 'n':
                sb_append_char(&sb, '\n');
                break;
            case 'r':
                sb_append_char(&sb, '\r');
                break;
            case 't':
                sb_append_char(&sb, '\t');
                break;
            case '\"':
                sb_append_char(&sb, '\"');
                break;
            default:
                sb_append_char(&sb, ch);
                break;
            }
            ss_skip_one(&decoder->ss);
        } else if (ch == '"') {
            ss_skip_one(&decoder->ss);
            StringView ret = sb.view;
            RETURN(StringView, ret);
        } else if (ch == 0) {
            ERROR(StringView, JSONError, 0, "Unterminated string");
        } else {
            sb_append_char(&sb, ch);
            ss_skip_one(&decoder->ss);
        }
    }
}

Result<JSONValue, JSONError> JSONValue::decode_value(JSONLexer &lexer)
{
    auto expect_symbol = [&lexer](int sym) -> Error<JSONError> {
        auto err = lexer.expect_symbol(sym);
        if (err.is_error()) {
            auto const &error_token = lexer.peek();
            return JSONError {
                JSONError::Code::SyntaxError,
                std::format("Expected '{:c}'", sym),
                (int) error_token.location.line,
                (int) error_token.location.column,
            };
        }
        return {};
    };

    auto const &token = lexer.peek();
    switch (token.kind) {
    case TokenKind::Symbol: {
        switch (token.symbol_code) {
        case '{': {
            auto result = JSONValue::object();
            lexer.lex();
            while (true) {
                if (lexer.accept_symbol('}')) {
                    break;
                }
                auto const &t = lexer.peek();
                if (t != TokenKind::QuotedString || t.quoted_string.quote_type == QuoteType::BackQuote) {
                    return JSONError {
                        JSONError::Code::SyntaxError,
                        "Expected a quoted string object key",
                        (int) t.location.line,
                        (int) t.location.column,
                    };
                }
                auto name = TRY_EVAL(JSONValue::decode_string(t));
                trace(JSON, "Name: {}", name);
                TRY(expect_symbol(':'));
                auto value = TRY_EVAL(JSONValue::decode_value(lexer));
                trace(JSON, "NVP: {}: {}", name, value.to_string());
                result.set(name, value);
                if (!lexer.accept_symbol(',')) {
                    TRY(expect_symbol('}'));
                    break;
                }
            }
            return result;
        } break;
        case '[': {
            auto result = JSONValue::array();
            lexer.lex();
            while (true) {
                if (lexer.accept_symbol(']')) {
                    break;
                }
                auto const &t = lexer.peek();
                auto        value = TRY_EVAL(JSONValue::decode_value(lexer));
                trace(JSON, "Array elem: {}", value.to_string());
                result.append(value);
                if (!lexer.accept_symbol(',')) {
                    TRY(expect_symbol('}'));
                    break;
                }
            }
            return result;
        } break;
        default:
            return JSONError {
                JSONError::Code::SyntaxError,
                std::format("Unexpected symbol '{:c}'", token.symbol_code),
                (int) token.location.line,
                (int) token.location.column,
            };
        }
    case TokenKind::QuotedString: {
    }
    }
        default: {
            if (isdigit(ss_peek(&decoder->ss)) || ss_peek(&decoder->ss) == '-') {
                ss_reset(&decoder->ss);
                ss_skip_one(&decoder->ss);
                while (isdigit(ss_peek(&decoder->ss))) {
                    ss_skip_one(&decoder->ss);
                }
                if (ss_peek(&decoder->ss) == '.') {
                    ss_skip_one(&decoder->ss);
                    while (isdigit(ss_peek(&decoder->ss))) {
                        ss_skip_one(&decoder->ss);
                    }
                    StringView sv = ss_read_from_mark(&decoder->ss);
                    char       ch = sv.ptr[sv.length];
                    ((char *) sv.ptr)[sv.length] = '\0';
                    double dbl = strtod(sv.ptr, NULL);
                    ((char *) sv.ptr)[sv.length] = ch;
                    RETURN(JSONValue, json_number(dbl));
                }
                StringView         sv = ss_read_from_mark(&decoder->ss);
                IntegerParseResult parse_result = sv_parse_integer(sv, I64);
                if (parse_result.success) {
                    RETURN(JSONValue, json_integer(parse_result.integer));
                }
                RETURN(JSONValue, json_integer(parse_result.integer));
            }
            if (ss_expect_sv(&decoder->ss, sv_from("true"))) {
                RETURN(JSONValue, json_bool(true));
            }
            if (ss_expect_sv(&decoder->ss, sv_from("false"))) {
                RETURN(JSONValue, json_bool(false));
            }
            if (ss_expect_sv(&decoder->ss, sv_from("null"))) {
                RETURN(JSONValue, json_null());
            }
            ERROR(JSONValue, JSONError, 0, "%d:%d: Invalid JSON", decoder->ss.point.line, decoder->ss.point.column);
        }
        }
    }

        Result<JSONValue, JSONError> JSONValue::deserialize(std::string_view const &str)
        {
            Language json_language {};
            json_language.name = "JSON";
            json_language.keywords = {
                { "true", JSONKeywordTrue },
                { "false", JSONKeywordFalse },
                { "null", JSONKeywordNull },
            };

            Lexer lexer { json_language };
            lexer.push_source(str, "string");
            return decode_value(lexer);
        }

        Result<JSONValue, JSONError> JSONValue::xdeserialize(std::string_view const &str)
        {
            JSONValue              current;
            std::vector<JSONValue> state {};

            auto dump_state = [&state](auto const &token) {
                std::string s { join(state, " | ", [](JSONValue const &v) { return JSONType_name(v.type()); }) };
                trace(JSON, "State: - {} - Token: {} '{}'", s, TokenKind_name(token.kind), token.text);
            };

            Language json_language {};
            json_language.name = "JSON";
            json_language.keywords = {
                { "true", JSONKeywordTrue },
                { "false", JSONKeywordFalse },
                { "null", JSONKeywordNull },
            };

            Lexer lexer { json_language };
            lexer.push_source(str, "string");

            while (true) {
                auto const &token = lexer.peek();
                dump_state(token);
                if (token == TokenKind::EndOfFile) {
                    break;
                }
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
                        if (state.empty() || !state.back().is_array()) {
                            return JSONError {
                                JSONError::Code::SyntaxError,
                                "Stray ']'",
                                (int) token.location.line,
                                (int) token.location.column,
                            };
                        }
                        lexer.lex();
                        current = state.back();
                        state.pop_back();
                        handled = true;
                        break;
                    case '{':
                        state.emplace_back(JSONType::Object);
                        lexer.lex();
                        handled = true;
                        break;
                    case '}':
                        if (state.empty() || !state.back().is_object()) {
                            return JSONError {
                                JSONError::Code::SyntaxError,
                                "Stray '}'",
                                (int) token.location.line,
                                (int) token.location.column,
                            };
                        }
                        lexer.lex();
                        current = state.back();
                        state.pop_back();
                        handled = true;
                        break;
                    case ',':
                        if (state.empty() || (!state.back().is_object() && !state.back().is_array())) {
                            return JSONError {
                                JSONError::Code::SyntaxError,
                                "Stray ','",
                                (int) token.location.line,
                                (int) token.location.column,
                            };
                        }
                        lexer.lex();
                        handled = true;
                        break;
                    case ':':
                        if (state.empty() || !state.back().is_string()) {
                            return JSONError {
                                JSONError::Code::SyntaxError,
                                "Stray ':'",
                                (int) token.location.line,
                                (int) token.location.column,
                            };
                        }
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
                    if (!current.is_string()) {
                        return JSONError {
                            JSONError::Code::SyntaxError,
                            std::format("Expected object key name, got '{}'", token.text),
                            (int) token.location.line,
                            (int) token.location.column,
                        };
                    }
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
