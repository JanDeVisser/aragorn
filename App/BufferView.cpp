/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <App/Aragorn.h>
#include <App/BufferView.h>
#include <App/Editor.h>
#include <App/MiniBuffer.h>
#include <App/Modal.h>

namespace Aragorn {

constexpr char const *OPEN_BRACES = "({[<";
constexpr char const *CLOSE_BRACES = ")}]>";

int get_closing_brace_code(int brace);

bool do_select(JSONValue const &key_combo)
{
    if (key_combo.type() == JSONType::Boolean) {
        bool result;
        MUST(key_combo.convert<bool>(result));
        return result;
    }
    assert(key_combo.type() == JSONType::Object);
    auto modifier = KModNone;
    if (auto modifier_maybe = key_combo.try_get<int>("modifier"); modifier_maybe.has_value()) {
        modifier = (KeyboardModifier) modifier_maybe.value();
    }
    return (modifier & KModShift) == KModShift;
}

void cmd_up(pBufferView const &view, JSONValue const &key_combo)
{
    view->move_up(do_select(key_combo));
}

void cmd_select_word(pBufferView const &view, JSONValue const &)
{
    view->select_word();
}

void cmd_down(pBufferView const &view, JSONValue const &key_combo)
{
    view->move_down(do_select(key_combo));
}

void cmd_left(pBufferView const &view, JSONValue const &key_combo)
{
    view->move_left(do_select(key_combo));
}

void cmd_word_left(pBufferView const &view, JSONValue const &key_combo)
{
    view->move_word_left(do_select(key_combo));
}

void cmd_right(pBufferView const &view, JSONValue const &key_combo)
{
    view->move_right(do_select(key_combo));
}

void cmd_word_right(pBufferView const &view, JSONValue const &key_combo)
{
    view->move_word_right(do_select(key_combo));
}

void cmd_begin_of_line(pBufferView const &view, JSONValue const &key_combo)
{
    view->move_begin_of_line(do_select(key_combo));
}

void cmd_top_of_buffer(pBufferView const &view, JSONValue const &key_combo)
{
    view->move_top(do_select(key_combo));
}

void cmd_bottom_of_buffer(pBufferView const &view, JSONValue const &key_combo)
{
    view->move_bottom(do_select(key_combo));
}

void cmd_end_of_line(pBufferView const &view, JSONValue const &key_combo)
{
    view->move_begin_of_line(do_select(key_combo));
}

void cmd_page_up(pBufferView const &view, JSONValue const &key_combo)
{
    view->lines_up(view->lines(), do_select(key_combo));
}

void cmd_page_down(pBufferView const &view, JSONValue const &key_combo)
{
    view->lines_down(view->lines(), do_select(key_combo));
}

void cmd_split_line(pBufferView const &view, JSONValue const &)
{
    view->character('\n');
}

void cmd_insert_tab(pBufferView const &view, JSONValue const &)
{
    view->character('\t');
}

void cmd_merge_lines(pBufferView const &view, JSONValue const &)
{
    auto const &buffer = view->buffer();
    auto        pos = view->cursor_position();
    Line const &line = buffer->lines[pos.y];
    buffer->merge_lines(pos.y);
    view->move_cursor(BufferView::CursorMovement::by_index(line.end(), true));
}

void find_closing_brace(pBufferView const &view, size_t index, bool selection)
{
    auto const &buffer = view->buffer();
    auto        brace = (*buffer)[index];
    auto        matching = get_closing_brace_code(brace);
    assert(matching > 0);
    if (selection) {
        view->set_mark(index);
    }
    auto depth = 1;
    while (++index < buffer->length()) {
        if ((*buffer)[index] == matching) {
            --depth;
        }
        if (!depth) {
            view->move_cursor(BufferView::CursorMovement::by_index(++index, true));
            return;
        }
        if ((*buffer)[index] == brace) {
            ++depth;
        }
    }
}

void find_opening_brace(pBufferView const &view, size_t index, bool selection)
{
    auto brace = (*view)[index];
    char matching = 0;
    for (size_t ix = 0; ix < 3; ++ix) {
        if (CLOSE_BRACES[ix] == brace) {
            matching = OPEN_BRACES[ix];
            break;
        }
    }
    if (selection) {
        view->set_mark(index);
    }
    assert(matching);
    int depth = 1;
    while (--index != -1) {
        if ((*view)[index] == matching) {
            --depth;
        }
        if (!depth) {
            view->move_cursor(BufferView::CursorMovement::by_index(index, selection));
            return;
        }
        if ((*view)[index] == brace) {
            ++depth;
        }
    }
}

void cmd_matching_brace(pBufferView const &view, JSONValue const &key_combo)
{
    bool selection = do_select(key_combo);
    auto index = view->index();
    if (strchr(OPEN_BRACES, (*view)[index])) {
        find_closing_brace(view, index, selection);
        return;
    }
    if ((index > 0) && strchr(OPEN_BRACES, (*view)[index - 1])) {
        find_closing_brace(view, index - 1, selection);
        return;
    }
    if (strchr(CLOSE_BRACES, (*view)[index])) {
        find_opening_brace(view, index, selection);
        return;
    }
    if ((index > 0) && strchr(CLOSE_BRACES, (*view)[index - 1])) {
        find_opening_brace(view, index - 1, selection);
        return;
    }
}

void cmd_backspace(pBufferView const &view, JSONValue const &)
{
    view->backspace();
}

void cmd_delete_current_char(pBufferView const &view, JSONValue const &)
{
    view->delete_current_char();
}

void cmd_clear_selection(pBufferView const &view, JSONValue const &)
{
    view->clear_selection();
}

void cmd_copy(pBufferView const &view, JSONValue const &)
{
    view->copy();
}

void cmd_cut(pBufferView const &view, JSONValue const &)
{
    view->cut();
}

void cmd_paste(pBufferView const &view, JSONValue const &)
{
    view->paste();
}

void cmd_undo(pBufferView const &view, JSONValue const &)
{
    view->buffer()->undo();
}

void cmd_redo(pBufferView const &view, JSONValue const &)
{
    view->buffer()->redo();
}

void do_find(pBufferView const &view, rune_string const &query)
{
    view->find_first(query);
}

void cmd_find(pBufferView const &view, JSONValue const &)
{
    MiniBuffer::query(view, L"Find", do_find);
}

void cmd_find_next(pBufferView const &view, JSONValue const &)
{
    view->find_next();
}

void do_ask_replace(pBufferView const &view, rune_string const &reply)
{
    int cmd = 0;
    if (!reply.empty()) {
        cmd = toupper(reply[0]);
    }
    switch (cmd) {
    case 'Y': {
        view->replace();
    } break;
    case 'N':
        break;
    case 'A': {
        do {
            view->replace();
        } while (view->find_next());
        return;
    }
    case 'Q':
        return;
    default:
        MiniBuffer::query(view, L"Replace ((Y)es/(N)o/(A)ll/(Q)uit)", do_ask_replace);
        return;
    }
    if (view->find_next()) {
        MiniBuffer::query(view, L"Replace ((Y)es/(N)o/(A)ll/(Q)uit)", do_ask_replace);
        return;
    }
    Aragorn::set_message("Not found");
}

void do_replacement_query(pBufferView const &view, rune_string const &replacement)
{
    view->replacement(replacement);
    MiniBuffer::query(view, L"Replace ((Y)es/(N)o/(A)ll/(Q)uit)", do_ask_replace);
}

void do_find_query(pBufferView const &view, rune_string const &query)
{
    if (!view->find_first(query)) {
        Aragorn::set_message("Not found");
        return;
    }
    MiniBuffer::query(view, L"Replace with", do_replacement_query);
}

void cmd_find_replace(pBufferView const &view, JSONValue const &)
{
    MiniBuffer::query(view, L"Find", do_find_query);
}

void do_goto(pBufferView const &view, rune_string const &query)
{
    auto coords = split(MUST_EVAL(to_utf8(query)), ':');
    int  line = -1;
    int  col = -1;
    if (!coords.empty()) {
        auto line_maybe = string_to_integer<int>(coords[0]);
        if (line_maybe.has_value()) {
            line = line_maybe.value();
            if (coords.size() > 1) {
                auto col_maybe = string_to_integer<int>(coords[1]);
                if (col_maybe.has_value()) {
                    col = col_maybe.value();
                }
            }
        }
    }
    if (line >= 1) {
        BufferView::CursorMovement movement = BufferView::CursorMovement::by_position(
            line, view->cursor_position().column, true);
        if (col >= 1) {
            movement.pos->column = col - 1;
        }
        view->move_cursor(movement);
    }
}

void cmd_goto(pBufferView const &view, JSONValue const &)
{
    MiniBuffer::query(view, L"Move to", do_goto);
}

void save_as_submit(pBufferView const &view, std::string const &filename)
{
    auto const &buffer = view->buffer();
    buffer->save_as(filename);
    Aragorn::set_message("Buffer saved");
}

void cmd_save_as(pBufferView const &view, JSONValue const &)
{
    auto const &buffer = view->buffer();
    input_box(view, "New file name", save_as_submit, buffer->name);
}

void cmd_save(pBufferView const &view, JSONValue const &dummy)
{
    auto const &buffer = view->buffer();
    if (buffer->name.empty()) {
        cmd_save_as(view, dummy);
        return;
    }
    buffer->save();
    Aragorn::set_message("Buffer saved");
}

BufferView::BufferView(pWidget const &editor, pBuffer buf)
    : Widget(editor)
    , m_buf(std::move(buf))
{
    viewport = editor->viewport;
    padding = editor->padding;
    if (m_buf->mode()) {
        delegate = m_buf->mode();
    }
}

void BufferView::initialize()
{
    add_command<BufferView>("cursor-up", cmd_up)
        .bind(KeyCombo { KEY_UP, KModNone })
        .bind(KeyCombo { KEY_UP, KModShift });
    add_command<BufferView>("select-word", cmd_select_word)
        .bind(KeyCombo { KEY_UP, KModAlt });
    add_command<BufferView>("cursor-down", cmd_down)
        .bind(KeyCombo { KEY_DOWN, KModNone })
        .bind(KeyCombo { KEY_DOWN, KModShift });
    add_command<BufferView>("cursor-left", cmd_left)
        .bind(KeyCombo { KEY_LEFT, KModNone })
        .bind(KeyCombo { KEY_LEFT, KModShift });
    add_command<BufferView>("cursor-word-left", cmd_word_left)
        .bind(KeyCombo { KEY_LEFT, KModAlt })
        .bind(KeyCombo { KEY_LEFT, KModAlt | KModShift });
    add_command<BufferView>("cursor-right", cmd_right)
        .bind(KeyCombo { KEY_RIGHT, KModNone })
        .bind(KeyCombo { KEY_RIGHT, KModShift });
    add_command<BufferView>("cursor-word-right", cmd_word_right)
        .bind(KeyCombo { KEY_RIGHT, KModAlt })
        .bind(KeyCombo { KEY_LEFT, KModAlt | KModShift });
    add_command<BufferView>("cursor-page-up", cmd_page_up)
        .bind(KeyCombo { KEY_PAGE_UP, KModNone })
        .bind(KeyCombo { KEY_PAGE_UP, KModShift })
        .bind(KeyCombo { KEY_UP, KModSuper })
        .bind(KeyCombo { KEY_UP, KModSuper | KModShift });
    add_command<BufferView>("cursor-page-down", cmd_page_down)
        .bind(KeyCombo { KEY_PAGE_DOWN, KModNone })
        .bind(KeyCombo { KEY_PAGE_DOWN, KModShift })
        .bind(KeyCombo { KEY_DOWN, KModSuper })
        .bind(KeyCombo { KEY_DOWN, KModSuper | KModShift });
    add_command<BufferView>("cursor-home", cmd_begin_of_line)
        .bind(KeyCombo { KEY_HOME, KModNone })
        .bind(KeyCombo { KEY_HOME, KModShift })
        .bind(KeyCombo { KEY_LEFT, KModSuper })
        .bind(KeyCombo { KEY_LEFT, KModSuper | KModShift });
    add_command<BufferView>("cursor-top", cmd_top_of_buffer)
        .bind(KeyCombo { KEY_HOME, KModControl })
        .bind(KeyCombo { KEY_HOME, KModSuper | KModShift });
    add_command<BufferView>("cursor-end", cmd_end_of_line)
        .bind(KeyCombo { KEY_END, KModNone })
        .bind(KeyCombo { KEY_END, KModShift })
        .bind(KeyCombo { KEY_RIGHT, KModSuper })
        .bind(KeyCombo { KEY_RIGHT, KModSuper | KModShift });
    add_command<BufferView>("cursor-top", cmd_top_of_buffer)
        .bind(KeyCombo { KEY_HOME, KModControl })
        .bind(KeyCombo { KEY_HOME, KModControl | KModShift });
    add_command<BufferView>("cursor-bottom", cmd_bottom_of_buffer)
        .bind(KeyCombo { KEY_END, KModControl })
        .bind(KeyCombo { KEY_END, KModControl | KModShift });
    add_command<BufferView>("split-line", cmd_split_line)
        .bind(KeyCombo { KEY_ENTER, KModNone })
        .bind(KeyCombo { KEY_KP_ENTER, KModNone });
    add_command<BufferView>("insert-tab", cmd_insert_tab)
        .bind(KeyCombo { KEY_TAB, KModNone });
    add_command<BufferView>("merge-lines", cmd_merge_lines)
        .bind(KeyCombo { KEY_J, KModShift | KModControl });
    add_command<BufferView>("matching-brace", cmd_matching_brace)
        .bind(KeyCombo { KEY_M, KModControl })
        .bind(KeyCombo { KEY_M, KModControl | KModShift });
    add_command<BufferView>("backspace", cmd_backspace)
        .bind(KeyCombo { KEY_BACKSPACE, KModNone });
    add_command<BufferView>("delete-current-char", cmd_delete_current_char)
        .bind(KeyCombo { KEY_DELETE, KModNone });
    add_command<BufferView>("clear-selection", cmd_clear_selection)
        .bind(KeyCombo { KEY_ESCAPE, KModNone });
    add_command<BufferView>("copy-selection", cmd_copy)
        .bind(KeyCombo { KEY_C, KModSuper });
    add_command<BufferView>("cut-selection", cmd_cut)
        .bind(KeyCombo { KEY_X, KModSuper });
    add_command<BufferView>("paste-from-clipboard", cmd_paste)
        .bind(KeyCombo { KEY_V, KModSuper });
    add_command<BufferView>("editor-undo", cmd_undo)
        .bind(KeyCombo { KEY_Z, KModSuper });
    add_command<BufferView>("editor-redo", cmd_redo)
        .bind(KeyCombo { KEY_Z, KModSuper | KModShift });
    add_command<BufferView>("editor-find", cmd_find)
        .bind(KeyCombo { KEY_F, KModSuper });
    add_command<BufferView>("editor-find-next", cmd_find_next)
        .bind(KeyCombo { KEY_G, KModSuper });
    add_command<BufferView>("editor-goto", cmd_goto)
        .bind(KeyCombo { KEY_L, KModSuper });
    add_command<BufferView>("editor-find-replace", cmd_find_replace)
        .bind(KeyCombo { KEY_R, KModSuper });
    add_command<BufferView>("editor-save", cmd_save)
        .bind(KeyCombo { KEY_S, KModControl });
    add_command<BufferView>("editor-save-as", cmd_save_as)
        .bind(KeyCombo { KEY_S, KModControl | KModAlt });
}

void BufferView::unselected()
{
}

void BufferView::selected()
{
    cursor_flash = Aragorn::the()->time;
    Aragorn::the()->focus = self();
    if (mode) {
        Aragorn::the()->focus = mode;
    }
}

pBuffer const &BufferView::buffer() const
{
    return m_buf;
}

size_t BufferView::lines() const
{
    pEditor editor = std::dynamic_pointer_cast<Editor>(parent);
    return editor->lines;
}

size_t BufferView::columns() const
{
    pEditor editor = std::dynamic_pointer_cast<Editor>(parent);
    return static_cast<size_t>(editor->columns);
}

void BufferView::lines_up(size_t count, bool select)
{
    move_cursor(CursorMovement::by_position(
        cursor_line - std::min(count, cursor_line),
        cursor_col,
        select));
}

void BufferView::lines_down(size_t count, bool select)
{
    move_cursor(CursorMovement::by_position(
        cursor_line + std::min(count, m_buf->lines.size() - std::min(count, m_buf->lines.size())),
        cursor_col,
        select));
}

void BufferView::move(size_t line, size_t col, bool select)
{
    move_cursor(CursorMovement::by_position(line, col, select));
}

void BufferView::select_line()
{
    size_t lineno = m_buf->line_for_index(cursor);
    Line  &line = m_buf->lines[lineno];
    set_mark(line.begin());
    move_cursor(CursorMovement::by_index(line.end() + 1, true));
}

void BufferView::word_left()
{
    auto new_cursor = cursor;
    while (0 < new_cursor && !isalnum((*m_buf)[new_cursor])) {
        --new_cursor;
    }
    while (0 < new_cursor && isalnum((*m_buf)[new_cursor])) {
        --new_cursor;
    }
    if (new_cursor > 0 || !isalnum((*m_buf)[0])) {
        ++new_cursor;
    }
    move_cursor(CursorMovement::by_index(new_cursor, true));
}

void BufferView::word_right()
{
    auto new_cursor = cursor;
    while (new_cursor < m_buf->length() - 1 && !isalnum((*m_buf)[new_cursor])) {
        ++new_cursor;
    }
    while (new_cursor < m_buf->length() - 1 && isalnum((*m_buf)[new_cursor])) {
        ++new_cursor;
    }
    move_cursor(CursorMovement::by_index(new_cursor, true));
}

void BufferView::select_word()
{
    set_mark(m_buf->word_boundary_left(cursor));
    move_cursor(CursorMovement::by_index(m_buf->word_boundary_right(cursor), true));
}

void BufferView::insert(size_t at, rune_view const &text)
{
    m_buf->insert(at, rune_string { text });
    move_cursor(CursorMovement::by_index(at + text.length()));
}

void BufferView::del(size_t at, size_t count)
{
    m_buf->del(at, count);
    move_cursor(CursorMovement::by_index(at));
}

void BufferView::delete_selection()
{
    assert(has_selection());
    auto sel = selection();
    del(sel->coords[0], sel->coords[1] - sel->coords[0]);
    move_cursor(CursorMovement::by_index(sel->coords[0]));
}

void BufferView::backspace()
{
    if (!has_selection()) {
        if (cursor != 0) {
            del(cursor - 1, 1);
        }
    } else {
        delete_selection();
    }
}

void BufferView::delete_current_char()
{
    if (!has_selection()) {
        if (cursor < m_buf->length()) {
            del(cursor, 1);
        }
    } else {
        delete_selection();
    }
}

int get_closing_brace_code(int brace)
{
    for (size_t ix = 0; ix < 3; ++ix) {
        if (OPEN_BRACES[ix] == brace) {
            return CLOSE_BRACES[ix];
        }
    }
    return -1;
}

bool BufferView::character(int ch)
{
    size_t at = cursor;
    if (auto sel = selection(); sel.has_value()) {
        switch (ch) {
        case '(':
        case '[':
        case '<':
        case '{': {
            int close = get_closing_brace_code(ch);
            insert(sel->coords[0], rune_string { (wchar_t const *) &ch, 1 });
            insert(sel->coords[1] + 1, rune_string { (wchar_t const *) &close, 1 });
            auto new_cursor = sel->coords[0] + 1;
            set_mark(sel->coords[1] + 1);
            if (cursor == sel->coords[1]) {
                set_mark(sel->coords[0] + 1);
                new_cursor = sel->coords[1] + 1;
            }
            move_cursor(CursorMovement::by_index(new_cursor));
            return true;
        }
        default:
            delete_selection();
            return true;
        }
    }
    insert(at, rune_string { (wchar_t const *) &ch, 1 });
    return true;
}

void BufferView::insert_string(rune_view const &sv)
{
    auto at = cursor;
    if (has_selection()) {
        delete_selection();
    }
    insert(at, sv);
    move_cursor(CursorMovement::by_index(at + sv.length()));
}

void BufferView::selection_to_clipboard()
{
    if (auto sel = selection(); sel.has_value()) {
        SetClipboardText(reinterpret_cast<char const *>(m_buf->substr(sel->coords[0], sel->coords[1] - sel->coords[0]).c_str()));
    }
}

void BufferView::draw()
{
    if (buffer()->version > version) {
        auto cursor_pos = m_buf->index_to_position(cursor, { { cursor_col, cursor_line } });
        cursor_line = cursor_pos.y;
        version = buffer()->version;
    }
    static size_t frame { 1 };
    draw_rectangle(0, 0, 0, 0, Theme::the().bg());

    auto const &ed = std::dynamic_pointer_cast<Editor>(parent);
    auto        y_offset = (ed->cell.y - Aragorn::the()->char_size.y) / 2;
    DrawText(TextFormat("cursor: %d", cursor), 700, 50, 20, RAYWHITE);
    DrawText(TextFormat("cursor line: %d", cursor_line), 700, 75, 20, RAYWHITE);
    DrawText(TextFormat("cursor col: %d", cursor_col), 700, 100, 20, RAYWHITE);

    auto cursor_drawn { false };
    for (int row = 0; row < lines() && top_line + row < m_buf->lines.size(); ++row) {
        auto        lineno = top_line + row;
        auto const &line = m_buf->lines[lineno];
        auto        line_len = min(line.length() - 1, left_column + columns());

        if (line.empty()) {
            continue;
        }
        if (has_selection()) {
            auto sel = selection();
            auto line_start = line.begin() + left_column;
            auto line_end = min(line.end(), line_start + columns());
            auto selection_offset = clamp(sel->coords[0] - min(sel->coords[0], line_start), 0, line_end);

            if (sel->coords[0] < line_end && sel->coords[1] > line.begin()) {
                auto width = sel->coords[1] - max(sel->coords[0], line_start);
                if (width > line_len - selection_offset) {
                    width = columns() - selection_offset;
                }
                draw_rectangle(
                    ed->cell.x * selection_offset,
                    ed->cell.y * row,
                    width * ed->cell.x,
                    ed->cell.y + 5.0f,
                    Theme::the().selection_bg());
            }
        }
        for (auto const &token : line.tokens) {
            auto start_col = token.column();
            // token ends before left edge
            if (start_col + token.length() <= left_column) {
                continue;
            }

            // token starts after right edge. We're done here; go to next line
            if (start_col >= left_column + columns()) {
                break;
            }

            // Cut off at left edge
            if (start_col < left_column) {
                start_col = left_column;
            }

            // length taking left edge into account
            auto length = token.length() - (start_col - token.column());
            // If start + length overflows right edge, clip length:
            if (start_col + length > left_column + columns()) {
                length = left_column + columns() - start_col;
            }

            auto start_ix = token.index() + (start_col - token.column());
            for (auto col_ix = 0; col_ix < length; ++col_ix) {
                auto const ch_ix = start_ix + col_ix;
                auto const col = start_col + col_ix;
                auto const screen_pos = Vector2 {
                    ed->cell.x * static_cast<float>(col - left_column),
                    ed->cell.y * static_cast<float>(row) + y_offset,
                };

                if (!cursor_drawn && (ch_ix >= cursor || (cursor_line == lineno && token.kind() == TokenKind::EndOfLine))) {
                    double time = Aragorn::the()->time - cursor_flash;
                    if (time - floor(time) < 0.5) {
                        draw_rectangle(
                            screen_pos.x,
                            screen_pos.y,
                            2,
                            ed->cell.y + 1,
                            Theme::the().fg());
                    }
                    cursor_drawn = true;
                }
                switch (token.kind()) {
                case TokenKind::EndOfFile:
                    break;
                case TokenKind::EndOfLine:
                    break;
                case TokenKind::Tab:
                    render_texture(screen_pos.x, screen_pos.y, Aragorn::the()->tab_char, static_cast<Colours>(token).fg());
                    break;
                default:
                    auto const ch = m_buf->at(ch_ix);
                    render_codepoint(
                        screen_pos.x, screen_pos.y,
                        ch,
                        Aragorn::the()->font.value(),
                        static_cast<Colours>(token).fg());
                    break;
                }
            }
        }
    }
    if (buffer()->mode()) {
        buffer()->mode()->draw();
    }

    for (auto const g : Aragorn::the()->guides) {
        if (g > left_column && g < left_column + columns()) {
            draw_line(
                static_cast<float>(g) * ed->cell.x,
                0,
                static_cast<float>(g) * ed->cell.x,
                static_cast<int>(viewport.height),
                Theme::the().fg());
        }
    }
    ++frame;
}

void BufferView::process_input()
{
    assert(num_clicks >= 0 && num_clicks < 3);
    if (!contains(GetMousePosition())) {
        return;
    }
    if (float const mouse_move = GetMouseWheelMove(); mouse_move != 0.0) {
        auto const move = static_cast<int>(-mouse_move);
        if (is_modifier_down(KModSuper)) {
            Aragorn::the()->change_font_size(move);
        } else {
            if (move < 0) {
                lines_down(move, is_modifier_down(KModShift));
            }
            if (move > 0) {
                lines_up(move, is_modifier_down(KModShift));
            }
        }
        return;
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        bool select = is_modifier_down(KModShift);
        auto mouse_line = static_cast<size_t>((static_cast<float>(GetMouseY()) - viewport.y) / Aragorn::the()->cell.y);
        auto mouse_col = static_cast<size_t>((static_cast<float>(GetMouseX()) - viewport.x) / Aragorn::the()->cell.x);
        auto lineno = min(mouse_line + top_line, m_buf->lines.size() - 1);
        auto col = min(mouse_col + left_column, m_buf->lines[lineno].length());
        move_cursor(CursorMovement::by_position(lineno, col));
        if (num_clicks > 0 && (Aragorn::the()->time - clicks[num_clicks - 1]) > 0.5) {
            num_clicks = 0;
        }
        clicks[num_clicks] = Aragorn::the()->time;
        ++num_clicks;
        switch (num_clicks) {
        case 1:
            break;
        case 2:
            set_mark(m_buf->word_boundary_left(cursor));
            move_cursor(CursorMovement::by_index(m_buf->word_boundary_right(cursor), true));
            break;
        case 3: {
            lineno = m_buf->line_for_index(cursor);
            Line const &line = m_buf->lines[lineno];
            set_mark(line.begin());
            move_cursor(CursorMovement::by_index(line.end() + 1, true));
        }
            // Fall through
        default:
            num_clicks = 0;
        }
    }
    assert(num_clicks >= 0 && num_clicks < 3);
}

void BufferView::copy()
{
    if (!has_selection()) {
        select_line();
    }
    selection_to_clipboard();
}

void BufferView::cut()
{
    copy();
    delete_selection();
}

void BufferView::paste()
{
    auto text = GetClipboardText();
    insert_string(MUST_EVAL(to_wstring(GetClipboardText())));
}

void BufferView::move_up(bool select)
{
    lines_up(1, select);
}

void BufferView::move_down(bool select)
{
    lines_down(1, select);
}

void BufferView::move_left(bool select)
{
    if (cursor > 0) {
        move_cursor(CursorMovement::by_index(cursor - 1, select));
    }
}

void BufferView::move_word_left(bool select)
{
    auto new_cursor = cursor;
    if (new_cursor > 0) {
        while (new_cursor > 0 && !isalnum((*m_buf)[new_cursor])) {
            --new_cursor;
        }
        while (new_cursor > 0 && isalnum((*m_buf)[new_cursor])) {
            --new_cursor;
        }
        if (new_cursor > 0 || !isalnum((*m_buf)[0])) {
            ++new_cursor;
        }
    }
    move_cursor(CursorMovement::by_index(new_cursor, select));
}

void BufferView::move_right(bool select)
{
    if (cursor < m_buf->length() - 1) {
        move_cursor(CursorMovement::by_index(cursor + 1, select));
    }
}

void BufferView::move_word_right(bool select)
{
    size_t len = m_buf->length();
    auto   new_cursor = cursor;
    if (new_cursor < len - 1) {
        while (new_cursor < len - 1 && !isalnum((*m_buf)[new_cursor])) {
            ++new_cursor;
        }
        while (new_cursor < len - 1 && isalnum((*m_buf)[new_cursor])) {
            ++new_cursor;
        }
    }
    move_cursor(CursorMovement::by_index(new_cursor, select));
}

void BufferView::move_begin_of_line(bool select)
{
    Line const &line = m_buf->lines[cursor_line];
    move_cursor(CursorMovement::by_index(line.begin(), select));
}

void BufferView::move_end_of_line(bool select)
{
    Line const &line = m_buf->lines[cursor_line];
    move_cursor(CursorMovement::by_index(line.end(), select));
}

void BufferView::move_top(bool select)
{
    move_cursor(CursorMovement::by_index(0, select));
}

void BufferView::move_bottom(bool select)
{
    move_cursor(CursorMovement::by_index(m_buf->length(), select));
}

void BufferView::move_cursor(BufferView::CursorMovement const &move)
{
    if (move.extend_selection) {
        if (!m_selection.has_value()) {
            set_mark(cursor);
        }
    } else {
        clear_selection();
    }
    if (move.index) {
        cursor = *move.index;
        auto cursor_pos = m_buf->index_to_position(cursor, { { cursor_col, cursor_line } });
        cursor_line = cursor_pos.y;
        cursor_col = cursor_pos.x;
        Line const &line = m_buf->lines[cursor_pos.line];
        cursor = clamp(cursor, line.begin(), line.end());
    } else if (move.pos) {
        cursor_line = clamp(move.pos->y, 0, m_buf->lines.size() - 1);
        cursor_col = move.pos->x;
        Line const &line = m_buf->lines[cursor_line];
        for (auto const &t : line.tokens) {
            if (t.kind() == TokenKind::EndOfLine) {
                cursor = t.index();
                break;
            }
            if (t.column() + t.length() >= cursor_col) {
                cursor = t.index() + std::min(std::max(cursor_col - t.column(), 0ul), t.length());
                break;
            }
        }
    } else {
        assert(false);
    }
    if (cursor_line < top_line) {
        top_line = cursor_line;
    }
    if (cursor_line >= top_line + lines()) {
        top_line = cursor_line - lines() + 1;
    }
    if (cursor_col < left_column) {
        left_column = cursor_col;
    }
    if (cursor_col >= left_column + columns()) {
        left_column = cursor_col - columns() + 1;
    }
    cursor_flash = 0;
}

bool BufferView::has_selection() const
{
    if (m_selection.has_value()) {
        if (cursor != *m_selection) {
            return true;
        }
    }
    return false;
}

Vec<size_t> BufferView::cursor_position() const
{
    return { cursor_col, cursor_line };
}

Vec<size_t> BufferView::view_offset() const
{
    return Vec<size_t> { .line = top_line, .column = left_column };
}

size_t BufferView::index() const
{
    return cursor;
}

std::optional<Vec<size_t>> BufferView::selection() const
{
    if (has_selection()) {
        auto selection_start = min(m_selection.value(), cursor);
        auto selection_end = max(m_selection.value(), cursor);
        return Vec<size_t> { selection_start, selection_end };
    }
    return {};
}

void BufferView::clear_selection()
{
    m_selection.reset();
}

void BufferView::set_mark(size_t at)
{
    m_selection = at;
}

bool BufferView::find_first(rune_view const &pattern)
{
    m_find_text = pattern;
    return find_next();
}

bool BufferView::find_next()
{
    assert(!m_find_text.empty());
    auto const &b = buffer();
    auto        pos = b->find(m_find_text, cursor);
    if (pos == std::string::npos) {
        pos = b->find(m_find_text);
    }
    if (pos != rune_view::npos) {
        set_mark(pos);
        move_cursor(CursorMovement::by_index(pos + m_find_text.length(), true));
        return true;
    }
    return false;
}

void BufferView::replacement(rune_view const &replacement)
{
    m_replacement = replacement;
}

void BufferView::clear_replacement()
{
    m_replacement = L"";
}

void BufferView::replace()
{
    assert(!m_replacement.empty());
    delete_selection();
    insert(cursor, m_replacement);
}

}
