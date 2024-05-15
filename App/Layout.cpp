/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <App/App.h>

namespace Eddy {

void Layout::resize()
{
    // printf("Resizing layout %s %s\n", layout->classname, rect_tostring(layout->viewport));
    on_resize();
    float allocated = 0.0f;
    int   stretch_count = 0;
    int   fixed_coord = (orientation == ContainerOrientation::Vertical) ? 0 : 1;
    int   var_coord = 1 - fixed_coord;
    float total = viewport.size.coords[var_coord];
    float fixed_size = viewport.size.coords[fixed_coord];
    float fixed_pos = viewport.position.coords[fixed_coord];
    float var_offset = viewport.position.coords[var_coord];

    // printf("Total available %f, laying out %s\n", total, (layout->orientation == CO_VERTICAL) ? "vertically" : "horizontally");
    // printf("Fixed %s: %f, fixed %s position: %f\n",
    //     (layout->orientation == CO_VERTICAL) ? "width" : "height",
    //     fixed_size,
    //     (layout->orientation == CO_VERTICAL) ? "x" : "y",
    //     fixed_pos);
    for (auto &w : widgets) {
        w->viewport.size.coords[fixed_coord] = fixed_size - w->padding.coords[fixed_coord] - w->padding.coords[fixed_coord + 2];
        w->viewport.position.coords[fixed_coord] = fixed_pos + w->padding.coords[fixed_coord];
        float sz = 0;
        // printf("Component widget %s has policy %s\n", w->classname, SizePolicy_name(w->policy));
        switch (w->policy) {
        case SizePolicy::Absolute:
            sz = w->policy_size;
            break;
        case SizePolicy::Relative: {
            sz = (total * w->policy_size) / 100.0f;
        } break;
        case SizePolicy::Characters: {
            sz = ceilf(1.2 * w->policy_size * ((orientation == ContainerOrientation::Vertical) ? app->cell.y : app->cell.x));
        } break;
        case SizePolicy::Calculated: {
            fatal("SP_CALCULATED not yet supported");
        };
        case SizePolicy::Stretch: {
            sz = -1.0f;
            stretch_count++;
        } break;
        }
        assert(sz != 0);
        w->viewport.size.coords[var_coord] = sz - w->padding.coords[var_coord] - w->padding.coords[var_coord + 2];
        if (sz > 0) {
            allocated += sz;
            // printf("Allocating %f, now allocated %f\n", sz, allocated);
        }
    }

    if (stretch_count) {
        // printf("Stretch count %d\n", stretch_count);
        assert(total > allocated);
        float stretch = floorf((total - allocated) / (float) stretch_count);
        for (auto &w : widgets) {
            if (w->policy == SizePolicy::Stretch) {
                // printf("Allocating %f to stretchable %s\n", stretch, w->classname);
                w->viewport.size.coords[var_coord] = stretch - w->padding.coords[var_coord] - w->padding.coords[var_coord + 2];
            }
        }
    }

    for (auto &w : widgets) {
        w->viewport.position.coords[var_coord] = var_offset + w->padding.coords[var_coord];
        var_offset += w->viewport.size.coords[var_coord] + w->padding.coords[var_coord] + w->padding.coords[var_coord + 2];
        // printf("Resizing %s to %s\n", w->classname, rect_tostring(w->viewport));
        w->resize();
    }
    after_resize();
}

void Layout::draw()
{
    on_draw();
    for (auto &w : widgets) {
        if (w->viewport.width > 0.0f && w->viewport.height > 0.0f) {
            DrawRectangle(w->viewport.x - w->padding.left, w->viewport.y - w->padding.top,
                w->viewport.width + w->padding.left + w->padding.right,
                w->viewport.height + w->padding.top + w->padding.bottom,
                w->background);
            w->draw();
        }
    }
    after_draw();
}

void Layout::process_input()
{
    on_process_input();
    for (auto &w : widgets) {
        if (w->viewport.width > 0.0f && w->viewport.height > 0.0f) {
            w->process_input();
        }
    }
    after_process_input();
}

void Layout::dump()
{
    int  dump_indent = 0;
    auto dump_fnc = [&dump_indent](pWidget const &w) {
        if (w == nullptr) {
            dump_indent -= 2;
            return;
        }
        if (auto layout = std::dynamic_pointer_cast<Layout>(w); layout) {
            std::cout << std::format("{:{}s}+ | {:} {:}", "", dump_indent, layout->widgets.size(), w->viewport.to_string()) << std::endl;
            dump_indent += 2;
            return;
        }
        std::cout << std::format("{:{}s}+-> {:} {:}\n", "", dump_indent, typeid(w).name(), w->viewport.to_string()) << std::endl;
    };
    std::string_view s;
    traverse(dump_fnc);
}

}
