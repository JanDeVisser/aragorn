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

namespace Eddy {

using namespace LibCore;

class Index {
public:
    explicit Index(size_t index_of, size_t first_token = 0)
        : m_index_of(index_of)
        , m_first_token(first_token)
    {
    }

    operator size_t() const { return m_index_of; }
    size_t length() const { return m_length; }
    size_t begin() const { return m_index_of; }
    size_t end() const { return begin() + length(); }
    auto   operator<=>(Index const &other) const { return m_index_of <=> other.m_index_of; }
    bool   contains(size_t ix) const
    {
        return ix >= begin() && ix <= end();
    }

    size_t tokens() const { return m_num_tokens; }
    size_t first_token() const { return m_first_token; }

    void extend(DisplayToken const &token)
    {
        ++m_num_tokens;
        m_length += token.length();
    }

private:
    size_t m_index_of;
    size_t m_length { 0 };
    size_t m_first_token;
    size_t m_num_tokens { 0 };
    size_t m_first_diagnostic { 0 };
    size_t m_num_diagnostics { 0 };
};

using pBuffer = std::shared_ptr<Buffer>;

struct Buffer : public Widget {
    std::string               name {};
    std::string               text {};
    int                       buffer_ix { -1 };
    std::vector<BufferEvent>  undo_stack {};
    std::vector<Index>        lines {};
    std::vector<DisplayToken> tokens {};
    size_t                    saved_version { 0 };
    size_t                    indexed_version { 0 };
    size_t                    version { 0 };
    size_t                    undo_pointer { 0 };
    //    std::vector<Diagnostic>          diagnostics;
    pMode const                     &mode() { return m_mode; }
    std::vector<BufferEventListener> listeners {};

    Buffer(pWidget const &parent);
    static Result<pBuffer, LibCError> open(std::string_view const &name);
    static pBuffer                    new_buffer();
    void                              close();
    bool                              build_indices();
    void                              apply(BufferEvent const &event);
    void                              edit(BufferEvent const &event);
    void                              undo();
    void                              redo();
    void                              insert(size_t at, std::string_view const &text);
    void                              del(size_t at, size_t count);
    void                              replace(size_t at, size_t num, std::string_view const &replacement);
    size_t                            line_for_index(size_t index) const;
    Vec<size_t>                       index_to_position(size_t index) const;
    size_t                            position_to_index(Vec<size_t> position) const;
    void                              merge_lines(size_t top_line);
    void                              save();
    void                              save_as(std::string_view const &new_name);
    size_t                            word_boundary_left(size_t index) const;
    size_t                            word_boundary_right(size_t index) const;
    void                              add_listener(BufferEventListener const &listener);
    std::string const                &uri();

    size_t length() const
    {
        return text.length();
    }

    auto operator[](size_t ix) const
    {
        return text[ix];
    }

private:
    std::string m_uri {};
    pMode       m_mode;
};

// extern void          lsp_on_open(Buffer *buffer);
// extern void          lsp_did_save(Buffer *buffer);
// extern void          lsp_did_close(Buffer *buffer);
// extern void          lsp_did_change(Buffer *buffer, IntVector2 start, IntVector2 end, StringView text);
// extern void          lsp_semantic_tokens(Buffer *buffer);

}
