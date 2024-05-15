/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cmath>

#include <App/Eddy.h>
#include <App/Widget.h>

namespace Eddy {

enum class ModalStatus {
    Dormant = 0,
    Active,
    Submitted,
    Dismissed,
};

enum QueryOption : uint8_t {
    QueryOptionYes = 0x01,
    QueryOptionNo = 0x02,
    QueryOptionYesNo = 0x03,
    QueryOptionCancel = 0x04,
    QueryOptionYesNoCancel = 0x07,
    QueryOptionOK = 0x08,
    QueryOptionOKCancel = 0x0C,
};

struct Modal : public Widget {
    float       textsize { 1.0 };
    std::string prompt {};
    ModalStatus status { ModalStatus::Dormant };

    explicit Modal(std::string_view const &prompt)
        : Widget()
        , prompt(prompt)
    {
        background = DARKGRAY; // colour_to_color(Eddy::the()->theme.editor.bg);
    }

    virtual void dismiss()
    {
    }

    virtual void submit()
    {
    }

    void make_visible()
    {
        Eddy::the()->push_modal(self());
        status = ModalStatus::Active;
    }

    void hide()
    {
        Eddy::the()->pop_modal();
        status = ModalStatus::Dormant;
    }
};

template<typename Payload, bool Search = true, bool Sort = true, bool Shrink = false, typename ToString = std::nullptr_t>
struct ListBox : public Modal {
    using pListBox = std::shared_ptr<ListBox>;
    struct ListBoxEntry {
        std::string text;
        Payload     payload;
        size_t      index { 0 };

        ListBoxEntry(std::string_view const &text, Payload const &payload)
            : text(text)
            , payload(payload)
        {
        }

        auto operator<=>(ListBoxEntry const &other) const = default;
    };

    using ListBoxEntries = std::vector<ListBoxEntry>;
    ListBoxEntries      entries {};
    std::vector<size_t> matches {};
    std::string         search {};
    int                 lines { 0 };
    int                 top_line { 0 };
    int                 selection { -1 };
    ModalStatus         status { ModalStatus::Dormant };
    ToString            to_string_fnc { nullptr };

    explicit ListBox(std::string_view const &prompt, ToString to_string = nullptr)
        : Modal(prompt)
        , to_string_fnc(std::move(to_string))
    {
        sort();
        filter();
        resize();
    }

    void draw_entries(size_t y_offset)
    {
        size_t maxlen = (viewport.width - 28) / (Eddy::the()->cell.x * textsize);
        auto   draw_entry = [this, &y_offset, maxlen](ListBoxEntry const &e) -> void {
            auto text_color = RAYWHITE; // colour_to_color(Eddy::the()->theme.editor.fg);
            if (e.index == selection) {
                draw_rectangle(8, y_offset - 1, -8, Eddy::the()->cell.y * textsize + 1, DARKGRAY /*colour_to_color(Eddy::the()->theme.selection.bg)*/);
                text_color = DARKGRAY; // colour_to_color(Eddy::the()->theme.selection.fg);
            }
            std::string_view sv;
            if constexpr (!std::is_same<ToString, std::nullptr_t>::value) {
                assert(to_string_fnc != nullptr);
                sv = to_string_fnc(e.text, e.payload);
            } else if constexpr (std::is_convertible<Payload, std::string>()) {
                sv = std::string { e.payload };
            } else {
                sv = e.text;
            }
            if (sv.length() > maxlen) {
                sv = sv.substr(0, maxlen);
            }
            render_sized_text(10, y_offset, sv, Eddy::the()->font, textsize, text_color);
            y_offset += (Eddy::the()->cell.y * textsize) + 2;
        };

        if constexpr (Search) {
            if (search.empty()) {
                for (auto const &e : entries) {
                    draw_entry(e);
                }
            } else {
                for (auto ix : matches) {
                    draw_entry(entries[ix]);
                }
            }
        } else {
            for (auto const &e : entries) {
                draw_entry(e);
            }
        }
    }

