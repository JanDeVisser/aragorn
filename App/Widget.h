/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <algorithm>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

#include <raylib.h>

#include <LibCore/JSON.h>

namespace Eddy {

using namespace LibCore;

#define PADDING 5.0f

#define KEYBOARDMODIFIERS(S) \
    S(None, 0, "")           \
    S(Shift, 1, "S-")        \
    S(Control, 2, "C-")      \
    S(Alt, 4, "M-")          \
    S(Super, 8, "U-")

using KeyboardModifier = uint8_t;

#undef KEYBOARDMODIFIER
#define KEYBOARDMODIFIER(mod, ord, str) constexpr KeyboardModifier KMod##mod = (ord);
    KEYBOARDMODIFIERS(KEYBOARDMODIFIER)
#undef KEYBOARDMODIFIER
constexpr KeyboardModifier KModCount = 16;

enum class ContainerOrientation {
    Horizontal = 0,
    Vertical,
};

enum class SizePolicy {
    Absolute = 0,
    Relative,
    Characters,
    Calculated,
    Stretch,
};

template<typename T1, typename T2>
    requires std::convertible_to<T2, T1>
constexpr T1 min(T1 i1, T2 i2)
{
    return (i2 < i1) ? i2 : i1;
}

template<typename T1, typename T2>
    requires std::convertible_to<T2, T1>
constexpr T1 max(T1 i1, T2 i2)
{
    return (i2 > i1) ? i2 : i1;
}

template<typename T, typename Min, typename Max>
    requires std::convertible_to<Min, T> && std::convertible_to<Max, T>
constexpr T clamp(T v, Min low, Max high)
{
    return min(max(v, low), high);
}

template<typename T, typename Min, typename Max>
    requires std::convertible_to<Min, T> && std::convertible_to<Max, T>
constexpr bool contains(T v, Min low, Max high)
{
    return v >= low && v <= high;
}

template<typename T>
union Vec {
    struct {
        T x;
        T y;
    };
    struct {
        T line;
        T column;
    };
    T coords[2];
};

template<typename T>
union Rect {
    constexpr Rect(T c1, T c2, T c3, T c4)
        : left(c1)
        , top(c2)
        , right(c3)
        , bottom(c4)
    {
    }
    explicit constexpr Rect(T c)
        : Rect(c, c, c, c)
    {
    }

    constexpr Rect() = default;
    constexpr Rect(Rect const &) = default;
    struct {
        T x { 0 };
        T y { 0 };
        T width { 0 };
        T height { 0 };
    };
    struct {
        T left;
        T top;
        T right;
        T bottom;
    };
    //  Rectangle r;
    T coords[4];
    struct {
        Vec<T> position;
        Vec<T> size;
    };

    constexpr std::string to_string()
    {
        return std::format("{:.1}x{:.1}@+{:.1},+{:.1}", width, height, x, y);
    }

    constexpr static Rect<T> zero()
    {
        return Rect<T>();
    }
};

struct App;

struct KeyCombo {
    int              key;
    KeyboardModifier modifier;
};

constexpr auto ZeroPadding = Rect<float> { 0.0 };
constexpr auto DefaultPadding = Rect<float> { 5.0 };

using pWidget = std::shared_ptr<struct Widget>;

class Widget : public std::enable_shared_from_this<Widget> {
private:
    struct Private {
    };

public:
    using Handler = std::function<void(pWidget const &, JSONValue const &)>;
    struct WidgetCommand {
        WidgetCommand(std::string name, pWidget owner, Handler handler)
            : owner(std::move(owner))
            , command(std::move(name))
            , handler(std::move(handler))
        {
        }
        WidgetCommand(WidgetCommand const &) = default;
        WidgetCommand(WidgetCommand &&) = default;

        pWidget               owner;
        std::string           command;
        Handler               handler;
        std::vector<KeyCombo> bindings {};

        WidgetCommand &bind(KeyCombo combo)
        {
            bindings.push_back(combo);
            return *this;
        }

