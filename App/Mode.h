/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <LibCore/Token.h>

#include <App/Event.h>
#include <App/Theme.h>
#include <App/Widget.h>

namespace Aragorn {

using namespace LibCore;

class DisplayToken {
public:
    DisplayToken(size_t index, size_t length, size_t line, size_t column, TokenKind kind, Scope scope)
        : m_index(index)
        , m_length(length)
        , m_line(line)
        , m_column(column)
        , m_kind(kind)
        , m_scope(scope)
    {
    }

    DisplayToken(DisplayToken const &) = default;

    explicit                operator size_t() const { return m_index; }
    explicit                operator Colours() const { return Theme::the().get_colours(m_scope); }
    [[nodiscard]] size_t    index() const { return m_index; }
    [[nodiscard]] size_t    length() const { return m_length; }
    [[nodiscard]] size_t    line() const { return m_line; }
    [[nodiscard]] size_t    column() const { return m_column; }
    [[nodiscard]] TokenKind kind() const { return m_kind; }

private:
    size_t    m_index;
    size_t    m_length;
    size_t    m_line;
    size_t    m_column;
    TokenKind m_kind;
    Scope     m_scope;
};

class Mode : public Widget {
public:
    explicit Mode(pWidget const &parent)
        : Widget(parent)
    {
    }

    virtual void                initialize_source() = 0;
    virtual DisplayToken        lex() = 0;
    virtual BufferEventListener event_listener() const { return nullptr; }

private:
    //
};

using pMode = std::shared_ptr<Mode>;

}
