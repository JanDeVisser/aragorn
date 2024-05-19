/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <App/Eddy.h>
#include <App/Editor.h>
#include <App/Modal.h>
#include <App/MiniBuffer.h>

namespace Eddy {

constexpr char const* OPEN_BRACES = "({[";
constexpr char const* CLOSE_BRACES = ")}]";

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
    view->manage_selection(do_select(key_combo));
    view->lines_up(1);
}

void cmd_select_word(pBufferView const &view, JSONValue const &)
{
    view->select_word();
}

void cmd_down(pBufferView const &view, JSONValue const &key_combo)
{
    view->manage_selection(do_select(key_combo));
    view->lines_down(1);
}

void cmd_left(pBufferView const &view, JSONValue const &key_combo)
{
    view->manage_selection(do_select(key_combo));
    if (view->new_cursor > 0) {
        --view->new_cursor;
    }
    view->cursor_col = -1;
}

void cmd_word_left(pBufferView const &view, JSONValue const &key_combo)
{
    view->manage_selection(do_select(key_combo));
    if (view->new_cursor > 0) {
        auto const &buffer = view->buffer();
        while (0 < ((int) view->new_cursor) && !isalnum(buffer->text[view->new_cursor])) {
            --view->new_cursor;
        }
        while (0 < ((int) view->new_cursor) && isalnum(buffer->text[view->new_cursor])) {
            --view->new_cursor;
        }
        view->new_cursor = ((int) view->new_cursor >= 0) ? view->new_cursor + 1 : 0;
    }
    view->cursor_col = -1;
}

void cmd_right(pBufferView const &view, JSONValue const &key_combo)
{
    auto const &buffer = view->buffer();
    view->manage_selection(do_select(key_combo));
    if (view->new_cursor < buffer->text.length() - 1) {
        ++view->new_cursor;
    }
    view->cursor_col = -1;
}

void cmd_word_right(pBufferView const &view, JSONValue const &key_combo)
{
    auto const &buffer = view->buffer();
    view->manage_selection(do_select(key_combo));
    size_t len = buffer->text.length();
    if (view->new_cursor < len - 1) {
        while (view->new_cursor < len - 1 && !isalnum(buffer->text[view->new_cursor])) {
            ++view->new_cursor;
        }
        while (view->new_cursor < len - 1 && isalnum(buffer->text[view->new_cursor])) {
            ++view->new_cursor;
        }
    }
    view->cursor_col = -1;
}

void cmd_begin_of_line(pBufferView const &view, JSONValue const &)
{
    auto const &buffer = view->buffer();
    assert(view->cursor_pos.y < buffer->lines.size());
    Index const &line = buffer->lines[view->cursor_pos.y];
    view->new_cursor = line.index_of;
    view->cursor_col = -1;
}

void cmd_top_of_buffer(pBufferView const &view, JSONValue const &)
{
    view->new_cursor = 0;
    view->cursor_col = -1;
    view->top_line = 0;
    view->left_column = 0;
}

void cmd_end_of_line(pBufferView const &view, JSONValue const &)
{
    auto const &buffer = view->buffer();
    assert(view->cursor_pos.y < buffer->lines.size());
    Index const &line = buffer->lines[view->cursor_pos.y];
    view->new_cursor = line.index_of + line.length;
    view->cursor_col = -1;
}

void cmd_page_up(pBufferView const &view, JSONValue const &)
{
    view->lines_up(view->lines());
}

void cmd_page_down(pBufferView const &view, JSONValue const &)
{
    view->lines_down(view->lines());
}

void cmd_top(pBufferView const &view, JSONValue const &)
{
    view->new_cursor = 0;
    view->cursor_col = -1;
}

void cmd_bottom(pBufferView const &view, JSONValue const &)
{
    auto const &buffer = view->buffer();
    view->new_cursor = buffer->text.length();
    view->cursor_col = -1;
}

void cmd_split_line(pBufferView const &view, JSONValue const &)
{
    view->character('\n');
}

