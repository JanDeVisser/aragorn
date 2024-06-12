/*
 * Copyright (c) 2023, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <array>
#include <cctype>
#include <format>
#include <string>
#include <string_view>

#include <LibCore/Result.h>
#include <LibCore/StringUtil.h>
#include <LibCore/Token.h>

namespace LibCore {

template<typename KW>
inline std::map<std::string_view, KW> get_keywords()
{
    return {};
}

struct LexerErrorMessage {
    TokenLocation      location;
    std::string        message;
    std::string const &to_string() const
    {
        return message;
    }
};

template<typename KW = NoKeyword, typename Directive = NoDirective, bool Whitespace = false, bool Comments = false, bool BackquotedStrings = false>
class Lexer {
public:
    using LexerError = Error<LexerErrorMessage>;
    using T = Token<KW, Directive>;
    using LexerResult = Result<T, LexerErrorMessage>;

    [[nodiscard]] std::optional<std::pair<KW, size_t>> match_keyword(std::string_view const &text) const
    {
        std::optional<std::pair<KW, size_t>> ret;
        std::string_view                     matched;
        for (auto const &kw : m_keywords) {
            std::string_view keyword { kw.first };
            if (text.starts_with(keyword)) {
                if (keyword.length() > matched.length()) {
                    matched = keyword;
                    ret.emplace(kw.second, matched.length());
                }
            }
        }
        return ret;
    }

    [[nodiscard]] std::optional<std::string_view> keyword(KW const &code) const
    {
        if (auto kw = std::find_if(m_keywords.begin(), m_keywords.end(),
                [&code](auto const &k) { return k.second == code; });
            kw != m_keywords.end()) {
            return kw->first;
        }
        return {};
    }

    [[nodiscard]] std::optional<KW> code(std::string_view const &key) const
    {
        if (m_keywords.contains(key)) {
            return m_keywords.at(key);
        }
        return {};
    }

    Lexer()
    {
        m_keywords = get_keywords<KW>();
    }

    void push_source(std::string_view const &source, std::string_view const &name)
    {
        m_sources.emplace_back(this, source, name);
        if constexpr (!BackquotedStrings) {
            m_sources.back().quote_chars = "\"'";
        }
    }

    T const &peek()
    {
        if (m_current.has_value()) {
            trace(LEXER, "lexer.peek() -> {} [cached]", *m_current);
            return m_current.value();
        }
        while (!m_sources.empty()) {
            TokenKind k;
            while (true) {
                m_current = m_sources.back().peek_next();
                trace(LEXER, "lexer.peek() -> {} [source.peek_next()]", *m_current);
                k = m_current->kind;
                if constexpr (!Whitespace) {
                    if (k == TokenKind::Whitespace || k == TokenKind::EndOfLine) {
                        lex();
                        trace(LEXER, "skip it");
                        continue;
                    }
                }
                if constexpr (!Comments) {
                    if (k == TokenKind::Comment) {
                        lex();
                        trace(LEXER, "skip it");
                        continue;
                    }
                }
                break;
            }
            if (k != TokenKind::EndOfFile) {
                break;
            }
            m_sources.pop_back();
        }
        trace(LEXER, "lexer.peek() -> {} [caching it]", *m_current);
        return m_current.value();
    }

    T lex()
    {
        auto ret = peek();
        if (!m_sources.empty()) {
            m_sources.back().lex();
        }
        m_current.reset();
        return ret;
    }

    LexerResult expect(TokenKind kind)
    {
        auto ret = peek();
        if (!ret.matches(kind)) {
            return LexerErrorMessage { location(),
                std::format("Expected '{}'", TokenKind_name(kind)) };
        }
        return lex();
    }

    bool accept(TokenKind kind)
    {
        auto ret = peek();
        if (ret.matches(kind)) {
            lex();
            return true;
        }
        return false;
    }

    LexerError expect_keyword(KW code)
    {
        auto ret = peek();
        if (!ret.matches_keyword(code)) {
            return LexerErrorMessage {
                location(),
                std::format("Expected keyword"), // FIXME need KW code -> text mechanism
            };
        }
        lex();
        return {};
    }

    bool accept_keyword(KW code)
    {
        auto ret = peek();
        if (ret.matches_keyword(code)) {
            lex();
            return true;
        }
        return false;
    }

    LexerError expect_symbol(int symbol)
    {
        auto ret = peek();
        if (!ret.matches_symbol(symbol)) {
            return LexerErrorMessage { location(), std::format("Expected '{}'", static_cast<char>(symbol)) };
        }
        lex();
        return {};
    }

    bool accept_symbol(int symbol)
    {
        auto ret = peek();
        if (ret.matches_symbol(symbol)) {
            lex();
            return true;
        }
        return false;
    }

    LexerResult expect_identifier()
    {
        auto ret = peek();
        if (!ret.is_identifier()) {
            return LexerErrorMessage { location(), "Expected identifier" };
        }
        return lex();
    }

    bool next_matches(TokenKind kind)
    {
        auto n = peek();
        return n.matches(kind);
    }

    bool next_matches(int symbol)
    {
        auto n = peek();
        return n.matches_symbol(symbol);
    }

private:
    [[nodiscard]] TokenLocation const &location() const
    {
        assert(!m_sources.empty());
        return m_sources.back().location();
    }

    std::map<std::string_view, KW> m_keywords {};
    std::optional<T>               m_current {};

    class Source {
    public:
        char const *quote_chars;

        TokenLocation const &location() const
        {
            return m_location;
        }

        Source(Lexer *lexer, std::string_view const &src, std::string_view const &name)
            : m_buffer(src)
            , m_lexer(lexer)
            , quote_chars("\"'`")
        {
            m_location.file = name;
        }

        T const &peek_next()
        {
            if (m_current.has_value()) {
                return m_current.value();
            }

            auto peek = [this]() -> T {
                auto scan_number = [this]() -> T {
                    NumberType type = NumberType::Integer;
                    int        ix = 0;
                    int (*predicate)(int) = isdigit;
                    if (m_buffer[1] && m_buffer[0] == '0') {
                        if (m_buffer[1] == 'x' || m_buffer[1] == 'X') {
                            if (!m_buffer[2] || !isxdigit(m_buffer[2])) {
                                return T::number(NumberType::Integer, m_buffer.substr(0, 1));
                            }
                            type = NumberType::HexNumber;
                            predicate = isxdigit;
                            ix = 2;
                        } else if (m_buffer[1] == 'b' || m_buffer[1] == 'B') {
                            if (!m_buffer[2] || !isbdigit(m_buffer[2])) {
                                return T::number(NumberType::Integer, m_buffer.substr(0, 1));
                            }
                            type = NumberType::BinaryNumber;
                            predicate = isbdigit;
                            ix = 2;
                        }
                    }

                    while (true) {
                        if (ix >= m_buffer.length()) {
                            return T::number(type, m_buffer);
                        }
                        char ch = m_buffer[ix];
                        if (!predicate(ch) && ((ch != '.') || (type == NumberType::Decimal))) {
                            // FIXME lex '1..10' as '1', '..', '10'. It will now lex as '1.', '.', '10'
                            return T::number(type, m_buffer.substr(0, ix));
                        }
                        if (ch == '.') {
                            if (type != NumberType::Integer) {
                                return T::number(type, m_buffer.substr(0, ix));
                            }
                            type = NumberType::Decimal;
                        }
                        ++ix;
                    }
                };

                auto block_comment = [this](size_t ix) -> T {
                    for (; m_buffer[ix] && m_buffer[ix] != '\n' && (m_buffer[ix - 1] != '*' || m_buffer[ix] != '/'); ++ix)
                        ;
                    if (!m_buffer[ix]) {
                        return T::comment(CommentType::Block, m_buffer.substr(0, ix), false);
                    }
                    if (m_buffer[ix] == '\n') {
                        m_in_comment = true;
                        return T::comment(CommentType::Block, m_buffer.substr(0, ix), false);
                    }
                    m_in_comment = false;
                    return T::comment(CommentType::Block, m_buffer.substr(0, ix + 1), true);
                };

                if (m_buffer.empty()) {
                    return T::end_of_file();
                }
                if (m_in_comment) {
                    if (m_buffer[0] == '\n') {
                        return T::end_of_line(m_buffer.substr(0, 1));
                    }
                    return block_comment(0);
                }
                if (strchr(quote_chars, m_buffer[0])) {
                    size_t ix = 1;
                    while (ix < m_buffer.length() && m_buffer[ix] != m_buffer[0]) {
                        if (m_buffer[ix] == '\\')
                            ++ix;
                        if (m_buffer[ix])
                            ++ix;
                    }
                    return T::string(static_cast<QuoteType>(m_buffer[0]), m_buffer.substr(0, ix + 1), ix < m_buffer.length());
                }
                switch (m_buffer[0]) {
                case '/':
                    switch (m_buffer[1]) {
                    case '/': {
                        size_t ix = 2;
                        for (; m_buffer[ix] && m_buffer[ix] != '\n'; ++ix)
                            ;
                        return T::comment(CommentType::Line, m_buffer.substr(0, ix));
                    }
                    case '*': {
                        return block_comment(2);
                    }
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                if (m_buffer[0] == '\n') {
                    return T::end_of_line(m_buffer.substr(0, 1));
                }
                if (isspace(m_buffer[0])) {
                    size_t ix = 0;
                    for (; isspace(m_buffer[ix]) && m_buffer[ix] != '\n'; ++ix)
                        ;
                    return T::whitespace(m_buffer.substr(0, ix));
                }
                if (isdigit(m_buffer[0])) {
                    return scan_number();
                }
                if (isalpha(m_buffer[0]) || m_buffer[0] == '_') {
                    size_t ix = 0;
                    for (; isalnum(m_buffer[ix]) || m_buffer[ix] == '_'; ++ix)
                        ;
                    if (auto kw = m_lexer->code(m_buffer.substr(0, ix)); kw) {
                        return T::keyword(kw.value(), m_buffer.substr(0, ix));
                    }
                    return T::identifier(m_buffer.substr(0, ix));
                }
                if (auto kw = m_lexer->match_keyword(m_buffer); kw) {
                    return T::keyword(kw.value().first, m_buffer.substr(0, kw.value().second));
                }
                Token ret = T::symbol((int) m_buffer[0]);
                return ret;
            };
            m_current = peek();
            m_current.value().location = m_location;
            return m_current.value();
        }

        void lex()
        {
            if (!m_current) {
                return;
            }
            m_location.index += m_current->text.length();
            m_buffer = m_buffer.substr(m_current->text.length());
            if (m_current->kind == TokenKind::EndOfLine) {
                ++m_location.line;
                m_location.column = 0;
            } else {
                m_location.column += m_current->text.length();
            }
            m_current.reset();
        }

    private:
        std::string_view m_buffer;
        Lexer           *m_lexer;
        TokenLocation    m_location {};
        std::optional<T> m_current {};
        bool             m_in_comment { false };
    };

    std::vector<Source> m_sources {};
};

}
