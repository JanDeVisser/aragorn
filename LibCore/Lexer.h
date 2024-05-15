/*
 * Copyright (c) 2023, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cctype>
#include <functional>
#include <string>
#include <string_view>

#include <LibCore/Token.h>

namespace LibCore {

#define NO_DIRECTIVE (-1)

class Lexer;

struct Language {
    Language() = default;
    Language(Language const &) = default;
    [[nodiscard]] std::optional<std::string_view> keyword(KeywordCode const &code) const;
    [[nodiscard]] std::optional<KeywordCode>      code(std::string_view const &kw) const;

    using DirectiveHandler = std::function<std::optional<DirectiveCode>(Lexer &lexer, std::optional<DirectiveCode> directive)>;
    std::string                   name {};
    std::vector<Keyword>          keywords {};
    Token                         preprocessor_trigger {};
    std::vector<std::string_view> directives {};
    DirectiveHandler              directive_handler { nullptr };
    void                         *language_data { nullptr };
};

struct Source {
    explicit Source(std::string_view const &src, std::string_view const &name);
    std::string_view source;
    TokenLocation    location {};
};

struct LexerErrorMessage {
    TokenLocation location;
    std::string   message;
};

using LexerError = Error<LexerErrorMessage>;
using LexerResult = Result<Token, LexerErrorMessage>;

class Lexer {
public:
    Lexer() = default;
    explicit Lexer(Language const &language);
    void                           push_source(std::string_view const &source, std::string_view const &name);
    [[nodiscard]] std::string_view source() const;
    Token                          peek();
    Token                          next();
    Token                          lex();
    LexerResult                    expect(TokenKind kind);
    LexerError                     expect_symbol(int symbol);
    LexerResult                    expect_identifier();
    bool                           next_matches(TokenKind kind);
    bool                           next_matches_symbol(int symbol);
    bool                           whitespace_significant { false };
    bool                           include_comments { false };

private:
    [[nodiscard]] TokenLocation location() const;
    Token                       set_current(Token const &token);
    void                        pop_source();
    Token                       peek_next();
    Token                       directive_handle(Token const &trigger);
    Token                       block_comment(std::string_view const &buffer, size_t ix);

    std::vector<Source> m_sources {};
    Token               m_current {};
    Language            m_language {};
    bool                m_in_comment { false };
    std::optional<int>  m_current_directive {};
    void               *language_data { nullptr };
};

}
