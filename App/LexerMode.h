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
S(for)                \
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

#define C_DIRECTIVES(S)      \
    S(include, "#include")   \
    S(define, "#define")     \
    S(if, "#if")             \
    S(ifdef, "#ifdef")       \
    S(ifndef, "#ifndef")     \
    S(else, "#else")         \
    S(elif, "#elif")         \
    S(elifdef, "#elifdef")   \
    S(elifndef, "#elifndef") \
    S(endif, "#endif")

#define CPP_KEYWORDS(S) \
    S(namespace)        \
    S(class)            \
    S(template)         \
    S(typename)

enum class CKeyword {
#undef S
#define S(kw) C_##kw,
    C_KEYWORDS(S)
#undef S
#define S(kw) CPP_##kw,
        CPP_KEYWORDS(S)
#undef S
#define S(D, STR) Dir_##D,
            C_DIRECTIVES(S)
#undef S
};

enum class CCategory {
    CKeyword,
    CPPKeyword,
    Directive,
    DirectiveArg,
};

template<typename Buffer>
struct CMatcher {
    using Keywords = CKeyword;
    using Categories = CCategory;
    using Token = Token<CCategory, CKeyword>;

    enum class State {
        NoState,
        Define,
        Ifdef,
        Include,
        PreprocExpression,
    };

    State state { State::NoState };

    std::optional<std::tuple<Token, size_t>> pre_match(Buffer const &buffer, size_t index)
    {
        if (state != State::NoState) {
            if (buffer[index] == '\t') {
                return std::tuple { Token::tab(), 1 };
            }
            auto ix = index;
            while (ix < buffer.length() && buffer[ix] == ' ') {
                ++ix;
            }
            if (ix > index) {
                return std::tuple { Token::whitespace(), ix - index };
            }
            switch (state) {
            case State::Include: {
                if (buffer[ix] == '\"' || buffer[ix] == '<') {
                    auto close = (buffer[ix] == '<') ? '>' : '\"';
                    while (ix < buffer.length() && buffer[ix] != close && buffer[ix] != '\n') {
                        ++ix;
                    }
                    state = State::NoState;
                    if (buffer[ix] == '\n' || ix >= buffer.length()) {
                        return std::tuple { Token::keyword(CCategory::DirectiveArg, CKeyword::Dir_include), ix - index };
                    }
                    return std::tuple { Token::keyword(CCategory::DirectiveArg, CKeyword::Dir_include), ix - index + 1 };
                }
            }
            case State::Ifdef: {
                while (ix < buffer.length() && (isalnum(buffer[ix]) || buffer[ix] == '_')) {
                    ++ix;
                }
                if (ix > index) {
                    state = State::NoState;
                    return std::tuple { Token::keyword(CCategory::DirectiveArg, CKeyword::Dir_ifdef), ix - index };
                }
            }
            case State::Define: {
                while (ix < buffer.length() && (isalnum(buffer[ix]) || buffer[ix] == '_')) {
                    ++ix;
                }
                if (ix > index) {
                    state = State::PreprocExpression;
                    return std::tuple { Token::keyword(CCategory::DirectiveArg, CKeyword::Dir_define), ix - index };
                }
            } break;
            case State::PreprocExpression: {
                while (ix < buffer.length() && !isspace(buffer[ix])) {
                    ++ix;
                }
                if (ix > index) {
                    if (buffer[ix] != '\n' || buffer[ix - 1] == '\\') {
                        state = State::NoState;
                    }
                    return std::tuple { Token::keyword(CCategory::DirectiveArg, CKeyword::Dir_define), ix - index };
                }
            } break;
            default:
                break;
            }
        }
        state = State::NoState;
        return {};
    }

    std::optional<std::tuple<CCategory, CKeyword>> match(std::string const &str)
    {
        if (auto m = match_keyword<CCategory, CKeyword>(str); m && std::get<MatchType>(*m) == MatchType::FullMatch) {
            return std::tuple { std::get<CCategory>(*m), std::get<CKeyword>(*m) };
        }
        return {};
    }

