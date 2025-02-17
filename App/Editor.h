/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <LibCore/JSON.h>

#include <App/BufferView.h>

namespace Aragorn {

class Editor : public Widget {
public:
    explicit Editor(pWidget const &parent);
    void        initialize() override;
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

    size_t columns { 0 };
    size_t lines { 0 };

private:
    std::vector<pBufferView> views {};
    int                      current_view_ix { -1 };
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
