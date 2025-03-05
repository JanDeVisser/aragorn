/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <App/Widget.h>

namespace Aragorn {

struct StatusBar : public Layout {
    explicit StatusBar(pWidget const& parent);
    void initialize() override;
    void on_resize() override;
    void on_draw() override;
};

}
