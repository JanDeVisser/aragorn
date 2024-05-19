/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <string_view>

#include <LibCore/JSON.h>

namespace LibCore {

struct NoSuchEnumValue {
    std::string enum_name;
    std::string value;
};

template<typename ResultType>
using EnumResult = Result<ResultType, NoSuchEnumValue>;

#define TOKENKINDS(S) \
    S(Unknown)        \
    S(EndOfFile)      \
    S(EndOfLine)      \
    S(Symbol)         \
    S(Keyword)        \
    S(Identifier)     \
    S(Number)         \
    S(QuotedString)   \
    S(Comment)        \
    S(Whitespace)     \
    S(Program)        \
    S(Directive)      \
    S(DirectiveArg)   \
    S(Module)

enum class TokenKind {
#undef S
#define S(kind) kind,
    TOKENKINDS(S)
#undef S
};

extern std::string           TokenKind_name(TokenKind kind);
extern EnumResult<TokenKind> TokenKind_from_string(std::string_view const &kind);

template<>
inline JSONValue to_json(TokenKind const &kind)
{
    return JSONValue { TokenKind_name(kind) };
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, TokenKind &kind)
{
    if (!json.is_string()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto kind_maybe = TokenKind_from_string(json.to_string());
    if (kind_maybe.is_error()) {
        return JSONError { JSONError::Code::ProtocolError, std::format("Invalid token kind '{}'", json.to_string()) };
    }
    kind = kind_maybe.value();
    return {};
};

#define QUOTETYPES(S)    \
    S(SingleQuote, '\'') \
    S(DoubleQuote, '"')  \
    S(BackQuote, '`')

enum class QuoteType : char {
#undef S
#define S(T, Q) T = (Q),
    QUOTETYPES(S)
#undef S
};

extern std::string           QuoteType_name(QuoteType quote);
extern EnumResult<QuoteType> QuoteType_from_string(std::string_view quote);

template<>
inline JSONValue to_json(QuoteType const &type)
{
    return JSONValue { QuoteType_name(type) };
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, QuoteType &type)
{
    if (!json.is_string()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto type_maybe = QuoteType_from_string(json.to_string());
    if (type_maybe.is_error()) {
        return JSONError { JSONError::Code::ProtocolError, std::format("Invalid quote type '{}'", json.to_string()) };
    }
    type = type_maybe.value();
    return {};
};

#define COMMENTTYPES(S) \
    S(Block)            \
    S(Line)

enum class CommentType {
#undef S
#define S(T) T,
    COMMENTTYPES(S)
#undef S
};

extern std::string             CommentType_name(CommentType quote);
extern EnumResult<CommentType> CommentType_from_string(std::string_view comment);

template<>
inline JSONValue to_json(CommentType const &type)
{
    return JSONValue { CommentType_name(type) };
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, CommentType &type)
{
    if (!json.is_string()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto type_maybe = CommentType_from_string(json.to_string());
    if (type_maybe.is_error()) {
        return JSONError { JSONError::Code::ProtocolError, std::format("Invalid comment type '{}'", json.to_string()) };
    }
    type = type_maybe.value();
    return {};
};

#define NUMBERTYPES(S) \
    S(Integer)         \
    S(Decimal)         \
    S(HexNumber)       \
    S(BinaryNumber)

enum class NumberType {
#undef S
#define S(T) T,
    NUMBERTYPES(S)
#undef S
};

extern std::string            NumberType_name(NumberType quote);
extern EnumResult<NumberType> NumberType_from_string(std::string_view comment);

template<>
inline JSONValue to_json(NumberType const &type)
{
    return JSONValue { NumberType_name(type) };
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, NumberType &type)
{
    if (!json.is_string()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto type_maybe = NumberType_from_string(json.to_string());
    if (type_maybe.is_error()) {
        return JSONError { JSONError::Code::ProtocolError, std::format("Invalid number type '{}'", json.to_string()) };
    }
    type = type_maybe.value();
    return {};
};

struct TokenLocation {
    TokenLocation() = default;
    TokenLocation(TokenLocation const &) = default;
    explicit TokenLocation(std::string_view const &name)
        : file(name)
    {
    }

    std::string file {};
    size_t      index { 0 };
    size_t      line { 0 };
    size_t      column { 0 };
};

template<>
inline JSONValue to_json(TokenLocation const &location)
{
    auto ret = JSONValue::object();
    set(ret, "index", location.index);
    set(ret, "file", location.file);
    set(ret, "line", location.line);
    set(ret, "column", location.column);
    return ret;
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, TokenLocation &location)
{
    if (!json.is_object()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    location.file = TRY_EVAL(json.try_get<std::string>("file"));
    location.index = TRY_EVAL(json.try_get<size_t>("index"));
    location.line = TRY_EVAL(json.try_get<size_t>("line"));
    location.column = TRY_EVAL(json.try_get<size_t>("column"));
    return {};
}

template<int CodeType>
struct TokenCode {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    TokenCode(int c)
        : code(c)
    {
    }

    operator int() const { return code; }
    operator bool() const { return code != 0; }
#pragma clang diagnostic pop
    TokenCode() = default;
    TokenCode(TokenCode const &) = default;

    int  code = { 0 };
    bool operator==(TokenCode const &other) const = default;
};

template<int CodeType>
inline JSONValue to_json(TokenCode<CodeType> const &code)
{
    return JSONValue { code.code };
}

template<int CodeType>
inline Error<JSONError> decode_value(JSONValue const &json, TokenCode<CodeType> &code)
{
    if (!json.is_integer()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    code = json.value<int>().value();
    return {};
};

using KeywordCode = TokenCode<0>;

struct Keyword {
    std::string_view keyword {};
    KeywordCode      code {};
};

using DirectiveCode = TokenCode<1>;

struct Token {
    Token()
    {
    }

    Token(Token const &) = default;

    TokenKind     kind { TokenKind::Unknown };
    std::string   text {};
    TokenLocation location {};
    union {
        NumberType number_type { NumberType::Integer };
        struct {
            QuoteType quote_type;
            bool      triple;
            bool      terminated;
        } quoted_string;
        struct {
            CommentType comment_type;
            bool        terminated;
        } comment_text;
        KeywordCode   keyword_code;
        DirectiveCode directive_code;
        int           symbol_code;
    };

    static Token number(NumberType type, std::string_view const &text)
    {
        Token ret;
        ret.kind = TokenKind::Number;
        ret.number_type = type;
        ret.text = std::string { text };
        return ret;
    }

    static Token symbol(int sym)
    {
        Token ret;
        ret.kind = TokenKind::Symbol;
        ret.symbol_code = sym;
        ret.text = std::string { "x" };
        ret.text[0] = static_cast<char>(sym);
        return ret;
    }

    static Token keyword(KeywordCode const &code, std::string_view const &text)
    {
        Token ret;
        ret.kind = TokenKind::Keyword;
        ret.keyword_code = code;
        ret.text = std::string { text };
        return ret;
    }

    static Token whitespace(std::string_view text)
    {
        Token ret;
        ret.kind = TokenKind::Whitespace;
        ret.text = std::string { text };
        return ret;
    }

    static Token identifier(std::string_view text)
    {
        Token ret;
        ret.kind = TokenKind::Identifier;
        ret.text = std::string { text };
        return ret;
    }

    static Token directive(DirectiveCode const &code, std::string_view const &text)
    {
        Token ret;
        ret.kind = TokenKind::Directive;
        ret.directive_code = code;
        ret.text = std::string { text };
        return ret;
    }

    static Token comment(CommentType type, std::string_view const &text, bool terminated = true)
    {
        Token ret;
        ret.kind = TokenKind::Comment;
        ret.comment_text.comment_type = type;
        ret.comment_text.terminated = terminated;
        ret.text = std::string { text };
        return ret;
    }

    static Token end_of_line(std::string_view text)
    {
        Token ret;
        ret.kind = TokenKind::EndOfLine;
        ret.text = std::string { text };
        return ret;
    }

    static Token end_of_file()
    {
        Token ret;
        ret.kind = TokenKind::EndOfFile;
        ret.text = std::string {};
        return ret;
    }

    static Token string(QuoteType type, std::string_view text, bool terminated = true, bool triple = false)
    {
        Token ret;
        ret.kind = TokenKind::QuotedString;
        ret.text = std::string { text };
        ret.quoted_string.quote_type = type;
        ret.quoted_string.terminated = terminated;
        ret.quoted_string.triple = triple;
        return ret;
    }

    bool operator==(TokenKind const &k) const
    {
        return k == kind;
    }

    bool operator!=(TokenKind const &k) const
    {
        return k != kind;
    }

    bool operator==(int s) const
    {
        return matches(s);
    }

    bool operator!=(int s) const
    {
        return !matches(s);
    }

    bool operator==(KeywordCode const &code) const
    {
        return matches(code);
    }

    bool operator!=(KeywordCode const &code) const
    {
        return !matches(code);
    }

    [[nodiscard]] bool matches(TokenKind k) const { return kind == k; }
    [[nodiscard]] bool matches(int symbol) const { return matches(TokenKind::Symbol) && this->symbol_code == symbol; }
    [[nodiscard]] bool matches(KeywordCode code) const { return matches(TokenKind::Keyword) && this->keyword_code == code; }
    [[nodiscard]] bool matches(DirectiveCode code) const { return matches(TokenKind::Directive) && this->directive_code == code; }
    [[nodiscard]] bool is_identifier() const { return matches(TokenKind::Identifier); }

    friend Error<JSONError> decode_value(JSONValue const &json, Token &token);
    friend Error<JSONError> decode_token(JSONValue const &json, Token &token);
};

template<TokenKind K>
inline void to_json_token(Token const &token, JSONValue &json)
{
    // By default nothing
}

template<>
inline void to_json_token<TokenKind::Number>(Token const &token, JSONValue &json)
{
    set(json, "number_type", NumberType_name(token.number_type));
}

template<>
inline void to_json_token<TokenKind::QuotedString>(Token const &token, JSONValue &json)
{
    auto quote = JSONValue::object();
    set(quote, "quote_type", token.quoted_string.quote_type);
    set(quote, "triple", token.quoted_string.triple);
    set(quote, "terminated", token.quoted_string.terminated);
    set(json, "quoted_string", quote);
}

template<>
inline void to_json_token<TokenKind::Comment>(Token const &token, JSONValue &json)
{
    auto comment = JSONValue::object();
    set(comment, "comment_type", token.comment_text.comment_type);
    set(comment, "terminated", token.comment_text.terminated);
    set(json, "comment", comment);
}

template<>
inline void to_json_token<TokenKind::Keyword>(Token const &token, JSONValue &json)
{
    set(json, "code", token.keyword_code);
}

template<>
inline void to_json_token<TokenKind::Directive>(Token const &token, JSONValue &json)
{
    set(json, "directive", token.directive_code);
}

template<>
inline void to_json_token<TokenKind::Symbol>(Token const &token, JSONValue &json)
{
    set(json, "symbol", token.symbol_code);
}

template<>
inline JSONValue to_json(Token const &token)
{
    auto ret = JSONValue::object();
    set(ret, "kind", TokenKind_name(token.kind));
    set(ret, "text", token.text);
    set(ret, "location", token.location);
    switch (token.kind) {
#undef S
#define S(K)                                     \
    case TokenKind::K:                           \
        to_json_token<TokenKind::K>(token, ret); \
        break;
        TOKENKINDS(S)
#undef S
    default:
        UNREACHABLE();
    }
    return ret;
}

template<TokenKind K>
inline Error<JSONError> decode_token(JSONValue const &json, Token &token)
{
    // By default nothing
    return {};
}

template<>
inline Error<JSONError> decode_token<TokenKind::Number>(JSONValue const &json, Token &token)
{
    token.number_type = TRY_EVAL(json.try_get<NumberType>("number_type"));
    return {};
}

template<>
inline Error<JSONError> decode_token<TokenKind::QuotedString>(JSONValue const &json, Token &token)
{
    auto quote = TRY_EVAL(json.try_get<JSONValue>("quoted_string"));
    token.quoted_string.quote_type = TRY_EVAL(quote.try_get<QuoteType>("quote_type"));
    token.quoted_string.triple = TRY_EVAL(quote.try_get<bool>("triple"));
    token.quoted_string.terminated = TRY_EVAL(quote.try_get<bool>("terminated"));
    return {};
}

template<>
inline Error<JSONError> decode_token<TokenKind::Comment>(JSONValue const &json, Token &token)
{
    auto comment = TRY_EVAL(json.try_get<JSONValue>("comment"));
    token.comment_text.comment_type = TRY_EVAL(comment.try_get<CommentType>("comment_type"));
    token.comment_text.terminated = TRY_EVAL(comment.try_get<bool>("terminated"));
    return {};
}

template<>
inline Error<JSONError> decode_token<TokenKind::Keyword>(JSONValue const &json, Token &token)
{
    token.keyword_code = TRY_EVAL(json.try_get<KeywordCode>("code"));
    return {};
}

template<>
inline Error<JSONError> decode_token<TokenKind::Directive>(JSONValue const &json, Token &token)
{
    token.directive_code = TRY_EVAL(json.try_get<DirectiveCode>("directive"));
    return {};
}

template<>
inline Error<JSONError> decode_token<TokenKind::Symbol>(JSONValue const &json, Token &token)
{
    token.symbol_code = TRY_EVAL(json.try_get<int>("symbol"));
    return {};
}

template<>
inline Error<JSONError> decode_value(JSONValue const &json, Token &token)
{
    if (!json.is_object()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    token.kind = TRY_EVAL(json.try_get<TokenKind>("kind"));
    token.text = TRY_EVAL(json.try_get<std::string>("text"));
    token.location = TRY_EVAL(json.try_get<TokenLocation>("location"));
    switch (token.kind) {
#undef S
#define S(K)                                          \
    case TokenKind::K:                                \
        TRY(decode_token<TokenKind::K>(json, token)); \
        break;
        TOKENKINDS(S)
#undef S
    default:
        UNREACHABLE();
    }
    return {};
}

}

template<>
struct std::formatter<LibCore::Token, char>
{
    using Token = LibCore::Token;
    bool quoted = false;

    template<class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("Invalid format args for QuotableString.");
        }
        return it;
    }

    template<class FmtContext>
    FmtContext::iterator format(Token const& token, FmtContext& ctx) const
    {
        std::ostringstream out;
        out << "[" << LibCore::TokenKind_name(token.kind) << "]";
        if (token != LibCore::TokenKind::EndOfLine && token != LibCore::TokenKind::EndOfFile) {
            out << " '" << token.text << "'";
        }
        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

