#include "button.h"
#include "draw.h"
#include "widget.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct wb_button {
    wb_widget_t parent;

    bool pressed;
    bool hover;
    bool dirty;
    wb_button_on_click_t *on_click;
    void *data;
    char title[WB_BUTTON_TITLE_SIZE];
    int x;
    int y;
    int width;
    int height;
} wb_button_t;


static wb_color_t
get_color(const wb_button_t *button)
{
    assert(button);

    if (button->pressed) {
        return WB_BUTTON_PRESSED_BG_COLOR;
    }
    if (button->hover) {
        return WB_BUTTON_HOVER_BG_COLOR;
    }
    return WB_BUTTON_BG_COLOR;
}


static void
draw(const wb_widget_t *widget, cairo_t *cr)
{
    assert(widget);
    wb_button_t *button = WB_BUTTON(widget);
    if (button->width == 0 || button->height == 0) {
        return;
    }

    wb_color_t bg_color = get_color(button);
    wb_draw_text_in_box(cr, button->x, button->y, button->width, button->height, button->title,
                        &bg_color, &WB_BUTTON_BORDER_COLOR, WB_BUTTON_BORDER_WIDTH);
    button->dirty = false;
}


static void
mouse_move(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_button_t *button = WB_BUTTON(widget);

    bool inside = (x >= button->x && x <= button->x + button->width) &&
                  (y >= button->y && y <= button->y + button->height);
    if (inside) {
        if (button->hover == false) {
            button->hover = true;
            button->dirty = true;
        }
    } else {
        if (button->hover) {
            button->hover = false;
            button->dirty = true;
        }
    }
}


static void
mouse_leave(wb_widget_t *widget)
{
    assert(widget);
    wb_button_t *button = WB_BUTTON(widget);

    if (button->hover) {
        button->hover = false;
        button->dirty = true;
    }
}


static void
mouse_press(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_button_t *button = WB_BUTTON(widget);

    if ((x >= button->x && x <= button->x + button->width) &&
        (y >= button->y && y <= button->y + button->height)) {
        if (!button->pressed) {
            button->pressed = true;
            button->dirty = true;
        }
    }
}


static void
mouse_release(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_button_t *button = WB_BUTTON(widget);

    bool inside = (x >= button->x && x <= button->x + button->width) &&
                  (y >= button->y && y <= button->y + button->height);
    if (button->pressed) {
        if (inside) {
            if (button->on_click) {
                button->on_click(widget, button->data);
            }
        }
        button->pressed = false;
        button->dirty = true;
    }
}


static void
set_geometry(wb_widget_t *widget, int x, int y, int width, int height)
{
    assert(widget);
    wb_button_t *button = WB_BUTTON(widget);

    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->dirty = true;
}


static wb_rect_t
get_geometry(wb_widget_t *widget)
{
    assert(widget);
    wb_button_t *button = WB_BUTTON(widget);
    return (wb_rect_t){
        .x = button->x,
        .y = button->y,
        .width = button->width,
        .height = button->height,
    };
}


static bool
get_dirty(wb_widget_t *widget)
{
    assert(widget);
    wb_button_t *button = WB_BUTTON(widget);
    return button->dirty;
}


static void
deinit(wb_widget_t *widget)
{
    assert(widget);
    wb_button_t *button = WB_BUTTON(widget);
    free(button);
}


wb_widget_t *
wb_button_init(const char *title, wb_button_on_click_t on_click_cb, void *data)
{
    wb_button_t *button = calloc(1, sizeof(wb_button_t));
    assert(button);

    button->on_click = on_click_cb;
    button->data = data;
    strncpy(button->title, title, WB_BUTTON_TITLE_SIZE - 1);
    button->title[WB_BUTTON_TITLE_SIZE - 1] = '\0';
    button->dirty = true;

    wb_widget_t *widget = WB_WIDGET(button);
    widget->draw = draw;
    widget->mouse_move = mouse_move;
    widget->mouse_leave = mouse_leave;
    widget->mouse_press = mouse_press;
    widget->mouse_release = mouse_release;
    widget->set_geometry = set_geometry;
    widget->get_geometry = get_geometry;
    widget->get_dirty = get_dirty;
    widget->deinit = deinit;

    return widget;
}
