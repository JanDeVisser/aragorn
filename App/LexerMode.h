/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "LSP/LSP.h"

#include <LibCore/Lexer.h>

#include <App/Buffer.h>
#include <App/Mode.h>
#include <LSP/LSP.h>

namespace Aragorn {
using namespace LibCore;

struct BufferSource {
    pBuffer buffer;

    explicit BufferSource(pBuffer buffer)
        : buffer(std::move(buffer))
    {
    }

    wchar_t operator[](size_t ix) const
    {
        return (*buffer)[ix];
    }

    [[nodiscard]] std::wstring_view substr(size_t pos, size_t len = std::wstring_view::npos) const
    {
        UNREACHABLE();
    }

    [[nodiscard]] size_t length() const
    {
        return buffer->length();
    }
};

template<typename Matcher>
class ModeLexer {
public:
    using Lexer = Lexer<BufferSource, Matcher, wchar_t, true, true, true>;
    using Token = typename Lexer::Token;

    void initialize_source(std::shared_ptr<Buffer> const &buffer)
    {
        m_lexer.push_source(BufferSource(buffer));
    }

    Token lex()
    {
        return m_lexer.lex();
    }

    [[nodiscard]] Scope get_scope(Token const &token) const
    {
        std::string_view scope = Matcher {}.get_scope(token);
        return Theme::the().get_scope(scope);
    }

private:
    Lexer m_lexer {};
};

template<typename KeywordCodes = NoKeywordCode, typename KeywordCategories = NoKeywordCategory>
struct SimpleLexer : ModeLexer<EnumKeywords<BufferSource, KeywordCategories, KeywordCodes>> {
    using Keywords = KeywordCodes;
    using Categories = KeywordCategories;

    static BufferEventListener event_listener()
    {
        return [](pBuffer, BufferEvent const &) -> void {
        };
    }
};

template<typename Lexer>
class LexerMode : public Mode {
public:
    using Token = LibCore::Token<typename Lexer::Keywords, typename Lexer::Categories>;
    constexpr static size_t config_tab_size = 4;

    explicit LexerMode(pBuffer const &buffer)
        : Mode(std::dynamic_pointer_cast<Widget>(buffer))
    {
    }

    BufferEventListener event_listener() const override
    {
        return Lexer::event_listener();
    }

    void initialize_source() override
    {
        m_lexer.initialize_source(std::dynamic_pointer_cast<Buffer>(parent));
        m_token_col = 0;
    }

    DisplayToken lex() override
    {
        auto const token = m_lexer.lex();
        auto       col = m_token_col;
        switch (token.kind) {
        case TokenKind::EndOfLine:
            m_token_col = 0;
            break;
        case TokenKind::Tab:
            m_token_col = ((m_token_col / config_tab_size) + 1) * config_tab_size;
            break;
        default:
            m_token_col += token.location.length;
            break;
        }
        return {
            token.location.index,
            token.location.length,
            token.location.line,
            col,
            token.kind,
            m_lexer.get_scope(token)
        };
    }

protected:
    Lexer m_lexer {};

private:
    size_t m_token_col { 0 };
};

using PlainTextLexer = SimpleLexer<BufferSource>;

}
