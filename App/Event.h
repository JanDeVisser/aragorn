/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <variant>

#include <App/Widget.h>

namespace Eddy {

enum class BufferEventType {
    None = 0,
    CursorMove,
    Insert,
    Delete,
    Replace,
    Indexed,
    Save,
    Close,
};

struct EventRange {
    Vec<int> start {};
    Vec<int> end {};
};

struct Buffer;

struct BufferEvent {
    struct Replacement {
        std::string overwritten {};
        std::string replacement {};
    };

    using Change = std::variant<std::string, Replacement, std::optional<std::string>>;

    BufferEventType type { BufferEventType::None };
    size_t          position { 0 };
    EventRange      range {};
    Change          change {};

    [[nodiscard]] BufferEvent revert() const
    {
        BufferEvent ret {};
        switch (type) {
        case BufferEventType::Insert: {
            ret.type = BufferEventType::Delete;
            ret.position = position;
            auto const &insert = std::get<std::string>(change);
            ret.change = std::string { insert };
        } break;
        case BufferEventType::Delete:
            ret.type = BufferEventType::Insert;
            ret.position = position;
            ret.change = change;
            break;
        case BufferEventType::Replace: {
            ret.type = BufferEventType::Replace;
            ret.position = position;
            auto const &replace = std::get<Replacement>(change);
            ret.change = Replacement {
                .overwritten = replace.replacement,
                .replacement = replace.overwritten
            };
        } break;
        default:
            break;
        }
        return ret;
    }

    [[nodiscard]] std::string const &insert() const
    {
        assert(type == BufferEventType::Insert);
        return std::get<std::string>(change);
    }

    [[nodiscard]] std::string const &deletion() const
    {
        assert(type == BufferEventType::Delete);
        return std::get<std::string>(change);
    }

    [[nodiscard]] Replacement const &replacement() const
    {
        assert(type == BufferEventType::Replace);
        return std::get<Replacement>(change);
    }

    [[nodiscard]] std::optional<std::string> const &filename() const
    {
        assert(type == BufferEventType::Save);
        return std::get<std::optional<std::string>>(change);
    }

    static BufferEvent make_insert(EventRange const &range, size_t at, std::string_view const &text)
    {
        BufferEvent ret;
        ret.type = BufferEventType::Insert;
        ret.range = range;
        ret.position = at;
        ret.change = std::string { text };
        return ret;
    }

    static BufferEvent make_delete(EventRange const &range, size_t at, std::string_view const &deletion)
    {
        BufferEvent ret;
        ret.type = BufferEventType::Delete;
        ret.range = range;
        ret.position = at;
        ret.change = std::string { deletion };
        return ret;
    }

    static BufferEvent make_replacement(EventRange const &range, size_t at, std::string_view const &overwritten, std::string_view const &replacement)
    {
        BufferEvent ret;
        ret.type = BufferEventType::Delete;
        ret.range = range;
        ret.position = at;
        ret.change = Replacement {
            std::string { overwritten },
            std::string { replacement }
        };
        return ret;
    }

    static BufferEvent make_save()
    {
        BufferEvent ret;
        ret.type = BufferEventType::Save;
        ret.change = std::optional<std::string> {};
        return ret;
    }

    static BufferEvent make_save_as(std::string_view const &file_name)
    {
        BufferEvent ret;
        ret.type = BufferEventType::Save;
        ret.change = std::optional<std::string> { file_name };
        return ret;
    }
};

using BufferEventListener = std::function<void(std::shared_ptr<Buffer>, BufferEvent const &)>;

}
