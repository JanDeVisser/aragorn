/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <App/Eddy.h>
#include <App/Editor.h>
#include <App/BufferView.h>
#include <App/MiniBuffer.h>
#include <App/Modal.h>

namespace Eddy {

constexpr char const *OPEN_BRACES = "({[";
constexpr char const *CLOSE_BRACES = ")}]";

int get_closing_brace_code(int brace);

bool do_select(JSONValue const &key_combo)
{
    if (key_combo.type() == JSONType::Boolean) {
        return value<bool>(key_combo).value();
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

void cmd_merge_lines(pBufferView const &view, JSONValue const &)
{
    auto const  &buffer = view->buffer();
    auto         pos = view->cursor_position();
    Index const &line = buffer->lines[pos.y];
    buffer->merge_lines(pos.y);
    view->move_cursor(BufferView::CursorMovement::by_index(line.index_of + line.length, true, false));
}

void find_closing_brace(pBufferView const &view, size_t index, bool selection)
{
    auto const &buffer = view->buffer();
    auto        brace = buffer->text[index];
    auto        matching = get_closing_brace_code(brace);
    assert(matching > 0);
    if (selection) {
        view->set_mark(index);
    }
    auto depth = 1;
    while (++index < buffer->text.length()) {
        if (buffer->text[index] == matching) {
            --depth;
        }
        if (!depth) {
            view->move_cursor(BufferView::CursorMovement::by_index(++index, true, false));
            return;
        }
        if (buffer->text[index] == brace) {
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
            view->move_cursor(BufferView::CursorMovement::by_index(index, selection, false));
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

void do_find(pBufferView const &view, std::string const &query)
{
    view->find_first(query);
}

void cmd_find(pBufferView const &view, JSONValue const &)
{
    MiniBuffer::query(view, "Find", do_find);
}

void cmd_find_next(pBufferView const &view, JSONValue const &)
{
    view->find_next();
}

void do_ask_replace(pBufferView const &view, std::string const &reply)
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
        MiniBuffer::query(view, "Replace ((Y)es/(N)o/(A)ll/(Q)uit)", do_ask_replace);
        return;
    }
    if (view->find_next()) {
        MiniBuffer::query(view, "Replace ((Y)es/(N)o/(A)ll/(Q)uit)", do_ask_replace);
        return;
    }
    Eddy::the()->set_message("Not found");
}

void do_replacement_query(pBufferView const &view, std::string const &replacement)
{
    view->replacement(replacement);
    MiniBuffer::query(view, "Replace ((Y)es/(N)o/(A)ll/(Q)uit)", do_ask_replace);
}

void do_find_query(pBufferView const &view, std::string const &query)
{
    if (!view->find_first(query)) {
        Eddy::the()->set_message("Not found");
        return;
    }
    MiniBuffer::query(view, "Replace with", do_replacement_query);
}

void cmd_find_replace(pBufferView const &view, JSONValue const &)
{
    MiniBuffer::query(view, "Find", do_find_query);
}

void do_goto(pBufferView const &view, std::string const &query)
{
    auto coords = split(query, ':');
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
            line, view->cursor_position().column, true, false);
        if (col >= 1) {
            movement.pos->column = col - 1;
        }
        view->move_cursor(movement);
    }
}

void cmd_goto(pBufferView const &view, JSONValue const &)
{
    MiniBuffer::query(view, "Move to", do_goto);
}

void save_as_submit(pBufferView const &view, std::string const &filename)
{
    auto const &buffer = view->buffer();
    buffer->save_as(filename);
    Eddy::the()->set_message("Buffer saved");
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
    Eddy::the()->set_message("Buffer saved");
}

BufferView::BufferView(pEditor const &editor, pBuffer buf)
    : Widget(std::dynamic_pointer_cast<Widget>(editor))
    , m_buf(std::move(buf))
{
    viewport = editor->viewport;
    padding = editor->padding;
    parent = editor;
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
    cursor_flash = Eddy::the()->time;
    Eddy::the()->focus = self();
    if (mode) {
        Eddy::the()->focus = mode;
    }
}

pBuffer const &BufferView::buffer() const
{
    return m_buf;
}

size_t BufferView::lines() const
{
    pEditor editor = std::dynamic_pointer_cast<Editor>(parent);
    return static_cast<size_t>(editor->lines);
}

size_t BufferView::columns() const
{
    pEditor editor = std::dynamic_pointer_cast<Editor>(parent);
    return static_cast<size_t>(editor->columns);
}

void BufferView::lines_up(size_t count, bool select)
{
    move_cursor(CursorMovement::by_position(
        cursor_pos.y - min(count, cursor_pos.y),
        cursor_pos.x,
        select, true));
}

void BufferView::lines_down(size_t count, bool select)
{
    move_cursor(CursorMovement::by_position(
        cursor_pos.y + min(count, m_buf->lines.size() - min(count, m_buf->lines.size())),
        cursor_pos.x,
        select, true));
}

void BufferView::move(size_t line, size_t col, bool select)
{
    move_cursor(CursorMovement::by_position(line, col, select, true));
}

void BufferView::select_line()
{
    size_t lineno = m_buf->line_for_index(cursor);
    Index &line = m_buf->lines[lineno];
    set_mark(line.index_of);
    move_cursor(CursorMovement::by_index(line.index_of + line.length + 1, true, false));
}

void BufferView::word_left()
{
    auto new_cursor = cursor;
    while (0 < new_cursor && !isalnum(m_buf->text[new_cursor])) {
        --new_cursor;
    }
    while (0 < new_cursor && isalnum(m_buf->text[new_cursor])) {
        --new_cursor;
    }
    if (new_cursor > 0 || !isalnum(m_buf->text[0])) {
        ++new_cursor;
    }
    move_cursor(CursorMovement::by_index(new_cursor, true, false));
}

void BufferView::word_right()
{
    auto new_cursor = cursor;
    while (new_cursor < m_buf->text.length() - 1 && !isalnum(m_buf->text[new_cursor])) {
        ++new_cursor;
    }
    while (new_cursor < m_buf->text.length() - 1 && isalnum(m_buf->text[new_cursor])) {
        ++new_cursor;
    }
    move_cursor(CursorMovement::by_index(new_cursor, true, false));
}

void BufferView::select_word()
{
    set_mark(m_buf->word_boundary_left(cursor));
    move_cursor(CursorMovement::by_index(m_buf->word_boundary_right(cursor), true, false));
}

void BufferView::insert(size_t at, std::string_view const &text)
{
    m_buf->insert(at, text);
    move_cursor(CursorMovement::by_index(at + text.length(), false, false));
}

void BufferView::del(size_t at, size_t count)
{
    m_buf->del(at, count);
    move_cursor(CursorMovement::by_index(at, false, false));
}

void BufferView::delete_selection()
{
    assert(has_selection());
    auto sel = selection();
    del(sel->coords[0], sel->coords[1] - sel->coords[0]);
    move_cursor(CursorMovement::by_index(sel->coords[0], false, false));
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
        if (cursor < m_buf->text.length()) {
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
        case '{': {
            int close = get_closing_brace_code(ch);
            insert(sel->coords[0], std::string_view { (char const *) &ch, 1 });
            insert(sel->coords[1] + 1, std::string_view { (char const *) &close, 1 });
            auto new_cursor = sel->coords[0] + 1;
            set_mark(sel->coords[1] + 1);
            if (cursor == sel->coords[1]) {
                set_mark(sel->coords[0] + 1);
                new_cursor = sel->coords[1] + 1;
            }
            move_cursor(CursorMovement::by_index(new_cursor, false, false));
            return true;
        };
        default:
            delete_selection();
            return true;
        }
    }
    insert(at, std::string_view { (char const *) &ch, 1 });
    return true;
}

void BufferView::insert_string(std::string_view const &sv)
{
    auto at = cursor;
    if (has_selection()) {
        delete_selection();
    }
    insert(at, sv);
    move_cursor(CursorMovement::by_index(at + sv.length(), false, false));
}

void BufferView::selection_to_clipboard()
{
    if (auto sel = selection(); sel.has_value()) {
        SetClipboardText(m_buf->text.substr(sel->coords[0], sel->coords[1] - sel->coords[0]).c_str());
    }
}

void BufferView::draw()
{
    if (buffer()->build_indices()) {
        cursor_pos = m_buf->index_to_position(cursor);
    }
    static size_t frame { 1 };
    draw_rectangle(0, 0, 0, 0, DARKGRAY /*colour_to_color(Eddy::the()->theme.editor.bg)*/);

    std::string_view txt { buffer()->text };

    for (int row = 0; row < lines() && top_line + row < m_buf->lines.size(); ++row) {
        auto        lineno = top_line + row;
        auto const &line = m_buf->lines[lineno];
        auto        line_len = min(line.length - 1, left_column + columns());
        if (has_selection()) {
            auto sel = selection();
            auto line_start = line.index_of + left_column;
            auto line_end = min(line.index_of + line.length - 1, line_start + columns());
            auto selection_offset = clamp(sel->coords[0] - min(sel->coords[0], line_start), 0, line_end);

            if (sel->coords[0] < line_end && sel->coords[1] > line.index_of) {
                auto width = sel->coords[1] - max(sel->coords[0], line_start);
                if (width > line_len - selection_offset) {
                    width = columns() - selection_offset;
                }
                draw_rectangle(
                    Eddy::the()->cell.x * selection_offset,
                    Eddy::the()->cell.y * row,
                    width * Eddy::the()->cell.x,
                    Eddy::the()->cell.y + 5.0f,
                    DARKGRAY /*colour_to_color(Eddy::the()->theme.selection.bg)*/);
            }
        }
        if (line.num_tokens == 0) {
            if (frame == 0) {
                trace(EDIT, "%5d:%5zu:[          ]", row, lineno);
            }
            continue;
        }
        //        if (frame == 0) {
        //            trace_nonl(EDIT, "%5d:%5zu:[%4zu..%4zu]", row, lineno, line.first_token, line.first_token + line.num_tokens - 1);
        //        }
        for (size_t ix = line.first_token; ix < line.first_token + line.num_tokens; ++ix) {
            DisplayToken &token = m_buf->tokens[ix];
            auto          start_col = token.index - line.index_of;

            // token ends before left edge
            if (start_col + (int) token.length <= (int) left_column) {
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
            auto length = token.length - (start_col - ((int) token.index - (int) line.index_of));
            // If start + length overflows right edge, clip length:
            if (start_col + length > left_column + columns()) {
                length = left_column + columns() - start_col;
            }

            std::string_view text { txt.substr(line.index_of + start_col, length) };
            render_text(
                Eddy::the()->cell.x * static_cast<float>(start_col - left_column),
                Eddy::the()->cell.y * row,
                text,
                Eddy::the()->font.value(),
                RAYWHITE /*token->color*/);
        }
    }

#if 0
    Widget *mode = (mode_data) ? (Widget *) ((ModeData *) mode_data)->mode : NULL;
    if (mode != NULL && mode->handlers.draw != NULL) {
        mode->handlers.draw(mode_data);
    }
#endif

    double time = Eddy::the()->time - cursor_flash;
    if (time - floor(time) < 0.5) {
        auto x = cursor_pos.x - left_column;
        auto y = cursor_pos.y - top_line;
        draw_rectangle(
            x * Eddy::the()->cell.x,
            y * Eddy::the()->cell.y,
            2,
            Eddy::the()->cell.y + 1,
            RAYWHITE /*colour_to_color(Eddy::the()->theme.editor.fg)*/);
    }
    DrawLine(
        viewport.x + 80 * Eddy::the()->cell.x,
        viewport.y,
        viewport.x + 80 * Eddy::the()->cell.x,
        viewport.y + viewport.height,
        RAYWHITE /*colour_to_color(Eddy::the()->theme.editor.fg)*/);
    DrawLine(
        viewport.x + 120 * Eddy::the()->cell.x,
        viewport.y,
        viewport.x + 120 * Eddy::the()->cell.x,
        viewport.y + viewport.height,
        RAYWHITE /*colour_to_color(Eddy::the()->theme.editor.fg)*/);
    ++frame;
}

void BufferView::process_input()
{
    assert(num_clicks >= 0 && num_clicks < 3);
    if (!contains(GetMousePosition())) {
        return;
    }
    float mouse_move = GetMouseWheelMove();
    if (mouse_move != 0.0) {
        if (is_modifier_down(KModSuper)) {
            Eddy::the()->change_font_size((int) -mouse_move);
        } else {
            if (mouse_move < 0) {
                lines_down((int) -mouse_move, is_modifier_down(KModShift));
            }
            if (mouse_move > 0) {
                lines_up((int) mouse_move, is_modifier_down(KModShift));
            }
        }
        return;
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        bool select = is_modifier_down(KModShift);
        auto mouse_line = static_cast<size_t>((static_cast<float>(GetMouseY()) - viewport.y) / Eddy::the()->cell.y);
        auto mouse_col = static_cast<size_t>((static_cast<float>(GetMouseX()) - viewport.x) / Eddy::the()->cell.x);
        auto lineno = min(mouse_line + top_line, m_buf->lines.size() - 1);
        auto col = min(mouse_col + left_column, m_buf->lines[lineno].length);
        move_cursor(CursorMovement::by_position(lineno, col, false, false));
        if (num_clicks > 0 && (Eddy::the()->time - clicks[num_clicks - 1]) > 0.5) {
            num_clicks = 0;
        }
        clicks[num_clicks] = Eddy::the()->time;
        ++num_clicks;
        switch (num_clicks) {
        case 1:
            break;
        case 2:
            set_mark(m_buf->word_boundary_left(cursor));
            move_cursor(CursorMovement::by_index(m_buf->word_boundary_right(cursor), true, false));
            break;
        case 3: {
            lineno = m_buf->line_for_index(cursor);
            Index &line = m_buf->lines[lineno];
            set_mark(line.index_of);
            move_cursor(CursorMovement::by_index(line.index_of + line.length + 1, true, false));
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
    insert_string(GetClipboardText());
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
        move_cursor(CursorMovement::by_index(cursor - 1, select, false));
    }
}

void BufferView::move_word_left(bool select)
{
    auto new_cursor = cursor;
    if (new_cursor > 0) {
        while (new_cursor > 0 && !isalnum(m_buf->text[new_cursor])) {
            --new_cursor;
        }
        while (new_cursor > 0 && isalnum(m_buf->text[new_cursor])) {
            --new_cursor;
        }
        if (new_cursor > 0 || !isalnum(m_buf->text[0])) {
            ++new_cursor;
        }
    }
    move_cursor(CursorMovement::by_index(new_cursor, select, false));
}

void BufferView::move_right(bool select)
{
    if (cursor < m_buf->text.length() - 1) {
        move_cursor(CursorMovement::by_index(cursor + 1, select, false));
    }
}

void BufferView::move_word_right(bool select)
{
    size_t len = m_buf->text.length();
    auto   new_cursor = cursor;
    if (new_cursor < len - 1) {
        while (new_cursor < len - 1 && !isalnum(m_buf->text[new_cursor])) {
            ++new_cursor;
        }
        while (new_cursor < len - 1 && isalnum(m_buf->text[new_cursor])) {
            ++new_cursor;
        }
    }
    move_cursor(CursorMovement::by_index(new_cursor, select, false));
}

void BufferView::move_begin_of_line(bool select)
{
    Index const &line = m_buf->lines[cursor_pos.y];
    move_cursor(CursorMovement::by_index(line.index_of, select, false));
}

void BufferView::move_end_of_line(bool select)
{
    Index const &line = m_buf->lines[cursor_pos.y];
    move_cursor(CursorMovement::by_index(line.index_of + line.length, select, false));
}

void BufferView::move_top(bool select)
{
    move_cursor(CursorMovement::by_index(0, select, false));
}

void BufferView::move_bottom(bool select)
{
    move_cursor(CursorMovement::by_index(m_buf->text.length(), select, false));
}

void BufferView::move_cursor(BufferView::CursorMovement const &move)
{
    if (move.index) {
        cursor = *move.index;
        cursor_pos = m_buf->index_to_position(cursor);
        Index const &line = m_buf->lines[cursor_pos.line];
        cursor = clamp(cursor, line.index_of, line.end());
    } else if (move.pos) {
        auto col = move.pos->column;
        if (move.with_virtual_column && cursor_col) {
            col = cursor_col.value();
        }
        cursor_pos.line = clamp(move.pos->y, 0, m_buf->lines.size() - 1);
        Index const &line = m_buf->lines[cursor_pos.line];
        cursor_pos.column = clamp(col, 0, max(1, line.length) - 1);
        cursor = line.index_of + cursor_pos.column;
    } else {
        assert(false);
    }
    if (move.extend_selection) {
        if (!m_selection.has_value()) {
            set_mark(cursor);
        }
    } else {
        clear_selection();
    }
    if (!move.with_virtual_column) {
        cursor_col.reset();
    }
    if (cursor_pos.line < top_line) {
        top_line = cursor_pos.line;
    }
    if (cursor_pos.line >= top_line + lines()) {
        top_line = cursor_pos.line - lines() + 1;
    }
    if (cursor_pos.column < left_column) {
        left_column = cursor_pos.column;
    }
    if (cursor_pos.column >= left_column + columns()) {
        left_column = cursor_pos.column - columns() + 1;
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
    return cursor_pos;
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

bool BufferView::find_first(std::string_view const &pattern)
{
    m_find_text = pattern;
    return find_next();
}

bool BufferView::find_next()
{
    assert(!m_find_text.empty());
    auto const &b = buffer();
    auto        pos = b->text.find(m_find_text, cursor);
    if (pos == std::string::npos) {
        pos = b->text.find(m_find_text);
    }
    if (pos != std::string::npos) {
        set_mark(pos);
        move_cursor(CursorMovement::by_index(pos + m_find_text.length(), true, false));
        return true;
    }
    return false;
}

void BufferView::replacement(std::string_view const &replacement)
{
    m_replacement = replacement;
}

void BufferView::clear_replacement()
{
    m_replacement = "";
}

void BufferView::replace()
{
    assert(!m_replacement.empty());
    delete_selection();
    insert(cursor, m_replacement);
}

}