void cmd_merge_lines(pBufferView const &view, JSONValue const &)
{
    auto const  &buffer = view->buffer();
    Index const &line = buffer->lines[view->cursor_pos.y];
    view->new_cursor = line.index_of + line.length;
    buffer->merge_lines(view->cursor_pos.y);
    view->cursor_col = -1;
}

void find_closing_brace(pBufferView const &view, size_t index, bool selection)
{
    auto const &buffer = view->buffer();
    int         brace = buffer->text[index];
    int         matching = get_closing_brace_code(brace);
    assert(matching > 0);
    if (selection) {
        view->selection = index;
    }
    int depth = 1;
    while (++index < buffer->text.length()) {
        if (buffer->text[index] == matching) {
            --depth;
        }
        if (!depth) {
            view->new_cursor = ++index;
            view->cursor_col = -1;
            return;
        }
        if (buffer->text[index] == brace) {
            ++depth;
        }
    }
}

void find_opening_brace(pBufferView const &view, size_t index, bool selection)
{
    auto const &buffer = view->buffer();
    int         brace = buffer->text[index];
    char        matching = 0;
    for (size_t ix = 0; ix < 3; ++ix) {
        if (CLOSE_BRACES[ix] == brace) {
            matching = OPEN_BRACES[ix];
            break;
        }
    }
    if (selection) {
        view->selection = index;
    }
    assert(matching);
    int depth = 1;
    while (--index != -1) {
        if (buffer->text[index] == matching) {
            --depth;
        }
        if (!depth) {
            view->new_cursor = index;
            view->cursor_col = -1;
            return;
        }
        if (buffer->text[index] == brace) {
            ++depth;
        }
    }
}

