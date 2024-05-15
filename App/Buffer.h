/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <LibCore/Result.h>

#include <App/Event.h>
#include <App/Widget.h>
// #include <app/event.h>
// #include <app/mode.h>
// #include <base/sv.h>
// #include <base/token.h>
// #include <lsp/lsp.h>
// #include <lsp/schema/Diagnostic.h>

namespace Eddy {

using namespace LibCore;

struct DisplayToken {
    size_t index;
    size_t length;
    size_t line;
    Color  color;
};

struct Index {
    size_t           index_of;
    std::string_view line;
    size_t           first_token;
    size_t           num_tokens;
    size_t           first_diagnostic;
    size_t           num_diagnostics;
};

using pBuffer = std::shared_ptr<Buffer>;

struct Buffer : public Widget {
    std::string               name;
    std::string               text;
    int                       buffer_ix;
    std::vector<BufferEvent>  undo_stack;
    std::vector<Index>        lines;
    std::vector<DisplayToken> tokens;
    size_t                    saved_version;
    size_t                    indexed_version;
    size_t                    version;
    size_t                    undo_pointer;
    //    std::vector<Diagnostic>          diagnostics;
    //    Mode                            *mode;
    std::vector<BufferEventListener> listeners;

    Buffer();
    static Result<pBuffer, LibCError> open(std::string_view const& name);
    static pBuffer                    new_buffer();
    void                              close();
    void                              build_indices();
    void                              apply(BufferEvent const& event);
    void                              edit(BufferEvent const& event);
    void                              undo();
    void                              redo();
    void                              insert(size_t at, std::string_view const &text);
    void                              del(size_t at, size_t count);
    void                              replace(size_t at, size_t num, std::string_view const &replacement);
    size_t                            line_for_index(int index);
    Vec<int>                          index_to_position(int index);
    size_t                            position_to_index(Vec<int> position);
    void                              merge_lines(int top_line);
    void                              save();
    void                              save_as(std::string_view const &new_name);
    size_t                            word_boundary_left(size_t index);
    size_t                            word_boundary_right(size_t index);
    void                              add_listener(BufferEventListener const &listener);
    std::string const                &uri();

private:
    std::string m_uri {};
} Buffer;

// extern void          lsp_on_open(Buffer *buffer);
// extern void          lsp_did_save(Buffer *buffer);
// extern void          lsp_did_close(Buffer *buffer);
// extern void          lsp_did_change(Buffer *buffer, IntVector2 start, IntVector2 end, StringView text);
// extern void          lsp_semantic_tokens(Buffer *buffer);

}
