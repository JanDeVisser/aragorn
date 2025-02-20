/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <App/Buffer.h>
#include <App/Widget.h>

namespace Aragorn {

using pEditor = std::shared_ptr<class Editor>;
using pBufferView = std::shared_ptr<class BufferView>;

class BufferView : public Widget {
public:
    struct CursorMovement {
        std::optional<size_t>      index;
        std::optional<Vec<size_t>> pos;
        bool                       extend_selection { false };

        static CursorMovement by_index(size_t index, bool extend_selection = false)
        {
            CursorMovement ret;
            ret.index = index;
            ret.extend_selection = extend_selection;
            return ret;
        }

        static CursorMovement by_position(size_t line, size_t column, bool extend_selection = false)
        {
            CursorMovement ret;
            ret.pos = { .line = line, .column = column };
            ret.extend_selection = extend_selection;
            return ret;
        }
    };

    BufferView(pWidget const &editor, pBuffer buf);
    pBuffer const             &buffer() const;
    void                       initialize() override;
    void                       draw() override;
    void                       process_input() override;
    bool                       character(int ch) override;
    void                       unselected();
    void                       selected();
    void                       insert(size_t at, std::string_view const &text);
    void                       insert_string(std::string_view const &sv);
    void                       del(size_t at, size_t count);
    void                       lines_up(size_t count, bool select);
    void                       lines_down(size_t count, bool select);
    void                       selection_to_clipboard();
    void                       select_line();
    void                       word_left();
    void                       word_right();
    void                       select_word();
    void                       backspace();
    void                       delete_current_char();
    void                       move(size_t line, size_t col, bool select);
    void                       delete_selection();
    size_t                     lines() const;
    size_t                     columns() const;
    void                       copy();
    void                       cut();
    void                       paste();
    void                       move_up(bool select);
    void                       move_down(bool select);
    void                       move_left(bool select);
    void                       move_right(bool select);
    void                       move_word_left(bool select);
    void                       move_word_right(bool select);
    void                       move_begin_of_line(bool select);
    void                       move_end_of_line(bool select);
    void                       move_top(bool select);
    void                       move_bottom(bool select);
    Vec<size_t>                cursor_position() const;
    Vec<size_t>                view_offset() const;
    size_t                     index() const;
    std::optional<Vec<size_t>> selection() const;
    bool                       has_selection() const;
    void                       clear_selection();
    void                       set_mark(size_t at);
    bool                       find_first(std::string_view const &pattern);
    bool                       find_next();
    void                       replacement(std::string_view const &replacement);
    void                       clear_replacement();
    void                       replace();
    void                       move_cursor(CursorMovement const &move);

    auto operator[](size_t ix) const
    {
        return m_buf->at(ix);
    }

private:
    size_t                version { 0 };
    size_t                cursor { 0 };
    size_t                cursor_line { 0 };
    size_t                cursor_col { 0 };
    size_t                top_line { 0 };
    size_t                left_column { 0 };
    std::optional<size_t> m_selection {};
    double                cursor_flash { 0.0 };
    std::string           m_find_text {};
    std::string           m_replacement {};
    pWidget               mode { nullptr };
    pBuffer               m_buf { nullptr };
    double                clicks[3] { 0.0, 0.0, 0.0 };
    int                   num_clicks { 0 };
};

}