void cmd_matching_brace(pBufferView const &view, JSONValue const &key_combo)
{
    auto const &buffer = view->buffer();
    bool        selection = do_select(key_combo);
    if (strchr(OPEN_BRACES, buffer->text[view->cursor])) {
        find_closing_brace(view, view->cursor, selection);
        return;
    }
    if (strchr(OPEN_BRACES, buffer->text[view->cursor - 1])) {
        find_closing_brace(view, view->cursor - 1, selection);
        return;
    }
    if (strchr(CLOSE_BRACES, buffer->text[view->cursor])) {
        find_opening_brace(view, view->cursor, selection);
        return;
    }
    if (strchr(CLOSE_BRACES, buffer->text[view->cursor - 1])) {
        find_opening_brace(view, view->cursor - 1, selection);
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
    view->selection = -1;
}

void cmd_copy(pBufferView const &view, JSONValue const &)
{
    if (view->selection == -1) {
        view->select_line();
    }
    view->selection_to_clipboard();
}

void cmd_cut(pBufferView const &view, JSONValue const &)
{
    if (view->selection == -1) {
        view->select_line();
    }
    view->selection_to_clipboard();
    view->new_cursor = view->delete_selection();
    view->cursor_col = -1;
    view->selection = -1;
}

void cmd_paste(pBufferView const &view, JSONValue const &)
{
    char const *text = GetClipboardText();
    view->insert_string(text);
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
    view->find_text = query;
    view->find_next();
}

void cmd_find(pBufferView const &view, JSONValue const &)
{
    view->find_text = {};
    view->replacement = {};
    MiniBuffer::query(view, "Find", do_find);
}

void cmd_find_next(pBufferView const &view, JSONValue const &)
{
    if (view->find_text.empty()) {
        return;
    }
    view->find_next();
}

void do_ask_replace(pBufferView const &view, std::string const &reply)
{
    int         cmd = 0;
    if (!reply.empty()) {
        cmd = toupper(reply[0]);
    }
    switch (cmd) {
    case 'Y': {
        view->delete_selection();
        view->insert_string(view->replacement);
    } break;
    case 'N':
        break;
    case 'A': {
        do {
            view->delete_selection();
            view->insert_string(view->replacement);
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
    view->replacement = replacement;
    if (!view->find_next()) {
        Eddy::the()->set_message("Not found");
        return;
    }
    MiniBuffer::query(view, "Replace ((Y)es/(N)o/(A)ll/(Q)uit)", do_ask_replace);
}

void do_find_query(pBufferView const &view, std::string const &query)
{
    view->find_text = query;
    MiniBuffer::query(view, "Replace with", do_replacement_query);
}

void cmd_find_replace(pBufferView const &view, JSONValue const &)
{
    view->find_text = {};
    view->replacement = {};
    MiniBuffer::query(view, "Find", do_find_query);
}

void do_goto(pBufferView const &view, std::string const &query)
{
    StringList coords = split(query, ':');
    int        line = -1;
    int        col = -1;
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
        view->move(line - 1, (col >= 1) ? col - 1 : col);
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

BufferView::BufferView(pEditor editor, pBuffer buf)
    : Widget()
    , m_buf(std::move(buf))
{
    viewport = editor->viewport;
    padding = editor->padding;
    parent = std::move(editor);
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
    add_command<BufferView>("cursor-top", cmd_top)
        .bind(KeyCombo { KEY_HOME, KModControl })
        .bind(KeyCombo { KEY_HOME, KModControl | KModShift });
    add_command<BufferView>("cursor-bottom", cmd_bottom)
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

pBuffer const &BufferView::buffer() const
{
    return m_buf;
}

void BufferView::manage_selection(bool sel)
{
    if (sel) {
        if (selection == -1) {
            selection = cursor;
        }
    } else {
        selection = -1;
    }
}

bool BufferView::find_next()
{
    assert(!find_text.empty());
    auto const &b = buffer();
    auto        pos = b->text.find(find_text, new_cursor);
    if (pos == std::string::npos) {
        pos = b->text.find(find_text);
    }
    if (pos != std::string::npos) {
        selection = pos;
        new_cursor = pos + find_text.length();
        cursor_col = -1;
        return true;
    }
    return false;
}

int BufferView::lines() const
{
    pEditor editor = std::dynamic_pointer_cast<Editor>(parent);
    return editor->lines;
}

int BufferView::columns() const
{
    pEditor editor = std::dynamic_pointer_cast<Editor>(parent);
    return editor->columns;
}

void BufferView::update_cursor()
{
    if (new_cursor == cursor) {
        return;
    }

    if (new_cursor != -1) {
        cursor_pos.line = m_buf->line_for_index(new_cursor);
    }
    auto &current_line = m_buf->lines[cursor_pos.line];
    if (new_cursor == -1) {
        assert(cursor_col >= 0);
        if (((int) current_line.length) <= (cursor_col - 1)) {
            cursor_pos.column = current_line.length;
        } else {
            cursor_pos.column = cursor_col;
        }
        new_cursor = current_line.index_of + cursor_pos.column;
    } else {
        cursor_pos.column = new_cursor - current_line.index_of;
    }
    cursor = new_cursor;

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

void BufferView::lines_up(int count)
{
    if (cursor_pos.y == 0) {
        return;
    }
    new_cursor = -1;
    if (cursor_col < 0) {
        cursor_col = cursor_pos.x;
    }
    cursor_pos.y = clamp(cursor_pos.y - count, 0, cursor_pos.y);
}

void BufferView::lines_down(int count)
{
    if (cursor_pos.y >= m_buf->lines.size() - 1) {
        return;
    }
    new_cursor = -1;
    if (cursor_col < 0) {
        cursor_col = cursor_pos.x;
    }
    cursor_pos.y = clamp(cursor_pos.y + count, 0, max(0, m_buf->lines.size() - 1));
}

void BufferView::move(int line, int col)
{
    new_cursor = -1;
    cursor_pos.y = clamp(line, 0, m_buf->lines.size() - 1);
    cursor_col = clamp(col, 0, max(0, m_buf->lines[cursor_pos.y].length - 1));
}

void BufferView::select_line()
{
    size_t lineno = m_buf->line_for_index(cursor);
    Index &line = m_buf->lines[lineno];
    selection = line.index_of;
    new_cursor = cursor = line.index_of + line.length + 1;
}

void BufferView::word_left()
{
    while (0 < ((int) cursor) && !isalnum(m_buf->text[cursor])) {
        ++cursor;
    }
    while (0 < ((int) cursor) && isalnum(m_buf->text[cursor])) {
        ++cursor;
    }
    ++cursor;
}

void BufferView::word_right()
{
    while (cursor < m_buf->text.length() - 1 && !isalnum(m_buf->text[cursor])) {
        ++cursor;
    }
    while (cursor < m_buf->text.length() - 1 && isalnum(m_buf->text[cursor])) {
        ++cursor;
    }
}

void BufferView::select_word()
{
    selection = m_buf->word_boundary_left(cursor);
    new_cursor = m_buf->word_boundary_right(cursor);
}

void BufferView::insert(size_t at, std::string_view const &text)
{
    m_buf->insert(at, text);
}

void BufferView::del(size_t at, size_t count)
{
    m_buf->del(at, count);
    new_cursor = at;
    cursor_col = -1;
    selection = -1;
}

int BufferView::delete_selection()
{
    int selection_start = -1;
    if (selection != -1) {
        selection_start = min(selection, new_cursor);
        int selection_end = max(selection, new_cursor);
        del(selection_start, selection_end - selection_start);
        new_cursor = selection_start;
        selection = -1;
    }
    return selection_start;
}

void BufferView::backspace()
{
    if (selection == -1) {
        if (cursor != 0) {
            del(cursor - 1, 1);
            new_cursor = cursor - 1;
            cursor_col = -1;
        }
    } else {
        new_cursor = delete_selection();
        cursor_col = -1;
    }
}

void BufferView::delete_current_char()
{
    if (selection == -1) {
        if (cursor < m_buf->text.length()) {
            del(cursor, 1);
            new_cursor = cursor;
            cursor_col = -1;
        }
    } else {
        new_cursor = delete_selection();
        cursor_col = -1;
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
    size_t at = new_cursor;
    if (selection != -1) {
        switch (ch) {
        case '(':
        case '[':
        case '{': {
            int close = get_closing_brace_code(ch);
            int selection_start = min((int) selection, (int) new_cursor);
            int selection_end = max((int) selection, (int) new_cursor);
            insert(selection_start, std::string_view { (char const *) &ch, 1 });
            insert(selection_end + 1, std::string_view { (char const *) &close, 1 });
            if (cursor == selection_end) {
                new_cursor = selection_end + 1;
                selection = selection_start + 1;
            } else {
                new_cursor = selection_start + 1;
                selection = selection_end + 1;
            }
            cursor_col = -1;
            return true;
        }
        default:
            at = new_cursor = (size_t) delete_selection();
        }
    }
    insert(at, std::string_view { (char const *) &ch, 1 });
    new_cursor = at + 1;
    cursor_col = -1;
    return true;
}

void BufferView::insert_string(std::string_view const &sv)
{
    int at = new_cursor;
    if (selection != -1) {
        at = new_cursor = delete_selection();
    }
    insert(at, sv);
    new_cursor = at + sv.length();
    cursor_col = -1;
}

void BufferView::selection_to_clipboard()
{
    if (selection != -1) {
        int selection_start = min(selection, new_cursor);
        int selection_end = max(selection, new_cursor);
        SetClipboardText(m_buf->text.substr(selection_start, selection_end - selection_start).c_str());
    }
}

void BufferView::draw()
{
    static size_t frame = 1;
    update_cursor();
    draw_rectangle(0, 0, 0, 0, DARKGRAY /*colour_to_color(Eddy::the()->theme.editor.bg)*/);

    int selection_start = -1, selection_end = -1;
    if (selection != -1) {
        selection_start = min(selection, cursor);
        selection_end = max(selection, cursor);
    }
    std::string_view txt { buffer()->text };

    for (int row = 0; row < lines() && top_line + row < m_buf->lines.size(); ++row) {
        auto        lineno = top_line + row;
        auto const &line = m_buf->lines[lineno];
        auto        line_len = min(line.length - 1, left_column + columns());
        if (selection != -1) {
            auto line_start = line.index_of + left_column;
            auto line_end = min(line.index_of + line.length - 1, line_start + columns());
            auto selection_offset = clamp(selection_start - line_start, 0, line_end);

            if (selection_start < line_end && selection_end > line.index_of) {
                auto width = selection_end - max(selection_start, line_start);
                if (width > line_len - selection_offset) {
                    width = columns() - selection_offset;
                }
                draw_rectangle( Eddy::the()->cell.x * selection_offset, Eddy::the()->cell.y * row,
                    width * Eddy::the()->cell.x, Eddy::the()->cell.y + 5,
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
            int           start_col = (int) token.index - (int) line.index_of;

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
            //            if (frame == 0) {
            //                trace_nonl(EDIT, "[%zu %.*s]", ix, SV_ARG(text));
            //            }
            render_text(Eddy::the()->cell.x * (start_col - left_column), Eddy::the()->cell.y * row,
                text, Eddy::the()->font.value(), RAYWHITE /*token->color*/);
        }
        //        if (frame == 0) {
        //            trace_nl(EDIT);
        //        }
    }

#if 0
    Widget *mode = (mode_data) ? (Widget *) ((ModeData *) mode_data)->mode : NULL;
    if (mode != NULL && mode->handlers.draw != NULL) {
        mode->handlers.draw(mode_data);
    }
#endif

    double time = Eddy::the()->time - cursor_flash;
    if (time - floor(time) < 0.5) {
        int x = cursor_pos.x - left_column;
        int y = cursor_pos.y - top_line;
        draw_rectangle(x * Eddy::the()->cell.x, y * Eddy::the()->cell.y, 2, Eddy::the()->cell.y + 1,
            RAYWHITE /*colour_to_color(Eddy::the()->theme.editor.fg)*/);
    }
    DrawLine(viewport.x + 80 * Eddy::the()->cell.x, viewport.y,
        viewport.x + 80 * Eddy::the()->cell.x, viewport.y + viewport.height,
        RAYWHITE /*colour_to_color(Eddy::the()->theme.editor.fg)*/);
    DrawLine(viewport.x + 120 * Eddy::the()->cell.x, viewport.y,
        viewport.x + 120 * Eddy::the()->cell.x, viewport.y + viewport.height,
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
        if (IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) {
            Eddy::the()->change_font_size((int) -mouse_move);
        } else {
            if (mouse_move < 0) {
                lines_down((int) -mouse_move);
            }
            if (mouse_move > 0) {
                lines_up((int) mouse_move);
            }
        }
        return;
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        int         lineno = min((GetMouseY() - viewport.y) / Eddy::the()->cell.y + top_line,
                    m_buf->lines.size() - 1);
        int         col = min((GetMouseX() - viewport.x) / Eddy::the()->cell.x + left_column,
                    m_buf->lines[lineno].length);
        new_cursor = m_buf->lines[lineno].index_of + col;
        cursor_col = -1;
        if (num_clicks > 0 && (Eddy::the()->time - clicks[num_clicks - 1]) > 0.5) {
            num_clicks = 0;
        }
        clicks[num_clicks] = Eddy::the()->time;
        ++num_clicks;
        switch (num_clicks) {
        case 1:
            selection = -1;
            break;
        case 2:
            selection = m_buf->word_boundary_left(new_cursor);
            new_cursor = m_buf->word_boundary_right(new_cursor);
            break;
        case 3: {
            lineno = m_buf->line_for_index(new_cursor);
            Index &line = m_buf->lines[lineno];
            selection = line.index_of;
            new_cursor = line.index_of + line.length + 1;
        }
            // Fall through
        default:
            num_clicks = 0;
        }
    }
    assert(num_clicks >= 0 && num_clicks < 3);
}

}
