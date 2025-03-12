/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/CompletionItem.h>
#include <LSP/Schema/InsertTextFormat.h>
#include <LSP/Schema/InsertTextMode.h>
#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/Range.h>

namespace LSP {

struct CompletionList {
    bool isIncomplete;
    struct ItemDefaults {
        std::optional<std::vector<std::string>> commitCharacters;
        struct EditRange_1 {
            Range insert;
            Range replace;

            static Decoded<EditRange_1> decode(JSONValue const &json)
            {
                EditRange_1 ret;
                ret.insert = TRY_EVAL(json.try_get<Range>("insert"));
                ret.replace = TRY_EVAL(json.try_get<Range>("replace"));
                return std::move(ret);
            }

            JSONValue encode() const
            {
                JSONValue ret;
                set(ret, "insert", insert);
                set(ret, "replace", replace);
                return ret;
            };
        };
        std::optional<std::variant<Range, EditRange_1>> editRange;
        std::optional<InsertTextFormat>                 insertTextFormat;
        std::optional<InsertTextMode>                   insertTextMode;
        std::optional<Any>                              data;

        static Decoded<ItemDefaults> decode(JSONValue const &json)
        {
            ItemDefaults ret;
            if (json.has("commitCharacters")) {
                ret.commitCharacters = TRY_EVAL(json.try_get_array<std::string>("commitCharacters"));
            }
            if (json.has("editRange")) {
                ret.editRange = TRY_EVAL(json.try_get_variant<Range, EditRange_1>("editRange"));
            }
            if (json.has("insertTextFormat")) {
                ret.insertTextFormat = TRY_EVAL(json.try_get<InsertTextFormat>("insertTextFormat"));
            }
            if (json.has("insertTextMode")) {
                ret.insertTextMode = TRY_EVAL(json.try_get<InsertTextMode>("insertTextMode"));
            }
            if (json.has("data")) {
                ret.data = TRY_EVAL(json.try_get<Any>("data"));
            }
            return std::move(ret);
        }

        JSONValue encode() const
        {
            JSONValue ret;
            set(ret, "commitCharacters", commitCharacters);
            set(ret, "editRange", editRange);
            set(ret, "insertTextFormat", insertTextFormat);
            set(ret, "insertTextMode", insertTextMode);
            set(ret, "data", data);
            return ret;
        };
    };
    std::optional<ItemDefaults> itemDefaults;
    std::vector<CompletionItem> items;

    static Decoded<CompletionList> decode(JSONValue const &json)
    {
        CompletionList ret;
        ret.isIncomplete = TRY_EVAL(json.try_get<bool>("isIncomplete"));
        if (json.has("itemDefaults")) {
            ret.itemDefaults = TRY_EVAL(json.try_get<ItemDefaults>("itemDefaults"));
        }
        ret.items = TRY_EVAL(json.try_get_array<CompletionItem>("items"));
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret;
        set(ret, "isIncomplete", isIncomplete);
        set(ret, "itemDefaults", itemDefaults);
        set(ret, "items", items);
        return ret;
    };
};

} /* namespace LSP */
