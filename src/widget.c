#include "widget.h"
#include "draw.h"

#include <assert.h>


void
wb_widget_draw(const wb_widget_t *widget, cairo_t *cr)
{
    assert(widget);
    widget->draw(widget, cr);
}


void
wb_widget_mouse_move(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    if (widget->mouse_move)
        widget->mouse_move(widget, x, y);
}


void
wb_widget_mouse_leave(wb_widget_t *widget)
{
    assert(widget);
    if (widget->mouse_leave)
        widget->mouse_leave(widget);
}


void
wb_widget_mouse_press(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    if (widget->mouse_press)
        widget->mouse_press(widget, x, y);
}


void
wb_widget_mouse_release(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    if (widget->mouse_release)
        widget->mouse_release(widget, x, y);
}


void
wb_widget_set_geometry(wb_widget_t *widget, int x, int y, int width, int height)
{
    assert(widget);
    widget->set_geometry(widget, x, y, width, height);
}

wb_rect_t
wb_widget_get_geometry(wb_widget_t *widget)
{
    assert(widget);
    return widget->get_geometry(widget);
}

bool
wb_widget_get_dirty(wb_widget_t *widget)
{
    assert(widget);
    return widget->get_dirty(widget);
}


void
wb_widget_deinit(wb_widget_t *widget)
{
    assert(widget);
    widget->deinit(widget);
}
