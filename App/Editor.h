/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <LibCore/JSON.h>

#include <App/Buffer.h>
#include <App/Widget.h>

namespace Eddy {

using pEditor = std::shared_ptr<class Editor>;
using pBufferView = std::shared_ptr<class BufferView>;

class BufferView : public Widget {
public:
    size_t      cursor { 0 };
    Vec<int>    cursor_pos {};
    int         cursor_col { 0 };
    size_t      new_cursor { 0 };
    int         top_line { 0 };
    int         left_column { 0 };
    size_t      selection { 0 };
    double      cursor_flash { 0.0 };
    std::string find_text {};
    std::string replacement {};
    pWidget     mode { nullptr };

    BufferView(pEditor editor, pBuffer buf);
    pBuffer const &buffer() const;
    void           draw() override;
    void           process_input() override;
    bool           character(int ch) override;
    void           manage_selection(bool sel);
    bool           find_next();
    void           insert(size_t at, std::string_view const &text);
    void           insert_string(std::string_view const &sv);
    void           del(size_t at, size_t count);
    void           lines_up(int count);
    void           lines_down(int count);
    void           selection_to_clipboard();
    void           select_line();
    void           word_left();
    void           word_right();
    void           select_word();
    void           backspace();
    void           delete_current_char();
    void           move(int line, int col);
    void           update_cursor();
    int            delete_selection();
    int            lines() const;
    int            columns() const;

private:
    pBuffer m_buf { nullptr };
    double  clicks[3] {0.0, 0.0, 0.0};
    int     num_clicks {0};
};

class Editor : public Widget {
public:
    Editor();
    void        draw() override;
    void        resize() override;
    void        process_input() override;
    Result<int> open(std::string_view const &file);
    void        new_buffer();
    void        select_buffer(pBuffer const &buffer);
    void        select_view(int view_ix);
    pBuffer     current_buffer();
    pBufferView current_view();
    void        close_view();
    void        close_buffer();

    int columns {0};
    int lines {0};

private:
    std::vector<pBufferView> views {};
    int                      current_view_ix {-1};
};

struct Gutter : public Widget {
    size_t  row_diagnostic_hover { 0 };
    size_t  first_diagnostic_hover { 0 };
    size_t  num_diagnostics_hover { 0 };
    pEditor editor;

    Gutter() = delete;
    explicit Gutter(pEditor editor);
    void draw() override;
    void resize() override;
    void process_input() override;

private:
    void draw_diagnostic_float();
};

}
