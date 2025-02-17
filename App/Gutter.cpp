/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <App/Aragorn.h>
#include <App/Editor.h>

namespace Aragorn {

Gutter::Gutter(pEditor editor)
    : Widget(editor->parent, SizePolicy::Characters, 5)
    , editor(std::move(editor))
{
    padding = Rect { PADDING };
    background = DARKGRAY; // colour_to_color(Aragorn::the()->theme.gutter.bg);
}

void Gutter::resize()
{
    background = DARKGRAY; // colour_to_color(Aragorn::the()->theme.gutter.bg);
}

void Gutter::draw_diagnostic_float()
{
#if 0
    auto  view = current_view();
    auto  buffer = current_buffer();

    StringList hover_text = { 0 };
    for (size_t ix = first_diagnostic_hover; ix < num_diagnostics_hover; ++ix) {
        hover_text.push_back(buffer->diagnostics[first_diagnostic_hover + ix].message);
    }
    if (hover_text.size() > 0) {
        draw_hover_panel(viewport.width - 3, Aragorn::the()->cell.y * row_diagnostic_hover + 6, hover_text,
            DARKGRAY, RAYWHITE);
//            //colour_to_color(Aragorn::the()->theme.editor.bg), colour_to_color(Aragorn::the()->theme.editor.fg));
    }
#endif
}

void Gutter::draw()
{
    auto const &view = editor->current_view();
    auto        buffer = view->buffer();

    auto offset = view->view_offset();
    for (auto row = 0; row < editor->lines && offset.line + row < buffer->lines.size(); ++row) {
        auto lineno = offset.line + row;
        render_text(0, Aragorn::the()->cell.y * row,
            std::format("{:4}", lineno + 1),
            Aragorn::the()->font.value(),
            RAYWHITE /*colour_to_color(Aragorn::the()->theme.gutter.fg)*/);
#if 0
        auto &line = buffer->lines[lineno];
        if (line->num_diagnostics > 0) {
            widget_draw_rectangle(gutter, -6, Aragorn::the()->cell.y * row, 6, Aragorn::the()->cell.y, RED);
        }
#endif
    }
#if 0
    if (num_diagnostics_hover > 0) {
        app_draw_floating(app, gutter, (WidgetDraw) draw_diagnostic_float);
    }
#endif
}

void Gutter::process_input()
{
    row_diagnostic_hover = 0;
    first_diagnostic_hover = 0;
    num_diagnostics_hover = 0;
    Vector2 mouse = GetMousePosition();
    if (contains(mouse)) {
        Vec<int>    gutter_coords = coordinates(mouse).value();
        int         row = gutter_coords.y / Aragorn::the()->cell.y;
        auto const &view = editor->current_view();
        auto const &buffer = view->buffer();
        auto        lineno = view->view_offset().line + row;
        auto       &line = buffer->lines[lineno];
#if 0
        if (line.num_diagnostics > 0) {
            row_diagnostic_hover = row;
            first_diagnostic_hover = line.first_diagnostic;
            num_diagnostics_hover = line.num_diagnostics;
        }
#endif
    }
}

}