    void draw() override
    {
        auto bg = DARKGRAY; // colour_to_color(Eddy::the()->theme.editor.bg);
        auto fg = RAYWHITE; // colour_to_color(Eddy::the()->theme.editor.fg);
        draw_rectangle(0.0f, 0.0f, 0.0f, 0.0, bg);
        draw_outline(2, 2, -2.0f, -2.0f, fg);
        render_text(8, 8, prompt, Eddy::the()->font, fg);
        render_text(-8, 8, search, Eddy::the()->font, fg);
        draw_line(2, Eddy::the()->cell.y + 10, -2, Eddy::the()->cell.y + 10, fg);
        draw_entries(Eddy::the()->cell.y + 14);
    }

    void resize() override
    {
        viewport.x = Eddy::the()->viewport.width / 4;
        viewport.y = Eddy::the()->viewport.height / 4;
        viewport.width = Eddy::the()->viewport.width / 2;
        viewport.height = Eddy::the()->viewport.height / 2;
        lines = (viewport.height - 19 + Eddy::the()->cell.y) / (Eddy::the()->cell.y + 2);
        if constexpr (Shrink && !Search) {
            lines = clamp(lines, 0, entries.size() < lines);
        }
        Vector2 m = MeasureTextEx(Eddy::the()->font, prompt.c_str(), Eddy::the()->font.baseSize, 2);
        if (m.x > viewport.width - 16) {
            viewport.width = m.x + 16;
            viewport.x = (Eddy::the()->viewport.width - m.x) / 2;
        }
        viewport.height = 19 + Eddy::the()->cell.y + lines * (Eddy::the()->cell.y + 2);
    }

    bool character(int ch) override
    {
        if constexpr (Search) {
            auto c = static_cast<char>(ch);
            search.append(std::string_view { &c, 1 });
            filter();
            return true;
        }
        return false;
    }

    void process_input() override
    {
        if (status == ModalStatus::Dormant) {
            return;
        }
        size_t sz = entries.size();
        if constexpr (Search) {
            if (!search.empty()) {
                sz = matches.size();
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            status = ModalStatus::Dismissed;
            dismiss();
        } else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            if (selection < sz) {
                status = ModalStatus::Submitted;
                submit();
            }
        } else if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
            if (selection > 0) {
                --selection;
                while (selection < top_line) {
                    --top_line;
                }
            }
        } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
            if (selection < sz - 1) {
                ++selection;
                while (selection > top_line + lines - 1) {
                    ++top_line;
                }
            }
        } else if (IsKeyPressed(KEY_PAGE_UP)) {
            if (selection >= lines) {
                selection -= lines;
                if (top_line > lines) {
                    top_line -= lines;
                }
            }
        } else if (IsKeyPressed(KEY_PAGE_DOWN)) {
            if (selection < sz - lines - 1) {
                selection += lines;
                top_line += lines;
            }
        }
        if constexpr (Search) {
            if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && !search.empty()) {
                search.erase(search.length() - 1, 1);
                filter();
            }
        }
        if (status != ModalStatus::Active) {
            Eddy::the()->pop_modal();
        }
    }

    void sort()
    {
        if constexpr (Sort) {
            std::sort(entries.begin(), entries.end());
        }
        for (size_t ix = 0; ix < entries.size(); ++ix) {
            entries[ix].index = ix;
        }
    }

    void filter()
    {
        matches.clear();
        if constexpr (Search) {
            if (search.empty()) {
                return;
            }
            for (auto const &e : entries) {
                if (strcasestr(e.text.c_str(), search.c_str()) != nullptr) {
                    matches.push_back(e.index);
                }
            }
            selection = 0;
        }
    }

    void refresh()
    {
        sort();
        filter();
        resize();
        selection = 0;
        top_line = 0;
        search = {};
        status = ModalStatus::Active;
    }

    void show()
    {
        refresh();
        make_visible();
    }
};

