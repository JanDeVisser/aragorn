/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <App/Buffer.h>
#include <App/Editor.h>
#include <App/StatusBar.h>

namespace Eddy {

struct FileName : public Label {
    FileName()
        : Label("", RAYWHITE)
    {
        policy_size = 40;
    }

    void resize() override
    {
        background = RAYWHITE; //colour_to_color(eddy.theme.selection.bg);
        color = DARKGRAY; //colour_to_color(eddy.theme.selection.fg);
    }

    void draw() override
    {
        if (!parent->memo) {
            parent->memo = std::dynamic_pointer_cast<Layout>(parent->parent)->find_by_class<Editor>();
        }
        pEditor editor = std::dynamic_pointer_cast<Editor>(parent->memo);
        assert(editor != nullptr);
        auto const& view = editor->current_view();
        auto const& buffer = view->buffer();
        if (buffer->name.empty()) {
            text = std::format("untitled-{}{}", view->buffer_num, buffer->saved_version < buffer->version ? '*' : ' ');
        } else {
            text = std::format("%{}%{}", buffer->name, buffer->saved_version < buffer->version ? '*' : ' ');
        }
        Label::draw();
    }
};

struct Cursor : public Label {
    Cursor()
        : Label("", DARKGRAY)
    {
        policy_size = 21;
    }

    void resize() override
    {
        background = RAYWHITE; //colour_to_color(eddy.theme.selection.bg);
        color = DARKGRAY; //colour_to_color(eddy.theme.selection.fg);
    }

    void draw() override
    {
        if (!parent->memo) {
            parent->memo = std::dynamic_pointer_cast<Layout>(parent->parent)->find_by_class<Editor>();
        }
        pEditor editor = std::dynamic_pointer_cast<Editor>(parent->memo);
        assert(editor != nullptr);
        auto const& view = editor->current_view();
        auto const& buffer = view->buffer();
        float       where = ((float) view->cursor_pos.line / (float) buffer->lines.size()) * 100.0f;
        text = std::format("ln {} ({}%) col {}", view->cursor_pos.line + 1, (int) roundf(where), view->cursor_pos.column + 1);
        Label::draw();
    }
};

struct FPS : public Label {
    FPS()
        : Label("", GREEN)
    {
        policy_size = 4;
    }

    void draw() override
    {
        int fps = GetFPS();
        text = std::format("{:}", fps);
        if (fps > 55) {
            color = GREEN;
        } else if (fps > 35) {
            color = ORANGE;
        } else {
            color = RED;
        }
        Label::draw();
    }
};

StatusBar::StatusBar()
    : Layout()
{
    orientation = ContainerOrientation::Horizontal;
    policy = SizePolicy::Characters;
    policy_size = 1.0f;
    add_widget<Spacer>(SizePolicy::Characters, 1);
    add_widget<FileName>();
    add_widget<Spacer>();
    add_widget<Cursor>();
    add_widget<FPS>();
}

void StatusBar::on_draw()
{
    draw_rectangle(0, 0, 0, 0, RAYWHITE /*colour_to_color(eddy.theme.selection.bg)*/);
}

}
