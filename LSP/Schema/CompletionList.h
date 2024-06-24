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

struct CompletionList : public LSPObject {
    bool isIncomplete;
    struct ItemDefaults : public LSPObject {
        std::optional<std::vector<std::string>> commitCharacters;
        struct EditRange_1 : public LSPObject {
            Range insert;
            Range replace;

            static Result<EditRange_1, JSONError> decode(JSONValue const &json)
            {
                EditRange_1 ret;
                ret.insert = TRY_EVAL(json.try_get<Range>(insert));
                ret.replace = TRY_EVAL(json.try_get<Range>(replace));
                return ret;
            }

            JSONValue encode()
            {
                set(ret, "insert", encode<Range>(insert));
                set(ret, "replace", encode<Range>(replace));
                JSONValue ret;
            };
        };
        std::optional<std::variant<Range, EditRange_1>> editRange;
        std::optional<InsertTextFormat>                 insertTextFormat;
        std::optional<InsertTextMode>                   insertTextMode;
        std::optional<LSP::Any>                         data;

        static Result<ItemDefaults, JSONError> decode(JSONValue const &json)
        {
            ItemDefaults ret;
        if (json.has("commitCharacters") {
                ret.commitCharacters = TRY_EVAL(json.try_get<std::vector<std::string>>(commitCharacters));
        }
        if (json.has("editRange") {
                ret.editRange = TRY_EVAL(json.try_get<std::variant<Range, EditRange_1>>(editRange));
        }
        if (json.has("insertTextFormat") {
                ret.insertTextFormat = TRY_EVAL(json.try_get<InsertTextFormat>(insertTextFormat));
        }
        if (json.has("insertTextMode") {
                ret.insertTextMode = TRY_EVAL(json.try_get<InsertTextMode>(insertTextMode));
        }
        if (json.has("data") {
                ret.data = TRY_EVAL(json.try_get<LSP::Any>(data));
        }
         return ret;
        }

        JSONValue encode()
        {
            set(ret, "commitCharacters", encode<std::optional<std::vector<std::string>>>(commitCharacters));
            set(ret, "editRange", encode<std::optional<std::variant<Range, EditRange_1>>>(editRange));
            set(ret, "insertTextFormat", encode<std::optional<InsertTextFormat>>(insertTextFormat));
            set(ret, "insertTextMode", encode<std::optional<InsertTextMode>>(insertTextMode));
            set(ret, "data", encode<std::optional<LSP::Any>>(data));
            JSONValue ret;
        };
    };
    std::optional<ItemDefaults> itemDefaults;
    std::vector<CompletionItem> items;

    static Result<CompletionList, JSONError> decode(JSONValue const &json)
    {
        CompletionList ret;
        ret.isIncomplete = TRY_EVAL(json.try_get<bool>(isIncomplete));
        if (json.has("itemDefaults") {
            ret.itemDefaults = TRY_EVAL(json.try_get<ItemDefaults>(itemDefaults));
        }
        ret.items = TRY_EVAL(json.try_get<std::vector<CompletionItem>>(items));
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "isIncomplete", encode<bool>(isIncomplete));
        set(ret, "itemDefaults", encode<std::optional<ItemDefaults>>(itemDefaults));
        set(ret, "items", encode<std::vector<CompletionItem>>(items));
        JSONValue ret;
    };
};

} /* namespace LSP */
