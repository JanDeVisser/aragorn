/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <filesystem>

#include <App/Modal.h>

namespace Eddy {

namespace fs = std::filesystem;

enum FileSelectorOption : uint8_t {
    FSFile = 0x01,
    FSDirectory = 0x02,
    FSShowHidden = 0x04,
    FSCreateDirectory = 0x08,
};

struct DirEntry {
    fs::directory_entry entry;

    explicit DirEntry(fs::directory_entry e)
        : entry(std::move(e))
    {
    }
    DirEntry(DirEntry const&) = default;
    DirEntry(DirEntry &&) = default;

    [[nodiscard]] fs::path const& path() const { return entry.path(); }
    auto operator<=>(DirEntry const& other) const = default;
    explicit operator std::string() const { return entry.path().string(); }
};

template <typename Submit>
struct FileSelector : public ListBox<DirEntry> {
    Submit submit_fnc;
    fs::path dir;
    FileSelectorOption options;

    FileSelector(std::string_view const& prompt, Submit const& submit, FileSelectorOption options)
        : ListBox(prompt)
        , submit_fnc(submit)
        , dir(".")
    {
        assert(submit_fnc != nullptr);
        populate();
    }

    void populate()
    {
        entries.clear();
        matches.clear();
        search = {};
        for (auto const& e : fs::directory_iterator(dir)) {
            if ((!e.is_directory() && !e.is_regular_file()) || (e.path().filename() == ".")) {
                continue;
            }
            if (e.is_directory() && !(options & FSDirectory)) {
                continue;
            }
            if (e.is_regular_file() && !(options & FSFile)) {
                continue;
            }
            auto const& name = e.path().filename().string();
            if (!(options & FSShowHidden) && name != ".." && name.starts_with(".")) {
                continue;
            }
            entries.emplace_back(name, DirEntry { e });
        }
    }

    void submit() override
    {
        fs::directory_entry const& e = entries[selection].payload.entry;
        if (e.is_directory()) {
            assert(options & FSDirectory);
            dir = dir / e.path();
            populate();
            return;
        }
        assert(e.is_regular_file());
        submit_fnc(self());
    }

    bool process_key(KeyboardModifier modifier, int key) override
    {
        if (key == KEY_LEFT) {
            dir = dir / "..";
            populate();
            return true;
        }
        if (key == KEY_RIGHT) {
            auto const& entry = entries[selection].payload.entry;
            if (entry.is_directory()) {
                dir = dir / entry.path();
                populate();
            }
            return true;
        }
        if ((options & FSCreateDirectory) && key == KEY_N && modifier == KModControl) {
            // TODO
        }
        return ListBox::process_key(key);
    }
};

}