        template<typename... Args>
        WidgetCommand &bind(KeyCombo combo, Args... args)
        {
            bindings.push_back(combo);
            return bind(std::forward<Args>(args)...);
        }

        void execute(JSONValue const &args) const
        {
            handler(owner, args);
        }
    };

    struct PendingCommand {
        WidgetCommand const &command;
        JSONValue            arguments;

        PendingCommand(WidgetCommand const &command, JSONValue arguments)
            : command(command)
            , arguments(std::move(arguments))
        {
        }
    };

    Rect<float>                          viewport { 0.0 };
    Rect<float>                          padding { ZeroPadding };
    Color                                background { BLACK };
    SizePolicy                           policy { SizePolicy::Absolute };
    float                                policy_size { 0 };
    pWidget                              parent { nullptr };
    pWidget                              delegate { nullptr };
    pWidget                              memo { nullptr };
    std::map<std::string, WidgetCommand> commands;
    std::deque<PendingCommand>           pending_commands;
    std::mutex                           commands_mutex {};

    virtual void draw() { }
    virtual void resize() { }
    virtual bool character(int) { return false; }
    virtual void process_input() { }

    void render_text(float x, float y, std::string_view const &text, Font font, Color color) const;
    void render_sized_text(float x, float y, std::string_view const &text, Font font, float size, Color color) const;
    void render_text_bitmap(float x, float y, std::string_view const &text, Color color) const;
    void draw_rectangle(float x, float y, float width, float height, Color color) const;
    void draw_outline(float x, float y, float width, float height, Color color) const;
    void draw_line(float x0, float y0, float x1, float y1, Color color) const;
    void draw_hover_panel(float x, float y, std::vector<std::string> text, Color bgcolor, Color textcolor) const;

    template<class C, typename... Args>
    requires std::derived_from<C, Widget>
    static std::shared_ptr<C> make(Args &&...args)
    {
        return std::make_shared<C>(std::forward<Args>(args)...);
    }

    template<class C = Widget>
    requires std::derived_from<C, Widget>
    std::shared_ptr<C> self()
    {
        return std::dynamic_pointer_cast<C>(shared_from_this());
    }

    WidgetCommand &bind(std::string_view const &command, KeyCombo combo)
    {
        auto command_name = std::string { command };
        assert(commands.contains(command_name));
        auto &cmd = commands.at(command_name);
        cmd.bind(combo);
        return cmd;
    }

    template<typename... Args>
    WidgetCommand &bind(std::string_view const &command, KeyCombo combo, Args... args)
    {
        auto &cmd = bind(command, combo);
        cmd.bind(std::forward<Args>(args)...);
        return cmd;
    }

    template<typename C>
    WidgetCommand &add_command(std::string_view const &command, std::function<void(std::shared_ptr<C> const &, JSONValue const &)> handler)
    {
        auto wrapper = [handler](pWidget const &target, JSONValue const &args) -> void {
            auto const &t = std::dynamic_pointer_cast<C>(target);
            assert(t != nullptr);
            handler(t, args);
        };

        auto command_name = std::string { command };
        commands.try_emplace(command_name, command_name, self(), wrapper);
        return commands.at(command_name);
    }

    template<typename Left, typename Top, typename Width, typename Height>
        requires std::assignable_from<float &, Left> && std::assignable_from<float &, Top> && std::assignable_from<float &, Width> && std::assignable_from<float &, Height>
    Rectangle normalize(Left left, Top top, Width width, Height height) const
    {
        auto l = static_cast<float>(left);
        auto t = static_cast<float>(top);
        auto w = static_cast<float>(width);
        auto h = static_cast<float>(height);
        if (l < 0) {
            l = viewport.width + l;
        }
        if (t < 0) {
            t = viewport.height + t;
        }
        if (w <= 0) {
            w = viewport.width + 2 * w;
        }
        if (h <= 0) {
            h = viewport.height + 2 * h;
        }
        l = clamp(l, 0, viewport.width);
        t = clamp(t, 0, viewport.height);
        w = clamp(w, 0, viewport.width - 1);
        h = clamp(h, 0, viewport.height - 1);
        return (Rectangle) { .x = viewport.x + l, .y = viewport.y + t, .width = w, .height = h };
    }

