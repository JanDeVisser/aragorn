/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <LibCore/Lexer.h>

#include <App/Buffer.h>
#include <App/Mode.h>

namespace Aragorn {

using namespace LibCore;

template<typename Keywords>
class LexerMode : public Mode {
public:
    constexpr static size_t config_tab_size = 4;

    explicit LexerMode(pBuffer const &buffer)
        : Mode(std::dynamic_pointer_cast<Widget>(buffer))
    {
    }

    void initialize_source() override
    {
        pBuffer const &buffer = std::dynamic_pointer_cast<Buffer>(parent);
        m_lexer.push_source(*buffer);
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
            Theme::the().get_scope(token.kind)
        };
    }

protected:
    Lexer<Buffer, Keywords, NoDirective, true, true, true> m_lexer {};

private:
    size_t m_token_col { 0 };
};

class PlainText : public LexerMode<NoKeyword> {
public:
    PlainText(pBuffer const &buffer)
        : LexerMode(buffer)
    {
    }
};

#define C_KEYWORDS(S) \
    S(alignas)        \
    S(alignof)        \
    S(auto)           \
    S(bool)           \
    S(break)          \
    S(case)           \
    S(char)           \
    S(const)          \
    S(constexpr)      \
    S(continue)       \
    S(default)        \
    S(do)             \
    S(double)         \
    S(else)           \
    S(enum)           \
    S(extern)         \
    S(false)          \
    S(float)          \
    S(for)            \
    S(goto)           \
    S(if)             \
    S(inline)         \
    S(int)            \
    S(long)           \
    S(nullptr)        \
    S(register)       \
    S(restrict)       \
    S(return)         \
    S(short)          \
    S(signed)         \
    S(sizeof)         \
    S(static)         \
    S(static_assert)  \
    S(struct)         \
    S(switch)         \
    S(thread_local)   \
    S(true)           \
    S(typedef)        \
    S(typeof)         \
    S(typeof_unqual)  \
    S(union)          \
    S(unsigned)       \
    S(void)           \
    S(volatile)       \
    S(while)          \
    S(_Alignas)       \
    S(_Alignof)       \
    S(_Atomic)        \
    S(_BitInt)        \
    S(_Bool)          \
    S(_Complex)       \
    S(_Decimal128)    \
    S(_Decimal32)     \
    S(_Decimal64)     \
    S(_Generic)       \
    S(_Imaginary)     \
    S(_Noreturn)      \
    S(_Static_assert) \
    S(_Thread_local)

enum class CKeyword {
#undef S
#define S(kw) C_##kw,
    C_KEYWORDS(S)
#undef S
};

class CMode : public LexerMode<CKeyword> {
public:
    explicit CMode(pBuffer const &buffer)
        : LexerMode(buffer)
    {
    }
};

}

namespace LibCore {

using namespace Aragorn;

template<>
inline std::map<std::string_view, CKeyword> get_keywords()
{
    return std::map<std::string_view, CKeyword> {
#undef S
#define S(kw) { #kw, CKeyword::C_##kw },
        C_KEYWORDS(S)
#undef S
    };
}

}
