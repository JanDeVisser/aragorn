/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <raylib.h>
#include <string>

#include <LSP/Schema/SemanticTokenTypes.h>
#include <LibCore/JSON.h>
#include <LibCore/Token.h>

#include <App/Colour.h>

namespace Aragorn {

using namespace LibCore;

using Scope = size_t;

class Theme {
public:
    [[nodiscard]] Colours get_colours(Scope scope_id) const
    {
        assert(scope_id < m_colours.size());
        auto style = m_colours[scope_id];
        return style.colours;
    }
    Scope  get_scope(SemanticTokenTypes semanticTokenTypes);
    Scope  get_scope(std::string_view const &name);
    [[nodiscard]] Colour fg() const { return m_default_colours.fg(); }
    [[nodiscard]] Colour bg() const { return m_default_colours.bg(); }
    [[nodiscard]] Colour selection_fg() const
    {
        if (m_selection.fg() == 0u) {
            return fg();
        }
        return m_selection.fg();
    }
    [[nodiscard]] Colour selection_bg() const
    {
        if (m_selection.bg() == 0u) {
            return bg();
        }
        return m_selection.bg();
    }
    [[nodiscard]] Colours selection() const
    {
        return { selection_bg(), selection_fg() };
    }

    static Result<Theme, JSONError> load(std::string_view const &name);
    static Result<Theme, JSONError> decode(JSONValue const &json);
    static Theme                   &the();

private:
    struct ScopeStyle {
        ScopeStyle(std::string_view const &s, Colours const &colours)
            : scope(std::move(std::string(s)))
            , colours(colours)
        {
        }

        ScopeStyle(ScopeStyle const &) = default;

        std::string scope;
        Colours     colours;
    };

    std::vector<ScopeStyle>              m_colours;
    std::map<std::string_view, size_t>   m_scope_ids;
    std::map<TokenKind, size_t>          m_token_kind_to_scope_id;
    std::map<SemanticTokenTypes, size_t> m_semantic_token_type_to_scope_id;
    Colours                              m_default_colours;
    Colours                              m_selection;
};

}
