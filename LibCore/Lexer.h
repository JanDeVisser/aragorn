/*
 * Copyright (c) 2023, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cctype>
#include <format>
#include <string>
#include <string_view>

#include <LibCore/Result.h>
#include <LibCore/StringUtil.h>
#include <LibCore/Token.h>

namespace LibCore {

template<typename KW>
std::map<std::string_view, KW> get_keywords()
{
    return {};
}

struct LexerErrorMessage {
    TokenLocation location;
    std::string   message;

    [[nodiscard]] std::string const &to_string() const
    {
        return message;
    }
};

template<typename Buffer, typename KW = NoKeyword, typename Directive = NoDirective, bool Whitespace = false, bool Comments = false, bool BackquotedStrings = false>
class Lexer {
public:
    using LexerError = Error<LexerErrorMessage>;
    using T = Token<KW, Directive>;
    using LexerResult = Result<T, LexerErrorMessage>;

    [[nodiscard]] std::optional<std::pair<KW, size_t>> match_keyword(Buffer const &text, size_t index) const
    {
        std::optional<std::pair<KW, size_t>> ret;
        std::string_view                     matched;
        for (auto const &kw : m_keywords) {
            std::string_view keyword { kw.first };
            for (auto ix = 0ul; ix + index < text.length() && ix < keyword.length(); ++ix) {
                if (text[index + ix] != keyword[ix]) {
                    goto next_keyword;
                }
            }
            if (keyword.length() > matched.length()) {
                matched = keyword;
                ret.emplace(kw.second, matched.length());
            }
        next_keyword:
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

    [[nodiscard]] std::optional<KW> code(std::string const &key) const
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

    void push_source(Buffer const &source)
    {
        m_sources.emplace_back(this, source);
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
                    if (k == TokenKind::Whitespace || k == TokenKind::Tab || k == TokenKind::EndOfLine) {
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
        if (auto ret = peek(); !ret.matches(kind)) {
            return LexerErrorMessage { location(),
                std::format("Expected '{}'", TokenKind_name(kind)) };
        }
        return lex();
    }

    bool accept(TokenKind kind)
    {
        if (auto ret = peek(); ret.matches(kind)) {
            lex();
            return true;
        }
        return false;
    }

    LexerError expect_keyword(KW code)
    {
        if (auto ret = peek(); !ret.matches_keyword(code)) {
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
        if (auto ret = peek(); ret.matches_keyword(code)) {
            lex();
            return true;
        }
        return false;
    }

    LexerError expect_symbol(int symbol)
    {
        if (auto ret = peek(); !ret.matches_symbol(symbol)) {
            return LexerErrorMessage { location(), std::format("Expected '{}'", static_cast<char>(symbol)) };
        }
        lex();
        return {};
    }

    bool accept_symbol(int symbol)
    {
        if (auto ret = peek(); ret.matches_symbol(symbol)) {
            lex();
            return true;
        }
        return false;
    }

    LexerResult expect_identifier()
    {
        if (auto ret = peek(); !ret.is_identifier()) {
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

        [[nodiscard]] TokenLocation const &location() const
        {
            return m_location;
        }

        Source(Lexer *lexer, Buffer const &src)
            : quote_chars("\"'`")
            , m_buffer(src)
            , m_lexer(lexer)
        {
        }

        T peek()
        {
            if (m_index >= m_buffer.length()) {
                return T::end_of_file();
            }
            m_scanned.clear();
            auto cur = m_buffer[m_index];
            if (m_in_comment) {
                if (cur == '\n') {
                    ++m_index;
                    return T::end_of_line();
                }
                return block_comment(m_index);
            }
            if (strchr(quote_chars, cur)) {
                auto ix { m_index + 1 };
                while (ix < m_buffer.length() && m_buffer[ix] != cur) {
                    if (m_buffer[ix] == '\\') {
                        ++ix;
                    }
                    if (ix < m_buffer.length()) {
                        ++ix;
                    }
                }
                auto ret { T::string(static_cast<QuoteType>(cur), ix < m_buffer.length()) };
                m_index = ix;
                return ret;
            }
            switch (cur) {
            case '/':
                switch (m_buffer[m_index + 1]) {
                case '/': {
                    size_t ix = m_index + 2;
                    for (; ix < m_buffer.length() && m_buffer[ix] != '\n'; ++ix)
                        ;
                    m_index = ix;
                    return T::comment(CommentType::Line);
                }
                case '*': {
                    return block_comment(m_index + 2);
                }
                default:
                    break;
                }
                break;
            case '\n':
                ++m_index;
                return T::end_of_line();
            case '\t':
                ++m_index;
                return T::tab();
            default:
                break;
            }
            if (isspace(cur)) {
                size_t ix = m_index;
                for (; ix < m_buffer.length() && isspace(m_buffer[ix]) && m_buffer[ix] != '\t' && m_buffer[ix] != '\n'; ++ix)
                    ;
                m_index = ix;
                return T::whitespace();
            }
            if (isdigit(cur)) {
                return scan_number();
            }
            if (isalpha(cur) || cur == '_') {
                size_t ix = m_index;
                for (; isalnum(m_buffer[ix]) || m_buffer[ix] == '_'; ++ix) {
                    m_scanned += m_buffer[ix];
                }
                auto kw = m_lexer->code(m_scanned);
                m_index = ix;
                if (kw) {
                    return T::keyword(kw.value());
                }
                return T::identifier();
            }
            auto kw = m_lexer->match_keyword(m_buffer, m_index);
            if (kw) {
                m_index += (*kw).second;
                return T::keyword((*kw).first);
            }
            auto ret = T::symbol(static_cast<int>(cur));
            m_index += 1;
            return ret;
        }

        T const &peek_next()
        {
            if (m_current.has_value()) {
                return *m_current;
            }

            m_current = peek();
            m_location.length = m_index - m_location.index;
            m_current->location = m_location;
            return *m_current;
        }

        void lex()
        {
            if (!m_current) {
                return;
            }
            m_location.index = m_index;
            if (m_current->kind == TokenKind::EndOfLine) {
                ++m_location.line;
                m_location.column = 0;
            } else {
                m_location.column += m_location.length;
            }
            m_location.length = 0;
            m_current.reset();
        }

        T block_comment(size_t ix)
        {
            for (; ix < m_buffer.length() && m_buffer[ix] != '\n' && (m_buffer[ix - 1] != '*' || m_buffer[ix] != '/'); ++ix)
                ;
            m_index = ix;
            if (ix >= m_buffer.length()) {
                return T::comment(CommentType::Block, false);
            }
            if (m_buffer[ix] == '\n') {
                m_in_comment = true;
                return T::comment(CommentType::Block, false);
            }
            m_in_comment = false;
            return T::comment(CommentType::Block, true);
        }

        T scan_number()
        {
            auto type = NumberType::Integer;
            auto cur = m_buffer[m_index];
            int  ix = m_index;
            int (*predicate)(int) = isdigit;
            if (m_index < m_buffer.length() - 1 && cur == '0') {
                if (m_buffer[m_index + 1] == 'x' || m_buffer[m_index + 1] == 'X') {
                    if (m_index == m_buffer.length() - 2 || !isxdigit(m_buffer[m_index + 2])) {
                        m_index += 1;
                        return T::number(NumberType::Integer);
                    }
                    type = NumberType::HexNumber;
                    predicate = isxdigit;
                    ix = 2;
                } else if (m_buffer[m_index + 1] == 'b' || m_buffer[m_index + 1] == 'B') {
                    if (m_index == m_buffer.length() - 2 || !isbdigit(m_buffer[m_index + 2])) {
                        m_index += 1;
                        return T::number(NumberType::Integer);
                    }
                    type = NumberType::BinaryNumber;
                    predicate = isbdigit;
                    ix = m_index + 2;
                }
            }

            while (ix < m_buffer.length()) {
                char const ch = m_buffer[ix];
                if (!predicate(ch) && ((ch != '.') || (type == NumberType::Decimal))) {
                    // FIXME lex '1..10' as '1', '..', '10'. It will now lex as '1.', '.', '10'
                    m_index = ix;
                    return T::number(type);
                }
                if (ch == '.') {
                    if (type != NumberType::Integer) {
                        m_index = ix;
                        return T::number(type);
                    }
                    type = NumberType::Decimal;
                }
                ++ix;
            }
            return T::number(type);
        }

    private:
        Buffer const    &m_buffer;
        size_t           m_index { 0 };
        Lexer           *m_lexer;
        TokenLocation    m_location {};
        std::optional<T> m_current {};
        bool             m_in_comment { false };
        std::string      m_scanned;
    };

    std::vector<Source> m_sources {};
};

}
