/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <App/Eddy.h>
#include <App/Widget.h>

namespace Eddy {

struct MiniBuffer : public Widget {
    std::string message {};
    double time {0.0};
    pWidget current_query { nullptr };

    MiniBuffer()
        : Widget(SizePolicy::Characters, 1.0f)
        , message()
    {
    }

    void resize() override
    {
        if (current_query != nullptr) {
            current_query->viewport = viewport;
            current_query->padding = padding;
        }
    }

    void draw() override
    {
        if (current_query != nullptr) {
            return;
        }
        draw_rectangle(0, 0, viewport.width, viewport.height, DARKGRAY /*colour_to_color(eddy.theme.editor.bg)*/);
        if (!message.empty()) {
            render_text(0, 0, message, Eddy::the()->font, RAYWHITE /*colour_to_color(eddy.theme.editor.fg)*/);
        }
    }

    void process_input() override
    {
        if (!message.empty() && Eddy::the()->time - time > 2.0) {
            message.clear();
        }
    }

    template <typename ...Args>
    void display(char const *fmt, Args&& ...args)
    {
        message = std::vformat(fmt, std::make_format_args(args)...);
        time = Eddy::the()->time;
    }

    void display(char const *msg)
    {
        message = msg;
        time = Eddy::the()->time;
    }

    void clear()
    {
        message.clear();
    }

    template <typename ...Args>
    static void set_message(char const *fmt, Args&& ...args)
    {
        auto const& minibuffer = Eddy::the()->find_by_class<MiniBuffer>();
        assert(minibuffer != nullptr);
        minibuffer->display(fmt, std::format<Args>(args)...);
    }

    static void clear_message()
    {
        auto const& minibuffer = Eddy::the()->find_by_class<MiniBuffer>();
        assert(minibuffer != nullptr);
        minibuffer->clear();
    }

    template<class C, typename Query>
    static void query(std::shared_ptr<C> const& target, std::string_view const& prompt, Query fnc)
    {
        struct MiniBufferQuery : public Widget {
            std::string prompt {};
            std::shared_ptr<C> target { nullptr };
            std::string text {};
            size_t cursor {0};
            Query query { nullptr };
            std::shared_ptr<MiniBuffer> minibuffer { nullptr };

            MiniBufferQuery(Query query, std::string_view const& prompt, std::shared_ptr<C> target)
                : Widget(SizePolicy::Characters, 1.0f)
                , query(std::move(query))
                , prompt(prompt)
                , target(std::move(target))
            {
                assert(query != nullptr);
            }

            void process_input() override
            {
                if (IsKeyPressed(KEY_ESCAPE)) {
                    Eddy::the()->pop_modal();
                    minibuffer->current_query = nullptr;
                } else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                    Eddy::the()->pop_modal();
                    minibuffer->current_query = nullptr;
                    query(target, text);
                } else if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
                    if (cursor > 0) {
                        text.erase(cursor - 1, 1);
                        --cursor;
                    }
                } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
                    if (cursor > 0) {
                        --cursor;
                    }
                } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
                    if (!text.empty() && cursor < text.length() - 1) {
                        ++cursor;
                    }
                } else if (IsKeyPressed(KEY_HOME) || IsKeyPressedRepeat(KEY_HOME)) {
                    cursor = 0;
                } else if (IsKeyPressed(KEY_END) || IsKeyPressedRepeat(KEY_END)) {
                    cursor = text.length() - 1;
                }
            }

            bool character(int ch) override
            {
                text.insert(cursor, static_cast<char>(ch), 1);
                ++cursor;
                return true;
            }

            void resize() override
            {
            }

            void draw() override
            {
                render_text(0, 0, std::format("{}: {}", prompt, text), Eddy::the()->font, RAYWHITE /*colour_to_color(eddy.theme.editor.fg)*/);
                double t = GetTime();
                if ((t - floor(t)) < 0.5) {
                    auto x = static_cast<float>(prompt.length() + 2 + cursor);
                    draw_rectangle(x * Eddy::the()->cell.x, 0, 2, Eddy::the()->cell.y + 5, RAYWHITE /*colour_to_color(eddy.theme.editor.fg)*/);
                }
            }
        };

        auto const& minibuffer = Eddy::the()->find_by_class<MiniBuffer>();
        assert(minibuffer != nullptr);
        assert(fnc != NULL);
        if (minibuffer->current_query != nullptr) {
            minibuffer->display("Minibuffer already active");
            return;
        }
        minibuffer->clear();
        auto const &q  = Widget::make<MiniBufferQuery>(fnc, prompt, target);
        q->minibuffer = minibuffer;
        minibuffer->current_query = q;
        Eddy::the()->push_modal(minibuffer->current_query);
    }
};

}
