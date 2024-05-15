/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <App/Eddy.h>
#include <App/Editor.h>

namespace Eddy {

Gutter::Gutter(pEditor editor)
    : Widget(SizePolicy::Characters, 5)
    , editor(std::move(editor))
{
    parent = editor;
    padding = Rect { PADDING };
    background = DARKGRAY; // colour_to_color(Eddy::the()->theme.gutter.bg);
}

void Gutter::resize()
{
    background = DARKGRAY; // colour_to_color(Eddy::the()->theme.gutter.bg);
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
        draw_hover_panel(viewport.width - 3, Eddy::the()->cell.y * row_diagnostic_hover + 6, hover_text,
            DARKGRAY, RAYWHITE);
//            //colour_to_color(Eddy::the()->theme.editor.bg), colour_to_color(Eddy::the()->theme.editor.fg));
    }
#endif
}

void Gutter::draw()
{
    auto const &view = editor->current_view();
    auto        buffer = view->buffer();

    for (auto row = 0; row < editor->lines && view->top_line + row < buffer->lines.size(); ++row) {
        auto lineno = view->top_line + row;
        render_text(0, Eddy::the()->cell.y * row,
            std::format("{:4}", lineno + 1),
            Eddy::the()->font,
            RAYWHITE /*colour_to_color(Eddy::the()->theme.gutter.fg)*/);
        auto &line = buffer->lines[lineno];
#if 0
        if (line->num_diagnostics > 0) {
            widget_draw_rectangle(gutter, -6, Eddy::the()->cell.y * row, 6, Eddy::the()->cell.y, RED);
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
        int         row = gutter_coords.y / Eddy::the()->cell.y;
        auto const &view = editor->current_view();
        auto const &buffer = view->buffer();
        auto        lineno = view->top_line + row;
        auto       &line = buffer->lines[lineno];
        if (line.num_diagnostics > 0) {
            row_diagnostic_hover = row;
            first_diagnostic_hover = line.first_diagnostic;
            num_diagnostics_hover = line.num_diagnostics;
        }
    }
}

}
