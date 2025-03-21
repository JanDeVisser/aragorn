/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <filesystem>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>

#include <LibCore/IO.h>

#include <App/Aragorn.h>

namespace Aragorn {

namespace fs = std::filesystem;

// Result<pProject, AragornError> Project::create()

Result<pProject, AragornError> Project::open(pAragorn const &aragorn, std::string_view const &dir)
{
    fs::path d { fs::canonical(dir) };
    if (!fs::exists(d) && mkdir(d.c_str(), 0700) < 0) {
        return AragornError { LibCError() };
    }
    if (chdir(d.c_str()) != 0) {
        return AragornError { LibCError() };
    }

    pProject ret = Widget::make<Project>(aragorn);
    aragorn->project = ret;
    ret->project_dir = d.string();
    auto dot_aragorn = d / ".aragorn";
    if (!fs::exists(dot_aragorn) && mkdir(dot_aragorn.c_str(), 0700) < 0) {
        return AragornError { LibCError() };
    }

    JSONValue prj = JSONValue::object();
    auto      prj_file = dot_aragorn / "project.json";
    if (fs::exists(prj_file)) {
        auto json_text_maybe = read_file_by_name(prj_file.string());
        if (json_text_maybe.is_error()) {
            return AragornError { json_text_maybe.error() };
        }
        auto json_maybe = JSONValue::deserialize(json_text_maybe.value());
        if (json_maybe.is_error()) {
            return AragornError { json_maybe.error() };
        }
        prj.merge(json_maybe.value());
    }
    if (auto sources = prj.try_get_array<std::string>("sources"); sources.has_value()) {
        auto srcs = sources.value();
        for (auto const &dir : srcs) {
            ret->source_dirs.emplace_back(dir);
        }
    }
    if (ret->source_dirs.empty()) {
        ret->source_dirs.emplace_back(".");
    }
    if (prj.has("cmake")) {
        auto &cmake = prj["cmake"];
        if (!cmake.is_object()) {
            return AragornError { SettingsError { "'cmake' section must be an object" } };
        }
        if (cmake.has("cmakelists")) {
            auto &cmakelists = cmake["cmakelists"];
            if (cmakelists.is_string()) {
                ret->cmake.cmakelists = cmakelists.to_string();
            }
            auto &build_dir = cmake["build_dir"];
            if (build_dir.is_string()) {
                ret->cmake.build_dir = build_dir.to_string();
            }
        }
    }

    auto state_file = dot_aragorn / "state.json";
    if (fs::exists(state_file) && fs::is_regular_file(state_file)) {
        auto json_text_maybe = read_file_by_name(state_file.string());
        if (json_text_maybe.is_error()) {
            return AragornError { json_text_maybe.error() };
        }
        auto json_maybe = JSONValue::deserialize(json_text_maybe.value());
        if (json_maybe.is_error()) {
            return AragornError { json_maybe.error() };
        }
        auto const              &state = json_maybe.value();
        std::vector<std::string> files;
        if (!state["files"].convert(files).is_error()) {
            for (auto const &f : files) {
                auto const result = aragorn->open_buffer(f);
                if (result.is_error()) {
                    return AragornError { result.error() };
                }
            }
        }
    }
    return ret;
}

void Project::close(pAragorn const &aragorn) const
{
    StringList buffer_names {};
    for (auto const &b : aragorn->buffers) {
        if (!b->name.empty()) {
            buffer_names.push_back(b->name);
        }
    }
    while (!aragorn->buffers.empty()) {
        aragorn->close_buffer(0);
    }
    JSONValue state = JSONValue::object();
    JSONValue files = JSONValue::array();
    for (auto const &n : buffer_names) {
        files.append(JSONValue(n));
    }
    state.set("files", files);
    fs::path state_file { project_dir };
    state_file /= ".aragorn";
    state_file /= "state.json";
    auto state_str = state.serialize();
    auto res = write_file_by_name(state_file.string(), state_str);
    if (res.is_error()) {
        warning(Project, "Error writing project state: {}", res.error().to_string());
        warning(Project, "State:\n{}", state_str);
    }
}

}

extern "C" {

void sigchld(int)
{
}
}
