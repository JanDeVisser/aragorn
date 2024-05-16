/*
 * Copyright (c) 2023, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <LibCore/Lexer.h>

namespace LibCore {

static int   isbdigit(int ch);
static Token scan_number(char const *buffer);

int isbdigit(int ch)
{
    return ch == '0' || ch == '1';
}

Token scan_number(std::string_view const &buffer)
{
    NumberType type = NumberType::Integer;
    int        ix = 0;
    int (*predicate)(int) = isdigit;
    if (buffer[1] && buffer[0] == '0') {
        if (buffer[1] == 'x' || buffer[1] == 'X') {
            if (!buffer[2] || !isxdigit(buffer[2])) {
                return Token::number(NumberType::Integer, buffer.substr(0, 1));
            }
            type = NumberType::HexNumber;
            predicate = isxdigit;
            ix = 2;
        } else if (buffer[1] == 'b' || buffer[1] == 'B') {
            if (!buffer[2] || !isbdigit(buffer[2])) {
                return Token::number(NumberType::Integer, buffer.substr(0, 1));
            }
            type = NumberType::BinaryNumber;
            predicate = isbdigit;
            ix = 2;
        }
    }

    while (true) {
        if (ix >= buffer.length()) {
            return Token::number(type, buffer);
        }
        char ch = buffer[ix];
        if (!predicate(ch) && ((ch != '.') || (type == NumberType::Decimal))) {
            // FIXME lex '1..10' as '1', '..', '10'. It will now lex as '1.', '.', '10'
            return Token::number(type, buffer.substr(0, ix));
        }
        if (ch == '.') {
            if (type != NumberType::Integer) {
                return Token::number(type, buffer.substr(0, ix));
            }
            type = NumberType::Decimal;
        }
        ++ix;
    }
}

std::optional<std::string_view> Language::keyword(KeywordCode const &code) const
{
    if (auto kw = std::find_if(keywords.begin(), keywords.end(),
        [&code](auto const& k) { return k.code == code; }); kw != keywords.end()) {
        return kw->keyword;
    }
    return {};
}

std::optional<KeywordCode> Language::code(std::string_view const &keyword) const
{
    for (auto const& kw : keywords) {
        if (kw.keyword == keyword) {
            return kw.code;
        }
    }
    return {};
}

Source::Source(std::string_view const &src, std::string_view const &name)
    : source(src)
{
    location.file = name;
}

Lexer::Lexer(Language const &language)
    : m_language(language)
{
}

std::string_view Lexer::source() const
{
    if (m_sources.empty()) {
        return {};
    }
    return m_sources.back().source;
}

TokenLocation Lexer::location() const
{
    if (!m_sources.empty()) {
        return m_sources.back().location;
    }
    return TokenLocation {};
}

void Lexer::push_source(std::string_view const &source, std::string_view const &name)
{
    m_sources.emplace_back(source, name);
}

void Lexer::pop_source()
{
    if (!m_sources.empty()) {
        m_sources.pop_back();
    }
}

Token Lexer::directive_handle(Token const& trigger)
{
    auto   buffer = source();
    size_t directive_start = 1;

    if (m_language.directives.empty()) {
        return trigger;
    }
    while (buffer[directive_start] && isspace(buffer[directive_start])) {
        ++directive_start;
    }
    if (!buffer[directive_start]) {
        return trigger;
    }
    size_t directive_end = directive_start;
    while (buffer[directive_end] && isalpha(buffer[directive_end]))
        ++directive_end;
    if (directive_end == directive_start) {
        return trigger;
    }
    auto directive = buffer.substr(directive_start, directive_end - directive_start);
    m_current_directive = {};
    for (auto ix = 0; ix < m_language.directives.size(); ++ix) {
        if (directive == m_language.directives[ix]) {
            m_current_directive = ix;
            return Token::directive(ix, buffer.substr(0, directive_end));
        }
    }
    return trigger;
}

Token Lexer::set_current(Token const &token)
{
    m_current = token;
    if (!m_sources.empty()) {
        m_current.location = m_sources.back().location;
    }
    return m_current;
}

Token Lexer::peek()
{
    if (!m_current.matches(TokenKind::Unknown)) {
        return m_current;
    }
    if (m_current_directive && m_language.directive_handler) {
        m_current_directive = m_language.directive_handler(*this, m_current_directive);
        if (m_current_directive) {
            assert(!m_current.matches(TokenKind::Unknown));
            return m_current;
        }
    }
    return set_current(peek_next());
}

Token Lexer::block_comment(std::string_view const &buffer, size_t ix)
{
    for (; buffer[ix] && buffer[ix] != '\n' && (buffer[ix - 1] != '*' || buffer[ix] != '/'); ++ix)
        ;
    if (!buffer[ix]) {
        return Token::comment(CommentType::Block, buffer.substr(0, ix), false);
    }
    if (buffer[ix] == '\n') {
        m_in_comment = true;
        return Token::comment(CommentType::Block, buffer.substr(0, ix), false);
    }
    m_in_comment = false;
    return Token::comment(CommentType::Block, buffer.substr(0, ix + 1), true);
}

Token Lexer::peek_next()
{
    if (!m_current.matches(TokenKind::Unknown)) {
        return m_current;
    }
    auto buffer = source();
    if (buffer.empty()) {
        return Token::end_of_file();
    }
    if (m_in_comment) {
        if (buffer[0] == '\n') {
            return Token::end_of_line(buffer.substr(0, 1));
        }
        return block_comment(buffer, 0);
    }
    switch (buffer[0]) {
    case '\'':
    case '"':
    case '`': {
        size_t ix = 1;
        while (buffer[ix] && buffer[ix] != buffer[0]) {
            if (buffer[ix] == '\\')
                ++ix;
            if (buffer[ix])
                ++ix;
        }
        return Token::string(static_cast<QuoteType>(buffer[0]), buffer.substr(0, ix + 1), buffer[ix] != 0);
    }
    case '/':
        switch (buffer[1]) {
        case '/': {
            size_t ix = 2;
            for (; buffer[ix] && buffer[ix] != '\n'; ++ix)
                ;
            return Token::comment(CommentType::Line, buffer.substr(0, ix));
        }
        case '*': {
            return block_comment(buffer, 2);
        }
        default:
            break;
        }
        break;
    default:
        break;
    }
    if (buffer[0] == '\n') {
        return Token::end_of_line(buffer.substr(0, 1));
    }
    if (isspace(buffer[0])) {
        size_t ix = 0;
        for (; isspace(buffer[ix]) && buffer[ix] != '\n'; ++ix)
            ;
        return Token::whitespace(buffer.substr(0, ix));
    }
    if (isdigit(buffer[0])) {
        return scan_number(buffer);
    }
    if (isalpha(buffer[0]) || buffer[0] == '_') {
        size_t ix = 0;
        for (; isalnum(buffer[ix]) || buffer[ix] == '_'; ++ix)
            ;
        for (auto const& kw : m_language.keywords) {
            std::string_view keyword { kw.keyword };
            if (keyword.length() == ix && buffer.starts_with(keyword)) {
                return Token::keyword(kw.code, buffer.substr(0, ix));
            }
        }
        return Token::identifier(buffer.substr(0, ix));
    }
    Keyword  matched {};
    for (auto const & kw : m_language.keywords) {
        std::string_view keyword {kw.keyword};
        if (buffer.starts_with(keyword)) {
            if (keyword.length() > matched.keyword.length()) {
                matched = kw;
            }
        }
    }
    if (!matched.keyword.empty()) {
        return Token::keyword(matched.code, buffer.substr(0, matched.keyword.length()));
    }
    Token ret = Token::symbol((int) buffer[0]);
    if (!m_current_directive && m_language.preprocessor_trigger == ret.symbol_code) {
        return directive_handle(ret);
    }
    return ret;
}

Token Lexer::next()
{
    Token token {};
    while (!m_sources.empty()) {
        for (token = peek(); !token.matches(TokenKind::EndOfFile); token = peek()) {
            // clang-format off
            if ((whitespace_significant && (token == TokenKind::Whitespace || token == TokenKind::EndOfLine)) ||
                (include_comments && token == TokenKind::Comment) ||
                (token != TokenKind::Whitespace && token != TokenKind::Comment && token != TokenKind::EndOfLine)) {
                return token;
            }
            // clang-format on
            lex();
        }
        pop_source();
    }
    return token;
}

Token Lexer::lex()
{
    Token ret = m_current;
    if (ret.matches(TokenKind::Unknown)) {
        ret = next();
    }
    if (!m_sources.empty()) {
        auto &src = m_sources.back();
        src.location.index += ret.text.length();
        src.source = src.source.substr(ret.text.length());
        if (ret.matches(TokenKind::EndOfLine)) {
            ++src.location.line;
            src.location.column = 0;
        } else {
            src.location.column += ret.text.length();
        }
        if (src.source.empty()) {
            src.source = {};
        }
    }
    m_current = Token {};
    return ret;
}

LexerResult Lexer::expect(TokenKind kind)
{
    Token ret = next();
    if (!ret.matches(kind)) {
        return LexerErrorMessage { location(),
            std::format("Expected '{}'", TokenKind_name(kind)) };
    }
    return lex();
}

LexerError Lexer::expect_symbol(int symbol)
{
    Token ret = next();
    if (ret.matches(symbol)) {
        return LexerErrorMessage { location(), std::format("Expected '{}'", static_cast<char>(symbol)) };
    }
    lex();
    return {};
}

LexerResult Lexer::expect_identifier()
{
    Token ret = next();
    if (!ret.is_identifier()) {
        return LexerErrorMessage { location(), "Expected identifier" };
    }
    return lex();
}

bool Lexer::next_matches(TokenKind kind)
{
    auto n = next();
    return n.matches(kind);
}

bool Lexer::next_matches_symbol(int symbol)
{
    auto n = next();
    return n.matches(symbol);
}

}
