/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <filesystem>

#include <App/Modal.h>

namespace Aragorn {

namespace fs = std::filesystem;

enum FileSelectorOption : uint8_t {
    FSFile = 0x01,
    FSDirectory = 0x02,
    FSShowHidden = 0x04,
    FSCreateDirectory = 0x08,
};

struct FileSelector : public ListBox<fs::directory_entry> {
    using Submit = std::function<void(pWidget const &)>;
    Submit             submit_fnc;
    fs::path           dir;
    FileSelectorOption options;

    FileSelector(std::string_view const &prompt, Submit const &submit, FileSelectorOption options)
        : ListBox(prompt)
        , submit_fnc(submit)
        , dir(".")
        , options(options)
    {
        assert(submit_fnc != nullptr);
        populate();
    }

    void populate()
    {
        entries.clear();
        matches.clear();
        search = {};
        for (auto const &e : fs::directory_iterator(dir)) {
            auto const &name = e.path().filename().string();
            if ((!e.is_directory() && !e.is_regular_file()) || (name == ".")) {
                continue;
            }
            if (e.is_directory() && !(options & FSDirectory)) {
                continue;
            }
            if (e.is_regular_file() && !(options & FSFile)) {
                continue;
            }
            if (!(options & FSShowHidden) && name != ".." && name.starts_with(".")) {
                continue;
            }
            entries.emplace_back(name, e);
        }
    }

    void submit() override
    {
        auto const &e = entries[selection].payload;
        if (e.is_directory()) {
            assert(options & FSDirectory);
            dir = dir / e.path();
            populate();
	    status = ModalStatus::Active;
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
            auto const &entry = entries[selection].payload;
            if (entry.is_directory()) {
                dir = dir / entry.path();
                populate();
            }
            return true;
        }
        if ((options & FSCreateDirectory) && key == KEY_N && modifier == KModControl) {
            // TODO
        }
        return ListBox::process_key(modifier, key);
    }
};

}
