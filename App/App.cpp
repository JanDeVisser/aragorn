/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <raylib.h>

#include <App/App.h>
#include <App/Widget.h>
#include <LibCore/JSON.h>
#include <LibCore/Logging.h>
#include <LibCore/Options.h>

namespace Eddy {

using namespace LibCore;

std::shared_ptr<App> App::s_app { nullptr };

void App::draw_floating(pWidget const &target, App::Draw const &draw)
{
    assert(target != nullptr);
    assert(draw != nullptr);
    floatings.emplace_back(target, draw);
}

void App::draw()
{
    floatings.clear();
    Layout::draw();
    for (auto &floating : floatings) {
        floating.draw(floating.target);
    }
    for (auto &modal : modals) {
        modal->draw();
    }
}

void App::set_font(std::string_view const &path, int sz)
{
    sz = clamp(sz, 4, 48);
    std::string p { path };
    info(App, "Loading font '{:}', size {:}", p, sz);
    auto f = LoadFontEx(p.c_str(), sz, nullptr, 0);
    if (f.baseSize == 0) {
        return;
    }
    if (font) {
        UnloadFont(*font);
    }
    font = f;
    if (font_path != path) {
        font_path = std::string { path };
    }
    font_size = sz;
    resize();
}

void App::on_resize()
{
    assert(font.has_value());
    auto measurements = MeasureTextEx(*font, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", font_size, 2);
    cell.x = measurements.x / 52.0f;
    auto rows = static_cast<int>(((viewport.height - 10) / measurements.y));
    cell.y = static_cast<float>((viewport.height - 10) / static_cast<float>(rows));
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = static_cast<float>(GetScreenWidth());
    viewport.height = static_cast<float>(GetScreenHeight());
}

App::App()
    : Layout(nullptr)
{
}

void App::start()
{
    on_start();
    resize();
    while (!quit) {
        if (WindowShouldClose()) {
            quit = query_close();
        } else {
            process_input();
        }
        BeginDrawing();
        draw();
        EndDrawing();
    }
    on_terminate();
    CloseWindow();
}

void App::on_process_input()
{
    time = GetTime();
    ++frame_count;
    if (IsWindowResized()) {
        viewport.width = static_cast<float>(GetScreenWidth());
        viewport.height = static_cast<float>(GetScreenHeight());
        resize();
    }
    monitor = GetCurrentMonitor();
}

void App::handle_keyboard(pWidget const &focus)
{
}

void App::process_input()
{
    {
        auto lg = std::lock_guard(commands_mutex);
        if (!pending_commands.empty()) {
            auto cmd = pending_commands.front();
            pending_commands.pop_front();
            trace(CMD, "Executing {}({})", cmd.command.command, cmd.arguments.serialize());
            cmd.command.execute(cmd.arguments);
            return;
        }
    }

    auto handle_keyboard = [this](pWidget const &f) {
        KeyboardModifier modifier = modifier_current();
        for (int ch = GetCharPressed(); ch != 0; ch = GetCharPressed()) {
            for (auto w = f; w != nullptr; w = w->parent) {
                if (w->character(ch)) {
                    break;
                }
            }
        }
        std::erase_if(m_pressed_keys, [](auto const &key) {
	    return IsKeyUp(key);
        });
        std::vector<int> keys;
	std::for_each(m_pressed_keys.begin(), m_pressed_keys.end(), [&keys](auto const& key) {
	    if (IsKeyPressedRepeat(key)) {
                keys.emplace_back(key);
	    }
	});
        for (int key = GetKeyPressed(); key != 0; key = GetKeyPressed()) {
            keys.emplace_back(key);
	    m_pressed_keys.insert(key);
        }
	for (auto const key : keys) {
            f->bubble_up([key, modifier](pWidget const &w) {
                for (auto const &[name, cmd] : w->commands) {
                    for (auto const &binding : cmd.bindings) {
                        if (binding.key == key && binding.modifier == modifier) {
                            JSONValue key_combo = JSONValue::object();
                            set(key_combo, "key", key);
                            set(key_combo, "modifier", modifier);
                            w->submit(name, key_combo);
                            return true;
                        }
                    }
                }
                return w->process_key(modifier, key);
            });
        }
    };

    if (!modals.empty())
    {
        pWidget modal = modals.back();
        handle_keyboard(modal);
        modal->process_input();
        return;
    }

    pWidget f = focus;
    if (!f) {
        f = self();
    }
    handle_keyboard(f);
    Layout::process_input();
}

void App::push_modal(pWidget const &modal)
{
    modals.push_back(modal);
}

void App::pop_modal()
{
    if (!modals.empty()) {
        modals.pop_back();
    }
}

void App::change_font_size(int increment)
{
    set_font(font_path, font_size + increment);
}
}
