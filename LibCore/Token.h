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

#define VALUE_TOKENKINDS(S) \
    S(Symbol)               \
    S(Keyword)              \
    S(Number)               \
    S(QuotedString)         \
    S(Comment)              \
    S(Directive)

#define TOKENKINDS(S)   \
    S(Unknown)          \
    VALUE_TOKENKINDS(S) \
    S(EndOfFile)        \
    S(EndOfLine)        \
    S(Identifier)       \
    S(Tab)              \
    S(Whitespace)       \
    S(Program)          \
    S(DirectiveArg)     \
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
inline JSONValue encode(TokenKind const &kind)
{
    return JSONValue { TokenKind_name(kind) };
}

template<>
inline Result<TokenKind, JSONError> decode(JSONValue const &json)
{
    if (!json.is_string()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto kind_maybe = TokenKind_from_string(json.to_string());
    if (kind_maybe.is_error()) {
        return JSONError { JSONError::Code::ProtocolError, std::format("Invalid token kind '{}'", json.to_string()) };
    }
    return kind_maybe.value();
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
inline JSONValue encode(QuoteType const &type)
{
    return JSONValue { QuoteType_name(type) };
}

template<>
inline Result<QuoteType, JSONError> decode(JSONValue const &json)
{
    if (!json.is_string()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto type_maybe = QuoteType_from_string(json.to_string());
    if (type_maybe.is_error()) {
        return JSONError { JSONError::Code::ProtocolError, std::format("Invalid quote type '{}'", json.to_string()) };
    }
    return type_maybe.value();
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
inline JSONValue encode(CommentType const &type)
{
    return JSONValue { CommentType_name(type) };
}

template<>
inline Result<CommentType, JSONError> decode(JSONValue const &json)
{
    if (!json.is_string()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto type_maybe = CommentType_from_string(json.to_string());
    if (type_maybe.is_error()) {
        return JSONError { JSONError::Code::ProtocolError, std::format("Invalid comment type '{}'", json.to_string()) };
    }
    return type_maybe.value();
};

#define NUMBERTYPES(S) \
    S(Integer)         \
    S(Decimal)         \
    S(HexNumber)       \
    S(BinaryNumber)

enum class NumberType : int {
#undef S
#define S(T) T,
    NUMBERTYPES(S)
#undef S
};

extern std::string            NumberType_name(NumberType quote);
extern EnumResult<NumberType> NumberType_from_string(std::string_view comment);

template<>
inline JSONValue encode(NumberType const &type)
{
    return JSONValue { NumberType_name(type) };
}

template<>
inline Result<NumberType, JSONError> decode(JSONValue const &json)
{
    if (!json.is_string()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto type_maybe = NumberType_from_string(json.to_string());
    if (type_maybe.is_error()) {
        return JSONError { JSONError::Code::ProtocolError, std::format("Invalid number type '{}'", json.to_string()) };
    }
    return type_maybe.value();
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
inline JSONValue encode(TokenLocation const &location)
{
    auto ret = JSONValue::object();
    set(ret, "index", location.index);
    set(ret, "file", location.file);
    set(ret, "line", location.line);
    set(ret, "column", location.column);
    return ret;
}

template<>
inline Result<TokenLocation, JSONError> decode(JSONValue const &json)
{
    if (!json.is_object()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto location = TokenLocation {};
    location.file = TRY_EVAL(json.try_get<std::string>("file"));
    location.index = TRY_EVAL(json.try_get<size_t>("index"));
    location.line = TRY_EVAL(json.try_get<size_t>("line"));
    location.column = TRY_EVAL(json.try_get<size_t>("column"));
    return location;
}

struct QuotedString {
    QuoteType quote_type;
    bool      triple;
    bool      terminated;
};

struct CommentText {
    CommentType comment_type;
    bool        terminated;
};

enum class NoKeyword {
};

enum class NoDirective {
};

template<typename KeywordCodeType = NoKeyword, typename DirectiveCodeType = NoDirective>
struct Token {
    using TokenValue = std::variant<bool, NumberType, QuotedString, CommentText, KeywordCodeType, DirectiveCodeType, int>;

    Token() = default;
    Token(Token const &) = default;

    TokenKind     kind { TokenKind::Unknown };
    std::string   text {};
    TokenLocation location {};
    TokenValue    value;

    static Token number(NumberType type, std::string_view const &text)
    {
        Token ret;
        ret.kind = TokenKind::Number;
        ret.value = type;
        ret.text = std::string { text };
        return ret;
    }

    static Token symbol(int sym)
    {
        Token ret;
        ret.kind = TokenKind::Symbol;
        ret.value = TokenValue { std::in_place_index<6>, sym };
        ret.text = std::string { "x" };
        ret.text[0] = static_cast<char>(sym);
        return ret;
    }

    static Token keyword(KeywordCodeType const &code, std::string_view const &text)
    {
        Token ret;
        ret.kind = TokenKind::Keyword;
        ret.value = TokenValue { std::in_place_index<4>, code };
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

    static Token tab(std::string_view text)
    {
        Token ret;
        ret.kind = TokenKind::Tab;
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

    static Token directive(DirectiveCodeType const &code, std::string_view const &text)
    {
        Token ret;
        ret.kind = TokenKind::Directive;
        ret.value = TokenValue { std::in_place_index<5>, code };
        ret.text = std::string { text };
        return ret;
    }

    static Token comment(CommentType type, std::string_view const &text, bool terminated = true)
    {
        Token ret;
        ret.kind = TokenKind::Comment;
        ret.value = CommentText { .comment_type = type, .terminated = terminated };
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
        ret.value = QuotedString {
            .quote_type = type,
            .triple = triple,
            .terminated = terminated
        };
        ret.text = std::string { text };
        return ret;
    }

    NumberType number_type() const
    {
        assert(kind == TokenKind::Number);
        return std::get<1>(value);
    }

    int symbol_code() const
    {
        assert(kind == TokenKind::Symbol);
        return std::get<6>(value);
    }

    KeywordCodeType keyword_code() const
    {
        assert(kind == TokenKind::Keyword);
        return std::get<4>(value);
    }

    DirectiveCodeType directive_code() const
    {
        assert(kind == TokenKind::Directive);
        return std::get<5>(value);
    }

    QuotedString const &quoted_string() const
    {
        assert(kind == TokenKind::QuotedString);
        return std::get<QuotedString>(value);
    }

    CommentText const &comment_text() const
    {
        assert(kind == TokenKind::Comment);
        return std::get<CommentText>(value);
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

    bool operator==(KeywordCodeType const &code) const
    {
        return matches(code);
    }

    bool operator!=(KeywordCodeType const &code) const
    {
        return !matches(code);
    }

    [[nodiscard]] bool matches(TokenKind k) const { return kind == k; }
    [[nodiscard]] bool matches_symbol(int symbol) const { return matches(TokenKind::Symbol) && this->symbol_code() == symbol; }
    [[nodiscard]] bool matches_keyword(KeywordCodeType code) const { return matches(TokenKind::Keyword) && this->keyword_code() == code; }
    [[nodiscard]] bool matches_directive(DirectiveCodeType code) const { return matches(TokenKind::Directive) && this->directive_code() == code; }
    [[nodiscard]] bool is_identifier() const { return matches(TokenKind::Identifier); }
};

template<typename KeywordCodeType = int, typename DirectiveCodeType = int>
inline JSONValue encode(Token<KeywordCodeType, DirectiveCodeType> const &token)
{
    auto ret = JSONValue::object();
    set(ret, "kind", TokenKind_name(token.kind));
    set(ret, "text", token.text);
    set(ret, "location", token.location);

    auto encode_token_Number = [&ret, &token]() -> void {
        set(ret, "number_type", NumberType_name(token.number_type));
    };

    auto encode_token_QuotedString = [&ret, &token]() -> void {
        auto quote = JSONValue::object();
        auto quoted_string = std::get<QuotedString>(token.value);
        set(quote, "quote_type", quoted_string.quote_type);
        set(quote, "triple", quoted_string.triple);
        set(quote, "terminated", quoted_string.terminated);
        set(ret, "quoted_string", quote);
    };

    auto encode_token_Comment = [&ret, &token]() -> void {
        auto comment = JSONValue::object();
        auto comment_text = std::get<CommentText>(token.value);
        set(comment, "comment_type", comment_text.comment_type);
        set(comment, "terminated", comment_text.terminated);
        set(ret, "comment", comment);
    };

    auto encode_token_Keyword = [&ret, &token]() -> void {
        set(ret, "directive", std::get<4>(token.value));
    };

    auto encode_token_Directive = [&ret, &token]() -> void {
        set(ret, "directive", std::get<5>(token.value));
    };

    auto encode_token_Symbol = [&ret, &token]() -> void {
        set(ret, "symbol", std::get<6>(token.value));
    };

    switch (token.kind) {
#undef S
#define S(K)                \
    case TokenKind::K:      \
        encode_token_##K(); \
        break;
        VALUE_TOKENKINDS(S)
#undef S
    default:
        UNREACHABLE();
    }
    return ret;
}

template<typename KeywordCodeType, typename DirectiveCodeType>
inline Result<Token<KeywordCodeType, DirectiveCodeType>, JSONError> decode(JSONValue const &json)
{
    using T = Token<KeywordCodeType, DirectiveCodeType>;
    if (!json.is_object()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto token = Token {};
    token.kind = TRY_EVAL(json.try_get<TokenKind>("kind"));
    token.text = TRY_EVAL(json.try_get<std::string>("text"));
    token.location = TRY_EVAL(json.try_get<TokenLocation>("location"));

    auto decode_token_Number = [&json, &token]() -> Error<JSONError> {
        token.value = TRY_EVAL(json.try_get<NumberType>("number_type"));
        return {};
    };

    auto decode_token_QuotedString = [&json, &token]() -> Error<JSONError> {
        auto quote = TRY_EVAL(json.try_get<JSONValue>("quoted_string"));
        token.value = QuotedString {
            .quote_type = TRY_EVAL(quote.try_get<QuoteType>("quote_type")),
            .triple = TRY_EVAL(quote.try_get<bool>("triple")),
            .terminated = TRY_EVAL(quote.try_get<bool>("terminated")),
        };
        return {};
    };

    auto decode_token_Comment = [&json, &token]() -> Error<JSONError> {
        auto comment = TRY_EVAL(json.try_get<JSONValue>("comment"));
        token.value = CommentText {
            .comment_type = TRY_EVAL(comment.try_get<CommentType>("comment_type")),
            .terminated = TRY_EVAL(comment.try_get<bool>("terminated")),
        };
        return {};
    };

    auto decode_token_Keyword = [&json, &token]() -> Error<JSONError> {
        token.value = TRY_EVAL(json.try_get<KeywordCodeType>("code"));
        return {};
    };

    auto decode_token_Directive = [&json, &token]() -> Error<JSONError> {
        token.value = TRY_EVAL(json.try_get<DirectiveCodeType>("directive"));
        return {};
    };

    auto decode_token_Symbol = [&json, &token]() -> Error<JSONError> {
        token.value = TRY_EVAL(json.try_get<int>("symbol"));
        return {};
    };

    switch (token.kind) {
#undef S
#define S(K)                     \
    case TokenKind::K:           \
        TRY(decode_token_##K()); \
        break;
        VALUE_TOKENKINDS(S)
#undef S
    default:
        UNREACHABLE();
    }
    return {};
}

}

template<typename KeywordCodeType, typename DirectiveCodeType>
struct std::formatter<LibCore::Token<KeywordCodeType, DirectiveCodeType>, char> {
    using Token = LibCore::Token<KeywordCodeType, DirectiveCodeType>;
    bool quoted = false;

    template<class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext &ctx)
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("Invalid format args for QuotableString.");
        }
        return it;
    }

    template<class FmtContext>
    FmtContext::iterator format(Token const &token, FmtContext &ctx) const
    {
        std::ostringstream out;
        out << "[" << LibCore::TokenKind_name(token.kind) << "]";
        if (token != LibCore::TokenKind::EndOfLine && token != LibCore::TokenKind::EndOfFile) {
            out << " '" << token.text << "'";
        }
        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};
