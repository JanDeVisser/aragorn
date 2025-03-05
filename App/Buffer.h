/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <LibCore/Result.h>

#include <App/Event.h>
#include <App/Mode.h>
#include <App/Theme.h>
#include <App/Widget.h>

namespace Aragorn {

using namespace LibCore;

struct Line {
    std::vector<DisplayToken> tokens {};

    [[nodiscard]] size_t begin() const
    {
        return tokens[0].index();
    }

    [[nodiscard]] size_t end() const
    {
        auto const &t = tokens[tokens.size() - 1];
        assert(t.kind() == TokenKind::EndOfLine || t.kind() == TokenKind::EndOfFile);
        return t.index() + t.length();
    }

    [[nodiscard]] size_t right() const
    {
        auto const &t = tokens[tokens.size() - 1];
        assert(t.kind() == TokenKind::EndOfFile || t.kind() == TokenKind::EndOfFile);
        return t.column();
    }

    [[nodiscard]] size_t length() const
    {
        return end() - begin();
    }

    [[nodiscard]] bool empty() const
    {
        return length() == 0;
    }
};

using pBuffer = std::shared_ptr<Buffer>;

using rune = char;
using rune_view = std::basic_string_view<rune>;
using rune_string = std::basic_string<rune>;

struct Buffer : public Widget {
    std::string                      name {};
    int                              buffer_ix { -1 };
    std::vector<BufferEvent>         undo_stack {};
    std::vector<Line>                lines {};
    size_t                           cursor { 0 };
    size_t                           end_gap { 0 };
    size_t                           text_size { 0 };
    size_t                           saved_version { 0 };
    size_t                           indexed_version { 0 };
    size_t                           version { 0 };
    size_t                           undo_pointer { 0 };
    pMode const                     &mode() { return m_mode; }
    std::vector<BufferEventListener> listeners {};

    explicit Buffer(pWidget const &parent);
    static Result<pBuffer, LibCError> open(std::string_view const &name);
    static pBuffer                    new_buffer();
    void                              close();
    bool                              lex();
    void                              apply(BufferEvent const &event);
    void                              edit(BufferEvent const &event);
    void                              undo();
    void                              redo();
    void                              insert(size_t pos, std::string text);
    void                              del(size_t pos, size_t count);
    void                              replace(size_t pos, size_t num, std::string replacement);
    size_t                            line_for_index(size_t index, std::optional<Vec<size_t>> const &hint = {}) const;
    Vec<size_t>                       index_to_position(size_t index, std::optional<Vec<size_t>> const &hint = {}) const;
    size_t                            position_to_index(Vec<size_t> position) const;
    void                              merge_lines(size_t top_line);
    size_t                            find(rune_view needle, size_t offset = 0);
    void                              save();
    void                              save_as(std::string_view const &new_name);
    size_t                            word_boundary_left(size_t index) const;
    size_t                            word_boundary_right(size_t index) const;
    void                              add_listener(BufferEventListener const &listener);
    std::string const                &uri();

    size_t length() const
    {
        return text_size;
    }

    bool empty()
    {
        return text_size == 0;
    }

    auto operator[](size_t ix) const
    {
        return at(ix);
    }

    void lock()
    {
        m_locked = true;
    }

    void unlock()
    {
        m_locked = false;
    }

    [[nodiscard]] rune at(size_t pos) const;
    rune_string        substr(size_t pos, size_t len = rune_view::npos);

private:
    std::vector<rune> m_text {};
    std::string       m_uri {};
    pMode             m_mode;
    bool              m_locked { false };

    void set(size_t pos);
    void insert_rune(size_t pos, rune r);
    void ensure_capacity(size_t num);
    void insert_string(size_t pos, rune_view s);
    void append_string(rune_view s);
    void erase(size_t pos, size_t len = rune_view::npos);

    auto it(size_t pos = 0)
    {
        return m_text.begin() + pos;
    }
};

// extern void          lsp_on_open(Buffer *buffer);
// extern void          lsp_did_save(Buffer *buffer);
// extern void          lsp_did_close(Buffer *buffer);
// extern void          lsp_did_change(Buffer *buffer, IntVector2 start, IntVector2 end, StringView text);
// extern void          lsp_semantic_tokens(Buffer *buffer);

}
