/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <cmath>

#include <LibCore/JSON.h>

#include <App/App.h>

namespace Eddy {

char const *SizePolicy_name(SizePolicy policy)
{
    switch (policy) {
    case SizePolicy::Absolute:
        return "Absolute";
    case SizePolicy::Relative:
        return "Relative";
    case SizePolicy::Characters:
        return "Characters";
    case SizePolicy::Calculated:
        return "Calculated";
    case SizePolicy::Stretch:
        return "Stretch";
    default:
        UNREACHABLE();
    }
}

KeyboardModifiers modifier_current()
{
    auto current_modifier = static_cast<KeyboardModifiers>(KeyboardModifier::None);
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        current_modifier |= (KeyboardModifiers) KeyboardModifier::Shift;
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        current_modifier |= (KeyboardModifiers) KeyboardModifier::Control;
    }
    if (IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) {
        current_modifier |= (KeyboardModifiers) KeyboardModifier::Super;
    }
    if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) {
        current_modifier |= (KeyboardModifiers) KeyboardModifier::Alt;
    }
    return current_modifier;
}

bool is_modifier_down(KeyboardModifier modifier)
{
    KeyboardModifiers current_modifier = modifier_current();
    if (modifier == KeyboardModifier::None) {
        return current_modifier == (KeyboardModifiers) KeyboardModifier::None;
    }
    return ((KeyboardModifiers) current_modifier & (KeyboardModifiers) modifier) == (uint8_t) modifier;
}

std::string modifier_string(KeyboardModifiers modifier)
{
    std::string ret {};
#undef S
#define S(mod, ord, str)                                                                                       \
    if (((KeyboardModifiers) KeyboardModifier::mod & modifier) == (KeyboardModifiers) KeyboardModifier::mod) { \
        ret += (str);                                                                                          \
    }
    KEYBOARDMODIFIERS(S)
#undef S
    return ret;
}

Widget::Widget(SizePolicy policy, float policy_size)
    : policy(policy)
    , policy_size(policy_size)
{
}


void Widget::render_text(float x, float y, std::string_view const &text, Font font, Color color) const
{
    render_sized_text(x, y, text, font, 1.0, color);
}

void Widget::render_sized_text(float x, float y, std::string_view const &text, Font font, float size, Color color) const
{
    if (text.empty()) {
        return;
    }
    std::string t { text };
    if (x < 0 || y < 0) {
        Vector2 m = MeasureTextEx(font, t.c_str(), (float) font.baseSize * size, 2);
        if (x < 0) {
            x = viewport.width - m.x + x;
        }
        if (y < 0) {
            y = viewport.height - m.y + y;
        }
    }
    Vector2 pos = { viewport.x + x, viewport.y + y };
    DrawTextEx(font, t.c_str(), pos, (float) font.baseSize * size, 2, color);
}

void Widget::render_text_bitmap(float x, float y, std::string_view const &text, Color color) const
{
    if (text.empty()) {
        return;
    }
    std::string t { text };
    if (x < 0) {
        auto text_width = MeasureText(t.c_str(), 20);
        x = viewport.width - (float) text_width + x;
    }
    if (y < 0) {
        y = viewport.height - 20 + y;
    }
    DrawText(t.c_str(), (float) viewport.x + PADDING + x, viewport.y + PADDING + y, 20, color);
}

void Widget::draw_line(float x0, float y0, float x1, float y1, Color color) const
{
    if (x0 < 0) {
        x0 = viewport.width + x0;
    }
    if (y0 < 0) {
        y0 = viewport.height + y0;
    }
    if (x1 <= 0) {
        x1 = viewport.width + x1;
    }
    if (y1 <= 0) {
        y1 = viewport.height + y1;
    }
    x0 = clamp(x0, 0.0f, viewport.width);
    y0 = clamp(y0, 0.0f, viewport.height);
    x1 = clamp(x1, 0.0f, viewport.width - 1);
    y1 = clamp(y1, 0.0f, viewport.height - 1);
    DrawLine(viewport.x + x0, viewport.y + y0,
        viewport.x + x1, viewport.y + y1,
        color);
}

