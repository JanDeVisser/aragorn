/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <App/Widget.h>

namespace Eddy {

struct StatusBar : public Layout {
    StatusBar();
    void on_draw() override;
};

}