    std::optional<std::tuple<CCategory, CKeyword, size_t>> match(Buffer const &buffer, size_t index)
    {
        std::string scanned;
        switch (state) {
        case State::NoState: {
            if (buffer[index] == '#') {
                scanned += '#';
                auto ix = index + 1;
                while (ix < buffer.length() && (buffer[ix] == ' ' || buffer[ix] == '\t')) {
                    ++ix;
                }
                while (isalpha(buffer[ix])) {
                    scanned += buffer[ix];
                    ++ix;
                }
                if (auto m = match_keyword<CCategory, CKeyword>(scanned); m && std::get<MatchType>(*m) == MatchType::FullMatch) {
                    switch (std::get<CKeyword>(*m)) {
                    case CKeyword::Dir_define:
                        state = State::Define;
                        break;
                    case CKeyword::Dir_include:
                        state = State::Include;
                        break;
                    case CKeyword::Dir_ifdef:
                    case CKeyword::Dir_ifndef:
                    case CKeyword::Dir_elifdef:
                    case CKeyword::Dir_elifndef:
                        state = State::Ifdef;
                        break;
                    case CKeyword::Dir_if:
                    case CKeyword::Dir_elif:
                        state = State::PreprocExpression;
                        break;
                    default:
                        break;
                    }
                    return std::tuple { std::get<CCategory>(*m), std::get<CKeyword>(*m), ix - index };
                }
                return {};
            }
        } break;
        default:
            return {};
        }
        return {};
    }

    [[nodiscard]] std::string_view get_scope(Token const &token) const
    {
        switch (token.kind) {
        case TokenKind::Comment:
            return "comment";
        case TokenKind::Keyword: {
            switch (token.keyword().category) {
            case CCategory::CKeyword:
            case CCategory::CPPKeyword:
                return "keyword";
            case CCategory::Directive:
                return "meta.preprocessor";
            case CCategory::DirectiveArg:
                return "meta.preprocessor.string";
            default:
                UNREACHABLE();
            }
        } break;
        case TokenKind::Identifier:
            return "identifier";
        case TokenKind::Number:
            return "constant.numeric";
        case TokenKind::Symbol:
            return "punctuation";
        case TokenKind::QuotedString:
            return "string";
        default:
            return "identifier";
        }
        UNREACHABLE();
    }
};

struct CLexer : ModeLexer<CMatcher<BufferSource>> {
    using Keywords = CKeyword;
    using Categories = CCategory;
    std::shared_ptr<::LSP::LSP> lsp();

private:
    static std::shared_ptr<::LSP::LSP> the_lsp;
};

}

namespace LibCore {

using namespace Aragorn;

#define _S(X, Y) X## #Y

template<>
inline std::optional<std::tuple<CCategory, CKeyword, MatchType>> match_keyword(std::string const &str)
{
#undef S
#define S(KW)                                                            \
    if (std::string_view(#KW).starts_with(str)) {                        \
        return std::tuple {                                              \
            CCategory::CKeyword,                                         \
            CKeyword::C_##KW,                                            \
            (str == #KW) ? MatchType::FullMatch : MatchType::PrefixMatch \
        };                                                               \
    }
    C_KEYWORDS(S)
#undef S
#define S(KW)                                                            \
    if (std::string_view(#KW).starts_with(str)) {                        \
        return std::tuple {                                              \
            CCategory::CPPKeyword,                                       \
            CKeyword::CPP_##KW,                                          \
            (str == #KW) ? MatchType::FullMatch : MatchType::PrefixMatch \
        };                                                               \
    }
    CPP_KEYWORDS(S)
#undef S
#define S(D, STR)                                                        \
    if (std::string_view(STR).starts_with(str)) {                        \
        return std::tuple {                                              \
            CCategory::Directive,                                        \
            CKeyword::Dir_##D,                                           \
            (str == STR) ? MatchType::FullMatch : MatchType::PrefixMatch \
        };                                                               \
    }
    C_DIRECTIVES(S)
#undef S
    return {};
}

}
