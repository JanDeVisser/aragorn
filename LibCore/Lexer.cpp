/*
 * Copyright (c) 2023, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <LibCore/Lexer.h>

namespace LibCore {

static int   isbdigit(int ch);

int isbdigit(int ch)
{
    return ch == '0' || ch == '1';
}

Source::Source(Language *language, std::string_view const &src, std::string_view const &name)
    : m_buffer(src)
    , m_language(language)
    , quote_chars("\"'`")
{
    m_location.file = name;
}


Token const& Source::peek_next()
{
    if (m_current.has_value()) {
        return m_current.value();
    }

    auto peek = [this]() -> Token {
        auto scan_number = [this]() -> Token
        {
            NumberType type = NumberType::Integer;
            int        ix = 0;
            int (*predicate)(int) = isdigit;
            if (m_buffer[1] && m_buffer[0] == '0') {
                if (m_buffer[1] == 'x' || m_buffer[1] == 'X') {
                    if (!m_buffer[2] || !isxdigit(m_buffer[2])) {
                        return Token::number(NumberType::Integer, m_buffer.substr(0, 1));
                    }
                    type = NumberType::HexNumber;
                    predicate = isxdigit;
                    ix = 2;
                } else if (m_buffer[1] == 'b' || m_buffer[1] == 'B') {
                    if (!m_buffer[2] || !isbdigit(m_buffer[2])) {
                        return Token::number(NumberType::Integer, m_buffer.substr(0, 1));
                    }
                    type = NumberType::BinaryNumber;
                    predicate = isbdigit;
                    ix = 2;
                }
            }

            while (true) {
                if (ix >= m_buffer.length()) {
                    return Token::number(type, m_buffer);
                }
                char ch = m_buffer[ix];
                if (!predicate(ch) && ((ch != '.') || (type == NumberType::Decimal))) {
                    // FIXME lex '1..10' as '1', '..', '10'. It will now lex as '1.', '.', '10'
                    return Token::number(type, m_buffer.substr(0, ix));
                }
                if (ch == '.') {
                    if (type != NumberType::Integer) {
                        return Token::number(type, m_buffer.substr(0, ix));
                    }
                    type = NumberType::Decimal;
                }
                ++ix;
            }
        };

        auto block_comment = [this](size_t ix) -> Token
        {
            for (; m_buffer[ix] && m_buffer[ix] != '\n' && (m_buffer[ix - 1] != '*' || m_buffer[ix] != '/'); ++ix)
                ;
            if (!m_buffer[ix]) {
                return Token::comment(CommentType::Block, m_buffer.substr(0, ix), false);
            }
            if (m_buffer[ix] == '\n') {
                m_in_comment = true;
                return Token::comment(CommentType::Block, m_buffer.substr(0, ix), false);
            }
            m_in_comment = false;
            return Token::comment(CommentType::Block, m_buffer.substr(0, ix + 1), true);
        };

        if (m_buffer.empty()) {
            return Token::end_of_file();
        }
        if (m_in_comment) {
            if (m_buffer[0] == '\n') {
                return Token::end_of_line(m_buffer.substr(0, 1));
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
            return Token::string(static_cast<QuoteType>(m_buffer[0]), m_buffer.substr(0, ix + 1), ix < m_buffer.length());
        }
        switch (m_buffer[0]) {
        case '/':
            switch (m_buffer[1]) {
            case '/': {
                size_t ix = 2;
                for (; m_buffer[ix] && m_buffer[ix] != '\n'; ++ix)
                    ;
                return Token::comment(CommentType::Line, m_buffer.substr(0, ix));
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
            return Token::end_of_line(m_buffer.substr(0, 1));
        }
        if (isspace(m_buffer[0])) {
            size_t ix = 0;
            for (; isspace(m_buffer[ix]) && m_buffer[ix] != '\n'; ++ix)
                ;
            return Token::whitespace(m_buffer.substr(0, ix));
        }
        if (isdigit(m_buffer[0])) {
            return scan_number();
        }
        if (isalpha(m_buffer[0]) || m_buffer[0] == '_') {
            size_t ix = 0;
            for (; isalnum(m_buffer[ix]) || m_buffer[ix] == '_'; ++ix)
                ;
            for (auto const &kw : m_language->keywords) {
                std::string_view keyword { kw.keyword };
                if (keyword.length() == ix && m_buffer.starts_with(keyword)) {
                    return Token::keyword(kw.code, m_buffer.substr(0, ix));
                }
            }
            return Token::identifier(m_buffer.substr(0, ix));
        }
        Keyword matched {};
        for (auto const &kw : m_language->keywords) {
            std::string_view keyword { kw.keyword };
            if (m_buffer.starts_with(keyword)) {
                if (keyword.length() > matched.keyword.length()) {
                    matched = kw;
                }
            }
        }
        if (!matched.keyword.empty()) {
            return Token::keyword(matched.code, m_buffer.substr(0, matched.keyword.length()));
        }
        Token ret = Token::symbol((int) m_buffer[0]);
        return ret;
    };
    m_current = peek();
    m_current.value().location = m_location;
    return m_current.value();
}

void Source::lex()
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

}