void Widget::draw_rectangle(float x, float y, float width, float height, Color color) const
{
    auto const r = normalize(x, y, width, height);
    DrawRectangleRec(r, color);
}

void Widget::draw_rectangle_no_normalize(float x, float y, float width, float height, Color color) const
{
    Rectangle const r = { .x = viewport.x + x, .y = viewport.y + y, .width = width, .height = height };
    DrawRectangleRec(r, color);
}

void Widget::draw_outline(float x, float y, float width, float height, Color color) const
{
    DrawRectangleLinesEx(normalize(x, y, width, height), 1, color);
}

void Widget::draw_outline_no_normalize(float x, float y, float width, float height, Color color) const
{
    Rectangle r = { .x = viewport.x + x, .y = viewport.y + y, .width = width, .height = height };
    DrawRectangleLinesEx(r, 1, color);
}

void Widget::draw_hover_panel(float x, float y, std::vector<std::string> text, Color bgcolor, Color textcolor) const
{
    assert(!text.empty());
    size_t longest_line = 0;
    size_t maxlen = 0;
    for (auto ix = 0; ix < text.size(); ++ix) {
        auto const &line = text.at(ix);
        if (line.length() > maxlen) {
            maxlen = line.length();
            longest_line = ix;
        }
    }
    auto text_size = MeasureTextEx(app->font, text.at(longest_line).c_str(), app->font.baseSize, 2);
    auto width = static_cast<float>(text_size.x + 12);
    auto height = static_cast<float>(app->cell.y + 2) * text.size() + 12;
    draw_rectangle_no_normalize(x, y, width, height, bgcolor);
    draw_outline_no_normalize(x + 2, y + 2, width - 4, height - 4, textcolor);
    for (size_t ix = 0; ix < text.size(); ++ix) {
        render_text(x + 6, y + (app->cell.y + 2) * ix + 6, text[ix], app->font, textcolor);
    }
}

bool Widget::contains(Vector2 world_coordinates) const
{
    return (viewport.x < world_coordinates.x) && (world_coordinates.x < viewport.x + viewport.width) && (viewport.y < world_coordinates.y) && (world_coordinates.y < viewport.y + viewport.height);
}

std::optional<Vec<int>> Widget::coordinates(Vector2 world_coordinates) const
{
    if (!contains(world_coordinates)) {
        return {};
    }
    auto ret = Vec<int> { .x = static_cast<int>(world_coordinates.x - viewport.x), .y = static_cast<int>(world_coordinates.y - viewport.y) };
    return ret;
}

bool Widget::find_and_run_shortcut(KeyboardModifiers modifier)
{
    for (pWidget w = self(); w; w = w->parent) {
        for (auto const &[name, cmd] : w->commands) {
            for (auto const &binding : cmd.bindings) {
                auto key = binding.key;
                if ((IsKeyPressed(key) || IsKeyPressedRepeat(key)) && binding.modifier == modifier) {
                    JSONValue key_combo = JSONValue::object();
                    set(key_combo, "key", key);
                    set(key_combo, "modifier", static_cast<int>(modifier));
                    w->submit(name, key_combo);
                    return true;
                }
            }
            if (w->delegate && w->delegate->find_and_run_shortcut(modifier)) {
                return true;
            }
        }
    }
    return false;
}

Spacer::Spacer()
    : Widget(SizePolicy::Stretch, 1.0f)
{
}

Spacer::Spacer(SizePolicy policy, float policy_size)
    : Widget(policy, policy_size)
{
}


Label::Label(std::string_view const &text, Color color)
    : Widget(SizePolicy::Characters, 1.0f)
    , text(text)
    , color(color)
{
}

void Label::draw()
{
    if (!text.empty()) {
        render_text(0, 0, text, app->font, color);
    }
}
}
