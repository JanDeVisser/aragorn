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
    : Layout()
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

void App::handle_characters(pWidget const &focus)
{
    while (!queue.empty()) {
        int ch = queue.front();
        for (auto w = focus; w != nullptr; w = w->parent) {
            if (w->character(ch)) {
                break;
            }
        }
        queue.pop_front();
    }
}

void App::process_input()
{
    auto execute_pending_command = [](pWidget const &w) -> bool {
        return w->execute();
    };
    if (find_by_predicate(execute_pending_command) != nullptr) {
        return;
    }
    for (int ch = GetCharPressed(); ch != 0; ch = GetCharPressed()) {
        queue.push_back(ch);
    }
    if (!modals.empty()) {
        pWidget modal = modals.back();
        handle_characters(modal);
        modal->process_input();
        return;
    }
    pWidget f = focus;
    if (!f) {
        f = self();
    }
    KeyboardModifier modifier = modifier_current();
    if (!f->find_and_run_shortcut(modifier)) {
        handle_characters(f);
        Layout::process_input();
    }
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
