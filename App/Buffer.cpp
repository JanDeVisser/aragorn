/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <cctype>
#include <codecvt>
#include <print>

#include <LibCore/IO.h>
#include <LibCore/ScopeGuard.h>

#include <App/Aragorn.h>
#include <App/Buffer.h>
#include <LSP/LSP.h>
#include <LSP/Schema/SemanticTokens.h>

namespace Aragorn {

using namespace LibCore;
using namespace std::literals::string_literals;

void semantic_tokens_response(pWidget const &widget, JSONValue const &resp);

Buffer::Buffer(pWidget const &parent)
    : Widget(parent)
{
}

void Buffer::initialize()
{
    add_command<Buffer>(
        "lsp-textDocument/semanticTokens/full",
        semantic_tokens_response);
}

Result<pBuffer> Buffer::open(std::string_view const &name)
{
    auto buffer = Widget::make<Buffer>(Aragorn::the());
    buffer->name = name;
    buffer->m_mode = Aragorn::the()->get_mode_for_buffer(buffer);
    if (auto listener = buffer->m_mode->event_listener(); listener) {
        buffer->add_listener(listener);
    }
    auto contents = TRY_EVAL(read_file_by_name<rune>(name));
    auto cap = static_cast<size_t>(static_cast<float>(contents.length()) * 1.2);
    buffer->text_size = contents.length();
    buffer->end_gap = cap - buffer->text_size;
    buffer->m_text.resize(cap, 0);
    auto i = buffer->it(buffer->end_gap);
    std::ranges::copy(contents, i);
    buffer->apply(BufferEvent::make_open());
    buffer->lex();
    return buffer;
}

pBuffer Buffer::new_buffer()
{
    auto buffer = Widget::make<Buffer>(Aragorn::the());
    buffer->lex();
    //    mode = aragorn_get_mode_for_buffer(&aragorn, name);
    //    if (mode) {
    //        buffer_add_listener(buffer, mode->event_listener);
    //    }
    return buffer;
}

void Buffer::close()
{
    BufferEvent event;
    event.type = BufferEventType::Close;
    apply(event);
}

bool Buffer::lex()
{
    assert(indexed_version <= version);
    if (indexed_version == version && !lines.empty()) {
        return false;
    }
    lines.clear();
    if (text_size == 0) {
        return true;
    }

    mode()->initialize_source();
    lock();
    ScopeGuard sg { [this]() {
        unlock();
    } };
    // std::println("Lexing...");

    auto new_line = [this]() -> Line * {
        return &lines.emplace_back();
    };
    bool   done = false;
    Line  *current = new_line();
    size_t lineno { 0 };
    // std::print("{}: ", lineno);
    do {
        auto const t = mode()->lex();
        // std::print("{} ", t.index());
        // switch (t.kind()) {
        // case TokenKind::EndOfLine:
        //     std::println("\\n");
        //     break;
        // case TokenKind::Symbol:
        //     std::print("{} [{}] ", t.kind(), at(t.index()));
        //     break;
        // default:
        //     std::print("{} ", t.kind());
        //     break;
        // }
        current->tokens.emplace_back(t);
        switch (t.kind()) {
        case TokenKind::EndOfFile:
            done = true;
            break;
        case TokenKind::EndOfLine: {
            assert(t.index() <= length());
            current = new_line();
            ++lineno;
            // std::print("{}: ", lineno);
        } break;
        default:
            break;
        }
        assert(lineno < 200);
    } while (!done);
    // std::println("Lexed...");
    indexed_version = version;
    BufferEvent event;
    event.type = BufferEventType::Indexed;
    for (auto &listener : listeners) {
        listener(std::dynamic_pointer_cast<Buffer>(self()), event);
    }
    // std::println("Lexing done...");
    return true;
}

size_t Buffer::line_for_index(size_t index, std::optional<Vec<size_t>> const &hint) const
{
    if (lines.empty() || index < lines[0].end()) {
        return 0;
    }
    if (index >= lines.back().begin()) {
        return lines.size() - 1;
    }

    auto on_line = [this, index](auto lineno) -> bool {
        auto const &line = lines[lineno];
        return (lineno < lines.size() - 1 && line.begin() <= index && lines[lineno + 1].begin() > index) || (lineno == lines.size() - 1 && line.begin() <= index);
    };
    if (hint) {
        auto const &line = lines[hint->line];
        if (on_line(hint->line)) {
            return hint->line;
        }
        if (index < line.begin() && on_line(hint->line - 1)) {
            return hint->line - 1;
        }
        if (index >= line.end() && on_line(hint->line + 1)) {
            return hint->line + 1;
        }
    }
    size_t line_min = 0;
    size_t line_max = lines.size() - 1;
    while (true) {
        auto lineno = line_min + (line_max - line_min) / 2;
        if (on_line(lineno)) {
            return lineno;
        }
        auto const &line = lines[lineno];
        if (line.begin() > index) {
            line_max = lineno;
        } else {
            line_min = lineno + 1;
        }
    }
}

Vec<size_t> Buffer::index_to_position(size_t index, std::optional<Vec<size_t>> const &hint) const
{
    Vec<size_t> ret { 0, 0 };
    if (index == 0) {
        return ret;
    }
    ret.line = line_for_index(index, hint);
    auto const &line = lines[ret.line];
    for (auto const &t : line.tokens) {
        if (t.index() <= index && t.index() + t.length() > index) {
            ret.column = t.column() + (index - t.index());
            break;
        }
    }
    return ret;
}

size_t Buffer::position_to_index(Vec<size_t> position) const
{
    Line const &line = lines[position.line];
    for (auto const &t : line.tokens) {
        if (t.column() <= position.column && t.column() + t.length() > position.column) {
            return t.index() + (position.column - t.column());
        }
    }
    return line.end();
}

void Buffer::set(size_t pos)
{
    assert(!m_locked);
    assert(pos <= text_size);
    if (pos == cursor) {
        return;
    }
    if (pos < cursor) {
        auto num = cursor - pos;
        std::copy(it(pos), it(cursor), it(end_gap - num));
        cursor = pos;
        end_gap -= num;
    } else {
        auto num = pos - cursor;
        std::copy(it(end_gap), it(end_gap + num), it(cursor));
        cursor = pos;
        end_gap += num;
    }
}

void Buffer::insert_rune(size_t pos, rune r)
{
    assert(pos < text_size);
    set(pos);
    m_text[cursor] = r;
    cursor += 1;
    text_size += 1;
}

void Buffer::ensure_capacity(size_t num)
{
    auto new_cap = m_text.size();
    while (text_size + num > static_cast<int>(static_cast<float>(new_cap) * 0.9)) {
        new_cap = static_cast<int>(static_cast<float>(new_cap) * 1.2);
    }
    if (new_cap > m_text.size()) {
        auto old_cap = m_text.size();
        m_text.resize(new_cap, 0);
        auto diff = m_text.size() - old_cap;
        std::copy(m_text.begin() + end_gap, m_text.begin() + old_cap, m_text.begin() + (end_gap + diff));
        end_gap += diff;
    }
}

void Buffer::insert_string(size_t pos, rune_view s)
{
    if (s.empty()) {
        return;
    }
    ensure_capacity(s.length());
    for (auto ix = 0; ix < s.length(); ++ix) {
        insert_rune(pos + ix, s[ix]);
    }
    ++version;
    lex();
}

void Buffer::append_string(rune_view s)
{
    if (s.empty()) {
        return;
    }
    ensure_capacity(s.length());
    for (auto r : s) {
        insert_rune(text_size, r);
    }
    ++version;
    lex();
}

void Buffer::erase(size_t pos, size_t len)
{
    if (text_size == 0 || len == 0) {
        return;
    }
    assert(pos < text_size);
    if (len == rune_view::npos || pos + len > text_size) {
        len = text_size - pos;
    }
    set(pos);
    end_gap += len;
    ++version;
    lex();
}

rune Buffer::at(size_t pos) const
{
    return (pos < cursor) ? m_text[pos] : m_text[end_gap + (pos - cursor)];
}

rune_string Buffer::substr(size_t pos, size_t len)
{
    assert(pos < text_size);
    if (len == rune_view::npos || pos + len > text_size) {
        len = text_size - pos;
    }
    rune_string ret;
    if (pos < cursor) {
        ret = rune_string { m_text.data() + pos, std::min(len, cursor - pos) };
        if (pos + len > cursor) {
            ret += rune_string { m_text.data() + end_gap, len - cursor };
        }
    } else {
        ret = rune_string { m_text.data() + end_gap + (pos - cursor), len };
    }
    return ret;
}

void Buffer::apply(BufferEvent const &event)
{
    switch (event.type) {
    case BufferEventType::Insert: {
        auto const &s = event.insert();
        if (s.empty()) {
            return;
        }

        if (event.position < text_size) {
            insert_string(event.position, s);
        } else {
            append_string(s);
        }
    } break;
    case BufferEventType::Delete: {
        auto const &deletion = event.deletion();
        erase(event.position, deletion.length());
    } break;
    case BufferEventType::Replace: {
        auto const &replacement = event.replacement();
        if (replacement.replacement.empty()) {
            return;
        }
        erase(event.position, replacement.overwritten.length());
        if (event.position < text_size) {
            insert_string(event.position, replacement.replacement);
        } else {
            append_string(replacement.replacement);
        }
        ++version;
    } break;
    case BufferEventType::Save: {
        assert(saved_version <= version);
        auto const &filename = event.filename();
        if (filename.has_value()) {
            name = filename.value();
        }
        if (name.empty() || saved_version == version) {
            return;
        }
        set(text_size);
        MUST(write_file_by_name(name, rune_view { m_text.data(), text_size }));
        saved_version = version;
    } break;
    case BufferEventType::Close: {
        for (auto &listener : listeners) {
            listener(std::dynamic_pointer_cast<Buffer>(self()), event);
        }
        m_text.clear();
        undo_stack.clear();
        undo_pointer = 0;
        lines.clear();
        version = 0;
        saved_version = 0;
        indexed_version = 0;
        name.clear();
        m_uri.clear();
        return;
    }
    default:
        break;
    }
    for (auto &listener : listeners) {
        listener(std::dynamic_pointer_cast<Buffer>(self()), event);
    }
}

void Buffer::edit(BufferEvent const &event)
{
    apply(event);
    if (undo_pointer < undo_stack.size()) {
        undo_stack.erase(undo_stack.begin() + static_cast<std::vector<BufferEvent>::difference_type>(undo_pointer), undo_stack.end());
    }
    undo_stack.push_back(event);
    undo_pointer = undo_stack.size();
}

void Buffer::undo()
{
    if (undo_pointer <= 0 || undo_pointer > undo_stack.size()) {
        return;
    }
    auto const &edit = undo_stack[--undo_pointer];
    auto        revert = edit.revert();
    apply(revert);
}

void Buffer::redo()
{
    if (undo_pointer >= undo_stack.size()) {
        return;
    }
    auto &edit = undo_stack[undo_pointer++];
    apply(edit);
}

void Buffer::insert(size_t pos, rune_string insert)
{
    if (insert.empty()) {
        return;
    }
    pos = clamp(pos, 0, text_size);
    EventRange range;
    range.start = range.end = index_to_position(static_cast<int>(pos));
    edit(BufferEvent::make_insert(range, pos, std::move(insert)));
}

void Buffer::del(size_t pos, size_t count)
{
    pos = clamp(pos, 0, text_size);
    count = clamp(count, 0, text_size - pos);
    if (count == 0) {
        return;
    }
    EventRange range;
    range.start = index_to_position(static_cast<int>(pos));
    range.end = index_to_position(static_cast<int>(pos + count));
    auto const del = substr(pos, count);
    edit(BufferEvent::make_delete(range, pos, del));
}

void Buffer::replace(size_t pos, size_t num, rune_string replacement)
{
    pos = clamp(pos, 0, text_size);
    num = clamp(num, 0, text_size - pos);
    if (num == 0) {
        return;
    }
    rune_string overwritten = substr(pos, num);
    EventRange  range;
    range.start = index_to_position(static_cast<int>(pos));
    range.end = index_to_position(static_cast<int>(pos + num));
    edit(BufferEvent::make_replacement(range, pos, overwritten, replacement));
}

void Buffer::merge_lines(size_t top_line)
{
    if (top_line > lines.size() - 1) {
        return;
    }
    Line &line = lines[top_line];
    replace(line.end(), 1, L" "s);
}

size_t Buffer::find(rune_view needle, size_t offset)
{
    auto pos = substr(offset).find(needle);
    if (pos == rune_view::npos) {
        return pos;
    }
    return offset + pos;
}

void Buffer::save()
{
    apply(BufferEvent::make_save());
}

void Buffer::save_as(std::string_view const &new_name)
{
    apply(BufferEvent::make_save_as(new_name));
}

size_t Buffer::word_boundary_left(size_t index) const
{
    index = clamp(index, 0, text_size - 1);
    if (isalnum(at(index)) || at(index) == '_') {
        while (static_cast<int>(index) > 0 && (isalnum(at(index)) || at(index) == '_')) {
            --index;
        }
        ++index;
    } else {
        while (static_cast<int>(index) > 0 && (!isalnum(at(index)) && at(index) != '_')) {
            --index;
        }
        ++index;
    }
    return index;
}

size_t Buffer::word_boundary_right(size_t index) const
{
    index = clamp(index, 0, text_size - 1);
    size_t max_index = text_size;
    if (isalnum(at(index)) || at(index) == '_') {
        while (index < max_index && (isalnum(at(index)) || at(index) == '_')) {
            ++index;
        }
    } else {
        while (index < max_index && (!isalnum(at(index)) && at(index) != '_')) {
            ++index;
        }
    }
    return index;
}

void Buffer::add_listener(BufferEventListener const &listener)
{
    listeners.emplace_back(listener);
}

std::string const &Buffer::uri()
{
    if (m_uri.empty() && !name.empty()) {
        m_uri = std::format("file://{}/{}", Aragorn::the()->project->project_dir, name);
    }
    return m_uri;
}

void semantic_tokens_response(pWidget const &widget, JSONValue const &resp)
{
    auto buffer = std::dynamic_pointer_cast<Buffer>(widget);
    auto response = Response::decode(resp);
    if (!response.has_value()) {
        std::println("Error decoding LSP response: {}", response.error().description);
        return;
    }
    if (!response.value().result.has_value()) {
        std::println("No response to textDocument/semanticTokens/full");
        return;
    }
    auto result_maybe = SemanticTokens::decode(response.value().result.value());
    if (!result_maybe.has_value()) {
        std::println("Couldn't decode response to textDocument/semanticTokens/full: {}", result_maybe.error().description);
        return;
    }
    auto        result = result_maybe.value();
    size_t      lineno { 0 };
    size_t      offset { 0 };
    auto const &data = result.data;
    size_t      token_ix { 0 };
    for (size_t ix = 0; ix < data.size(); ix += 5) {
        if (data[ix] > 0) {
            lineno += data[ix];
            if (lineno >= buffer->lines.size()) {
                break;
            }
            offset = 0;
            token_ix = 0;
        }
        offset += data[ix + 1];
        // std::println("Semantic token[{}] = (Δline {}, Δcol {}, length {} type {} {}) line {} col {}", ix, data[ix], data[ix + 1], data[ix + 2], data[ix + 3], data[ix + 4], lineno, offset);
        size_t length { data[ix + 2] };
        for (; token_ix < buffer->lines[lineno].tokens.size(); ++token_ix) {
            auto &t = buffer->lines[lineno].tokens[token_ix];
            // std::println("t.column: {} t.length: {}", t.column(), t.length());
            if (t.column() == offset && t.length() == length) {
                t.get_scope(static_cast<SemanticTokenTypes>(data[ix + 3]));
                break;
            }
        }
        if (token_ix >= buffer->lines[lineno].tokens.size()) {
            std::println("SemanticTokens OUT OF SYNC");
            break;
        }
    }
}

}
