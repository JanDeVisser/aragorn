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

KeyboardModifier modifier_current()
{
    auto current_modifier = KModNone;
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        current_modifier = static_cast<KeyboardModifier>(current_modifier | KModShift);
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        current_modifier = static_cast<KeyboardModifier>(current_modifier | KModControl);
    }
    if (IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) {
        current_modifier = static_cast<KeyboardModifier>(current_modifier | KModSuper);
    }
    if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) {
        current_modifier = static_cast<KeyboardModifier>(current_modifier | KModAlt);
    }
    return current_modifier;
}

bool is_modifier_down(KeyboardModifier modifier)
{
    KeyboardModifier current_modifier = modifier_current();
    if (modifier == KModNone) {
        return current_modifier == KModNone;
    }
    return (current_modifier & modifier) == modifier;
}

std::string modifier_string(KeyboardModifier modifier)
{
    std::string ret {};
#undef S
#define S(mod, ord, str)                       \
    if ((KMod##mod & modifier) == KMod##mod) { \
        ret += (str);                          \
    }
    KEYBOARDMODIFIERS(S)
#undef S
    return ret;
}

std::deque<Widget::PendingCommand> Widget::pending_commands {};
std::mutex                         Widget::commands_mutex {};

Widget::Widget(pWidget parent, SizePolicy policy, float policy_size)
    : policy(policy)
    , policy_size(policy_size)
    , parent(std::move(parent))
{
}

void Widget::render_sized_text_(float x, float y, std::string_view const &text, Font font, float size, Color color) const
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

void Widget::render_text_bitmap_(float x, float y, std::string_view const &text, Color color) const
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

void Widget::draw_line_(float x0, float y0, float x1, float y1, Color color) const
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

void Widget::draw_rectangle_(float x, float y, float width, float height, Color color) const
{
    auto const r = normalize(x, y, width, height);
    DrawRectangleRec(r, color);
}

void Widget::draw_rectangle_no_normalize_(float x, float y, float width, float height, Color color) const
{
    Rectangle const r = { .x = viewport.x + x, .y = viewport.y + y, .width = width, .height = height };
    DrawRectangleRec(r, color);
}

void Widget::draw_outline_(float x, float y, float width, float height, Color color) const
{
    DrawRectangleLinesEx(normalize(x, y, width, height), 1, color);
}

void Widget::draw_outline_no_normalize_(float x, float y, float width, float height, Color color) const
{
    Rectangle r = { .x = viewport.x + x, .y = viewport.y + y, .width = width, .height = height };
    DrawRectangleLinesEx(r, 1, color);
}

void Widget::draw_hover_panel_(float x, float y, StringList const& text, Color bgcolor, Color textcolor) const
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
    auto text_size = MeasureTextEx(App::the()->font.value(), text.at(longest_line).c_str(), App::the()->font.value().baseSize, 2);
    auto width = static_cast<float>(text_size.x + 12);
    auto height = static_cast<float>(App::the()->cell.y + 2) * text.size() + 12;
    draw_rectangle_no_normalize(x, y, width, height, bgcolor);
    draw_outline_no_normalize(x + 2, y + 2, width - 4, height - 4, textcolor);
    for (size_t ix = 0; ix < text.size(); ++ix) {
        render_text(x + 6, y + (App::the()->cell.y + 2) * ix + 6, text[ix], App::the()->font.value(), textcolor);
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

Spacer::Spacer(pWidget const &parent)
    : Widget(parent, SizePolicy::Stretch, 1.0f)
{
}

Spacer::Spacer(pWidget const &parent, SizePolicy policy, float policy_size)
    : Widget(parent, policy, policy_size)
{
}

Label::Label(pWidget const &parent, std::string_view const &text, Color color)
    : Widget(parent, SizePolicy::Characters, 1.0f)
    , text(text)
    , color(color)
{
}

void Label::draw()
{
    if (!text.empty()) {
        render_text(0, 0, text, App::the()->font.value(), color);
    }
}

}
