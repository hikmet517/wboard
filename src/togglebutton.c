#include "togglebutton.h"
#include "button.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct wb_togglebutton {
    wb_widget_t parent;

    bool pressed;
    bool hover;
    bool active;
    bool dirty;
    wb_togglebutton_on_change_t *on_change;
    void *data;
    char title[WB_BUTTON_TITLE_SIZE];
    int x;
    int y;
    int width;
    int height;
} wb_togglebutton_t;


static wb_color_t
get_color(const wb_togglebutton_t *togglebutton)
{
    assert(togglebutton);
    if (togglebutton->pressed || togglebutton->active) {
        return WB_BUTTON_PRESSED_BG_COLOR;
    }
    if (togglebutton->hover) {
        return WB_BUTTON_HOVER_BG_COLOR;
    }
    return WB_BUTTON_BG_COLOR;
}


static void
draw(const wb_widget_t *widget, cairo_t *cr)
{
    assert(widget);
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    if (togglebutton->width == 0 || togglebutton->height == 0) {
        return;
    }

    wb_color_t bg_color = get_color(togglebutton);
    wb_draw_text_in_box(cr, togglebutton->x, togglebutton->y, togglebutton->width,
                        togglebutton->height, togglebutton->title, &bg_color,
                        &WB_BUTTON_BORDER_COLOR, WB_BUTTON_BORDER_WIDTH);
    togglebutton->dirty = false;
}


static void
mouse_move(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    bool inside = (x >= togglebutton->x && x <= togglebutton->x + togglebutton->width) &&
                  (y >= togglebutton->y && y <= togglebutton->y + togglebutton->height);
    if (inside) {
        if (togglebutton->hover == false) {
            togglebutton->hover = true;
            togglebutton->dirty = true;
        }
    } else {
        if (togglebutton->hover) {
            togglebutton->hover = false;
            togglebutton->dirty = true;
        }
    }
}


static void
mouse_leave(wb_widget_t *widget)
{
    assert(widget);
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    if (togglebutton->hover) {
        togglebutton->hover = false;
        togglebutton->dirty = true;
    }
}


static void
mouse_press(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    bool inside = (x >= togglebutton->x && x <= togglebutton->x + togglebutton->width) &&
                  (y >= togglebutton->y && y <= togglebutton->y + togglebutton->height);
    if (inside) {
        if (!togglebutton->pressed) {
            togglebutton->pressed = true;
            togglebutton->dirty = true;
        }
    }
}


static void
mouse_release(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    bool inside = (x >= togglebutton->x && x <= togglebutton->x + togglebutton->width) &&
                  (y >= togglebutton->y && y <= togglebutton->y + togglebutton->height);
    if (togglebutton->pressed) {
        if (inside) {
            wb_togglebutton_set_active(togglebutton, !wb_togglebutton_get_active(togglebutton));
        }
        togglebutton->pressed = false;
        togglebutton->dirty = true;
    }
}


static void
set_geometry(wb_widget_t *widget, int x, int y, int width, int height)
{
    assert(widget);
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);

    togglebutton->x = x;
    togglebutton->y = y;
    togglebutton->width = width;
    togglebutton->height = height;
    togglebutton->dirty = true;
}


static wb_rect_t
get_geometry(wb_widget_t *widget)
{
    assert(widget);
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    return (wb_rect_t){
        .x = togglebutton->x,
        .y = togglebutton->y,
        .width = togglebutton->width,
        .height = togglebutton->height,
    };
}


static bool
get_dirty(wb_widget_t *widget)
{
    assert(widget);
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    return togglebutton->dirty;
}


static void
deinit(wb_widget_t *widget)
{
    assert(widget);
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    free(togglebutton);
}


wb_widget_t *
wb_togglebutton_init(const char *title, wb_togglebutton_on_change_t on_change_cb, void *data)
{
    wb_togglebutton_t *togglebutton = calloc(1, sizeof(wb_togglebutton_t));
    assert(togglebutton);

    togglebutton->on_change = on_change_cb;
    togglebutton->data = data;
    strncpy(togglebutton->title, title, WB_BUTTON_TITLE_SIZE - 1);
    togglebutton->title[WB_BUTTON_TITLE_SIZE - 1] = '\0';
    togglebutton->dirty = true;

    wb_widget_t *widget = WB_WIDGET(togglebutton);
    widget->draw = draw;
    widget->mouse_move = mouse_move;
    widget->mouse_leave = mouse_leave;
    widget->mouse_press = mouse_press;
    widget->mouse_release = mouse_release;
    widget->deinit = deinit;
    widget->get_geometry = get_geometry;
    widget->set_geometry = set_geometry;
    widget->get_dirty = get_dirty;

    return widget;
}


bool
wb_togglebutton_get_active(wb_togglebutton_t *togglebutton)
{
    assert(togglebutton);
    return togglebutton->active;
}


void
wb_togglebutton_set_active(wb_togglebutton_t *togglebutton, bool active)
{
    togglebutton->active = active;
    if (togglebutton->on_change) {
        togglebutton->on_change(WB_WIDGET(togglebutton), togglebutton->data);
    }
    togglebutton->pressed = !togglebutton->pressed;
    togglebutton->dirty = true;
}