template <typename C, typename Submit, typename Dismiss = std::nullptr_t>
void input_box(std::shared_ptr<C> target, std::string_view const& prompt, Submit fnc, std::string_view const& def = "", Dismiss dismiss = nullptr)
{
    using pC = std::shared_ptr<C>;
    struct InputBox : public Modal {
        pC target;
        std::string text;
        size_t      cursor {0};
        Submit      submit_fnc;
        Dismiss     dismiss_fnc;

        InputBox(pC target, std::string_view const &prompt, Submit submit, Dismiss dismiss = nullptr)
            : Modal(prompt)
            , target(target)
            , submit_fnc(submit)
            , dismiss_fnc(dismiss)
        {
            assert(submit_fnc != nullptr);
            background = DARKGRAY; // colour_to_color(Eddy::the()->theme.editor.bg);
            resize();
        }

        void submit() override
        {
            submit_fnc(target, text);
        }

        void dismiss() override
        {
            if constexpr (!std::is_same<Dismiss, std::nullptr_t>::value) {
                dismiss(target);
            }
        }

        bool character(int ch) override
        {
            if (cursor < text.length()) {
                text.insert(cursor, 1, static_cast<char>(ch));
            } else {
                text.append(1, static_cast<char>(ch));
            }
            return true;
        }

        void process_input() override
        {
            if (status == ModalStatus::Dormant) {
                return;
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                status = ModalStatus::Dismissed;
                dismiss();
            } else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                status = ModalStatus::Submitted;
                submit();
            } else if (IsKeyPressed(KEY_LEFT) && cursor > 0) {
                --cursor;
            } else if (IsKeyPressed(KEY_RIGHT) && cursor < text.length()) {
                ++cursor;
            } else if (IsKeyPressed(KEY_BACKSPACE) && cursor > 0) {
                text.erase(cursor - 1, 1);
                --cursor;
            }
            if (status != ModalStatus::Active) {
                hide();
            }
        }

        void show()
        {
            make_visible();
        }

        void resize() override
        {
            viewport.x = Eddy::the()->viewport.width / 4;
            viewport.y = Eddy::the()->viewport.height / 4;
            viewport.width = Eddy::the()->viewport.width / 2;
            viewport.height = 21 + 2 * Eddy::the()->cell.y;
        }

        void draw() override
        {
            draw_rectangle(0.0, 0.0, 0.0, 0.0, DARKGRAY /* colour_to_color(Eddy::the()->theme.editor.bg) */);
            draw_outline(2, 2, -2.0, -2.0, RAYWHITE /* colour_to_color(Eddy::the()->theme.editor.fg) */);
            render_text(8, 8, prompt, Eddy::the()->font, RAYWHITE /* colour_to_color(Eddy::the()->theme.editor.fg) */);
            draw_line(2, Eddy::the()->cell.y + 10, -2, Eddy::the()->cell.y + 10, RAYWHITE /* colour_to_color(Eddy::the()->theme.editor.fg) */);
            render_text(10, Eddy::the()->cell.y + 14, text, Eddy::the()->font, RAYWHITE /* colour_to_color(Eddy::the()->theme.editor.fg) */);
            double t = GetTime();
            if ((t - floor(t)) < 0.5) {
                draw_rectangle(10 + cursor * Eddy::the()->cell.x, Eddy::the()->cell.y + 12, 2, Eddy::the()->cell.y + 2, RAYWHITE /* colour_to_color(Eddy::the()->theme.editor.fg) */);
            }
        }
    };
    auto const& inputbox = Widget::make<InputBox>(target, prompt, fnc, dismiss);
    inputbox->text = def;
    inputbox->show();
}

template <typename C, typename Submit>
void query_box(std::shared_ptr<C> target, std::string_view const& prompt, Submit fnc, QueryOption options)
{
    using pC = std::shared_ptr<C>;
    struct QueryBox : public ListBox<QueryOption,false,false,true> {
        pC target;
        Submit submit_fnc;

        explicit QueryBox(pC const& target, std::string_view prompt, Submit submit, QueryOption options = QueryOptionOK)
            : ListBox(prompt)
            , target(target)
            , submit_fnc(std::move(submit))
        {
            if (options & QueryOptionYes) {
                entries.emplace_back("Yes", QueryOptionYes);
            }
            if (options & QueryOptionNo) {
                entries.emplace_back("No", QueryOptionNo);
            }
            if (options & QueryOptionCancel) {
                entries.emplace_back("Cancel", QueryOptionCancel);
            }
            if (options & QueryOptionOK) {
                entries.emplace_back("OK", QueryOptionOK);
            }
        }

        void submit() override
        {
            auto selected = entries[selection].payload;
            submit_fnc(target, selected);
        }
    };
    auto const& qbox = Widget::make<QueryBox>(target, prompt, fnc, options);
    qbox->show();
}

inline static void message_box(std::string_view const& prompt)
{
    auto dummy = [](auto const&, auto) {
    };
    pWidget p { nullptr };
    query_box<Widget>(p, prompt, dummy, QueryOptionOK);
}

}