    void submit(std::string_view const &command, JSONValue const &args)
    {
        auto lg = std::lock_guard(commands_mutex);
        auto name = std::string { command };
        if (!commands.contains(name)) {
            return;
        }
        auto const &cmd = commands.at(name);
        pending_commands.emplace_back(cmd, args);
    }

    virtual bool execute()
    {
        auto lg = std::lock_guard(commands_mutex);
        if (pending_commands.empty()) {
            return false;
        }
        auto cmd = pending_commands.front();
        pending_commands.pop_front();
        cmd.command.execute(cmd.arguments);
        return true;
    }

    [[nodiscard]] bool                    contains(Vector2 world_coordinates) const;
    [[nodiscard]] std::optional<Vec<int>> coordinates(Vector2 world_coordinates) const;
    bool                                  find_and_run_shortcut(KeyboardModifier modifier);

    void draw_rectangle_no_normalize(float x, float y, float width, float height, Color color) const;
    void draw_outline_no_normalize(float x, float y, float width, float height, Color color) const;

    Widget(Widget const &) = delete;
    Widget(Widget &&) = delete;
    Widget() = default;
    Widget(SizePolicy policy, float policy_size);
};

struct Layout : public Widget {
    ContainerOrientation orientation { ContainerOrientation::Vertical };
    std::vector<pWidget> widgets {};

    Layout() = default;

    void draw() override;
    void resize() override;
    void process_input() override;

    virtual void on_resize() { }
    virtual void after_resize() { }
    virtual void on_draw() { }
    virtual void after_draw() { }
    virtual void on_process_input() { }
    virtual void after_process_input() { }

    template<class Cls, typename... Args>
        requires std::derived_from<Cls, Widget>
    std::shared_ptr<Cls> add_widget(Args &&...args)
    {
        auto widget = Widget::make<Cls>(std::forward<Args>(args)...);
        widget->parent = self();
        widgets.push_back(widget);
        return std::dynamic_pointer_cast<Cls>(widget);
    }

    template<typename Predicate>
    pWidget find_by_predicate(Predicate p)
    {
        auto s = self();
        if (p(s)) {
            return s;
        }
        for (auto &w : widgets) {
            if (auto layout = std::dynamic_pointer_cast<Layout>(w); layout) {
                if (auto ret = layout->find_by_predicate(p); ret) {
                    return ret;
                }
            } else {
                if (p(w)) {
                    return w;
                }
            }
        }
        return nullptr;
    }

    template<typename C>
    std::shared_ptr<C> find_by_class()
    {
        auto p = [](auto &w) {
            return std::dynamic_pointer_cast<C>(w) != nullptr;
        };
        return std::dynamic_pointer_cast<C>(find_by_predicate(p));
    }

    template<typename Callback>
    void traverse(Callback fnc)
    {
        fnc(self());
        for (auto &w : widgets) {
            if (auto layout = dynamic_pointer_cast<Layout>(w); layout) {
                layout->traverse(fnc);
            } else {
                fnc(w);
            }
        }
        fnc(nullptr);
    }

    void dump();
};

struct Spacer : public Widget {
    Spacer();
    Spacer(SizePolicy policy, float policy_size);
};

struct Label : public Widget {
    Color       color;
    std::string text;

    Label(std::string_view const &text, Color color);
    void draw() override;
};

extern char const      *SizePolicy_name(SizePolicy policy);
extern bool             is_modifier_down(KeyboardModifier modifier);
extern std::string      modifier_string(KeyboardModifier modifiers);
extern KeyboardModifier modifier_current();

}
