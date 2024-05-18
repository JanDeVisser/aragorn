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
#include <LibCore/Token.h>

namespace LibCore {

struct Language {
    Language() = default;
    Language(Language const &) = default;
    [[nodiscard]] std::optional<std::string_view> keyword(KeywordCode const &code) const
    {
        if (auto kw = std::find_if(keywords.begin(), keywords.end(),
                [&code](auto const &k) { return k.code == code; });
            kw != keywords.end()) {
            return kw->keyword;
        }
        return {};
    }

    [[nodiscard]] std::optional<KeywordCode> code(std::string_view const &key) const
    {
        if (auto kw = std::find_if(keywords.begin(), keywords.end(),
                [&key](auto const &k) { return k.keyword == key; });
            kw != keywords.end()) {
            return kw->code;
        }
        return {};
    }

    std::string                   name {};
    std::vector<Keyword>          keywords {};
    Token                         preprocessor_trigger {};
    std::vector<std::string_view> directives {};
};

class Source {
public:
    Source(Language *language, std::string_view const &src, std::string_view const &name);
    Token const                       &peek_next();
    void                               lex();
    [[nodiscard]] TokenLocation const &location() const { return m_location; }

private:
    std::string_view     m_buffer;
    Language            *m_language;
    TokenLocation        m_location {};
    std::optional<Token> m_current {};
    bool                 m_in_comment { false };
};

struct LexerErrorMessage {
    TokenLocation location;
    std::string   message;
};

using LexerError = Error<LexerErrorMessage>;
using LexerResult = Result<Token, LexerErrorMessage>;

template<bool Whitespace = false, bool Comments = false>
class Lexer {
public:
    Lexer() = default;

    explicit Lexer(Language const &language)
        : m_language(language)
    {
    }

    void push_source(std::string_view const &source, std::string_view const &name)
    {
        m_sources.emplace_back(&m_language, source, name);
    }

    Token const& peek()
    {
        if (m_current.has_value()) {
            return m_current.value();
        }
        while (!m_sources.empty()) {
            TokenKind k;
            while (true) {
                m_current = m_sources.back().peek_next();
                k = m_current->kind;
                if constexpr (!Whitespace) {
                    if (k == TokenKind::Whitespace || k == TokenKind::EndOfLine) {
                        lex();
                        continue;
                    }
                }
                if constexpr (!Comments) {
                    if (k == TokenKind::Comment) {
                        lex();
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
        return m_current.value();
    }

    Token lex()
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

    LexerError expect_symbol(int symbol)
    {
        auto ret = peek();
        if (!ret.matches(symbol)) {
            return LexerErrorMessage { location(), std::format("Expected '{}'", static_cast<char>(symbol)) };
        }
        lex();
        return {};
    }

    bool accept_symbol(int symbol)
    {
        auto ret = peek();
        if (ret.matches(symbol)) {
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
        return n.matches(symbol);
    }

private:
    [[nodiscard]] TokenLocation const& location() const
    {
        assert(!m_sources.empty());
        return m_sources.back().location();
    }

    std::vector<Source>  m_sources {};
    Language             m_language {};
    std::optional<Token> m_current {};
};

}
