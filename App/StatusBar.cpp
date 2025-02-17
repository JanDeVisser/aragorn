/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <cmath>

#include <App/Buffer.h>
#include <App/Editor.h>
#include <App/StatusBar.h>

namespace Aragorn {

struct FileName : public Label {
    explicit FileName(pWidget const& parent)
        : Label(parent, "", RAYWHITE)
    {
        policy_size = 40;
    }

    void resize() override
    {
        background = RAYWHITE; //colour_to_color(aragorn.theme.selection.bg);
        color = DARKGRAY; //colour_to_color(aragorn.theme.selection.fg);
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
            text = std::format("untitled-{}{}", view->buffer()->buffer_ix, buffer->saved_version < buffer->version ? '*' : ' ');
        } else {
            text = std::format("%{}%{}", buffer->name, buffer->saved_version < buffer->version ? '*' : ' ');
        }
        Label::draw();
    }
};

struct Cursor : public Label {
    explicit Cursor(pWidget const& parent)
        : Label(parent, "", DARKGRAY)
    {
        policy_size = 21;
    }

    void resize() override
    {
        background = RAYWHITE; //colour_to_color(aragorn.theme.selection.bg);
        color = DARKGRAY; //colour_to_color(aragorn.theme.selection.fg);
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
        auto pos = view->cursor_position();
        float       where = ((float) pos.line / (float) buffer->lines.size()) * 100.0f;
        text = std::format("ln {} ({}%) col {}", pos.line + 1, (int) roundf(where), pos.column + 1);
        Label::draw();
    }
};

struct FPS : public Label {
    explicit FPS(pWidget const& parent)
        : Label(parent, "", GREEN)
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

StatusBar::StatusBar(pWidget const& parent)
    : Layout(parent, ContainerOrientation::Horizontal)
{
    policy = SizePolicy::Characters;
    policy_size = 1.0f;
}

void StatusBar::initialize()
{
    add_widget<Spacer>(SizePolicy::Characters, 1);
    add_widget<FileName>();
    add_widget<Spacer>();
    add_widget<Cursor>();
    add_widget<FPS>();
}

void StatusBar::on_draw()
{
    draw_rectangle(0, 0, 0, 0, RAYWHITE /*colour_to_color(aragorn.theme.selection.bg)*/);
}

}
