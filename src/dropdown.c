#include "dropdown.h"
#include "button.h"
#include "draw.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct item_state {
    bool pressed;
    bool hover;
} item_state_t;


typedef struct wb_dropdown {
    wb_widget_t parent;

    char items[WB_DROPDOWN_MAX_ITEMS][WB_BUTTON_TITLE_SIZE];
    wb_widget_t *item_buttons[WB_DROPDOWN_MAX_ITEMS];
    bool pressed;
    bool hover;
    bool dirty;
    bool open;
    int selected;
    wb_dropdown_on_select_t *on_select;
    void *data;
    int x;
    int y;
    int width;
    int height;
} wb_dropdown_t;


static void
item_on_click(wb_widget_t *widget, void *data)
{
    (void)widget;
    wb_dropdown_t *dropdown = data;
    if (dropdown->on_select) {
        dropdown->on_select(WB_WIDGET(dropdown), dropdown->data);
    }
    if (dropdown->open) {
        dropdown->open = false;
        dropdown->pressed = false;
        dropdown->dirty = true;
    }
}

static wb_color_t
get_color(const wb_dropdown_t *dropdown)
{
    assert(dropdown);

    if (dropdown->pressed) {
        return WB_BUTTON_PRESSED_BG_COLOR;
    }
    if (dropdown->hover) {
        return WB_BUTTON_HOVER_BG_COLOR;
    }
    return WB_BUTTON_BG_COLOR;
}


static void
draw(const wb_widget_t *widget, cairo_t *cr)
{
    assert(widget);
    wb_dropdown_t *dropdown = WB_DROPDOWN(widget);
    if (dropdown->width == 0 || dropdown->height == 0) {
        return;
    }

    wb_color_t bg_color = get_color(dropdown);

    char title[WB_BUTTON_TITLE_SIZE] = {0};
    if (dropdown->selected != -1) {
        strncpy(title, dropdown->items[dropdown->selected], WB_BUTTON_TITLE_SIZE - 1);
    }

    int box_width = 9 * dropdown->height / 10;
    wb_draw_text_in_box(cr, dropdown->x, dropdown->y,
                        dropdown->width - box_width + WB_BUTTON_BORDER_WIDTH, dropdown->height,
                        title, &bg_color, &WB_BUTTON_BORDER_COLOR, WB_BUTTON_BORDER_WIDTH);
    wb_draw_text_in_box(cr, dropdown->x + dropdown->width - box_width, dropdown->y, box_width,
                        dropdown->height, "▼", &bg_color, &WB_BUTTON_BORDER_COLOR,
                        WB_BUTTON_BORDER_WIDTH);

    if (dropdown->open) {
        for (int i = 0; i < WB_DROPDOWN_MAX_ITEMS && *dropdown->items[i]; i++) {
            wb_widget_draw(dropdown->item_buttons[i], cr);
        }
    }

    dropdown->dirty = false;
}


static void
mouse_move(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_dropdown_t *dropdown = WB_DROPDOWN(widget);

    bool inside = (x >= dropdown->x && x <= dropdown->x + dropdown->width) &&
                  (y >= dropdown->y && y <= dropdown->y + dropdown->height);
    if (inside) {
        if (dropdown->hover == false) {
            dropdown->hover = true;
            dropdown->dirty = true;
        }
    } else {
        if (dropdown->hover) {
            dropdown->hover = false;
            dropdown->dirty = true;
        }
    }

    if (dropdown->open) {
        for (int i = 0; i < WB_DROPDOWN_MAX_ITEMS && *dropdown->items[i]; i++) {
            wb_widget_mouse_move(dropdown->item_buttons[i], x, y);
            dropdown->dirty |= wb_widget_get_dirty(dropdown->item_buttons[i]);
        }
    }
}


static void
mouse_leave(wb_widget_t *widget)
{
    assert(widget);
    wb_dropdown_t *dropdown = WB_DROPDOWN(widget);

    if (dropdown->hover) {
        dropdown->hover = false;
        dropdown->dirty = true;
    }

    if (dropdown->open) {
        for (int i = 0; i < WB_DROPDOWN_MAX_ITEMS && *dropdown->items[i]; i++) {
            wb_widget_mouse_leave(dropdown->item_buttons[i]);
            dropdown->dirty |= wb_widget_get_dirty(dropdown->item_buttons[i]);
        }
    }
}


static void
mouse_press(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_dropdown_t *dropdown = WB_DROPDOWN(widget);

    bool inside = (x >= dropdown->x && x <= dropdown->x + dropdown->width) &&
                  (y >= dropdown->y && y <= dropdown->y + dropdown->height);
    if (inside) {
        if (!dropdown->pressed) {
            dropdown->pressed = true;
            dropdown->dirty = true;
        }
    }

    if (dropdown->open) {
        for (int i = 0; i < WB_DROPDOWN_MAX_ITEMS && *dropdown->items[i]; i++) {
            wb_widget_mouse_press(dropdown->item_buttons[i], x, y);
            dropdown->dirty |= wb_widget_get_dirty(dropdown->item_buttons[i]);
        }
    }
}


static void
mouse_release(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_dropdown_t *dropdown = WB_DROPDOWN(widget);

    bool inside = (x >= dropdown->x && x <= dropdown->x + dropdown->width) &&
                  (y >= dropdown->y && y <= dropdown->y + dropdown->height);
    if (inside) {
        if (dropdown->pressed) {
            dropdown->open = !dropdown->open;
            if (dropdown->open == false) {
                dropdown->pressed = false;
            }
            dropdown->dirty = true;
        }
    } else {
        if (dropdown->open) {
            for (int i = 0; i < WB_DROPDOWN_MAX_ITEMS && *dropdown->items[i]; i++) {
                wb_rect_t rect = wb_widget_get_geometry(dropdown->item_buttons[i]);
                bool inside_item = (x >= rect.x && x <= rect.x + rect.width) &&
                                   (y >= rect.y && y <= rect.y + rect.width);
                if (inside_item) {
                    dropdown->selected = i;
                }
                wb_widget_mouse_release(dropdown->item_buttons[i], x, y);
                dropdown->dirty |= wb_widget_get_dirty(dropdown->item_buttons[i]);
            }
        }
    }
}


static void
set_geometry(wb_widget_t *widget, int x, int y, int width, int height)
{
    assert(widget);
    wb_dropdown_t *dropdown = WB_DROPDOWN(widget);

    dropdown->x = x;
    dropdown->y = y;
    dropdown->width = width;
    dropdown->height = height;
    int item_height = 9 * dropdown->height / 10;
    for (int i = 0; i < WB_DROPDOWN_MAX_ITEMS && *dropdown->items[i]; i++) {
        wb_widget_set_geometry(dropdown->item_buttons[i], dropdown->x,
                               dropdown->y + dropdown->height - WB_BUTTON_BORDER_WIDTH +
                                   i * (item_height - WB_BUTTON_BORDER_WIDTH),
                               dropdown->width, item_height);
    }
    dropdown->dirty = true;
}


static wb_rect_t
get_geometry(wb_widget_t *widget)
{
    assert(widget);
    wb_dropdown_t *dropdown = WB_DROPDOWN(widget);
    return (wb_rect_t){
        .x = dropdown->x,
        .y = dropdown->y,
        .width = dropdown->width,
        .height = dropdown->height,
    };
}


static bool
get_dirty(wb_widget_t *widget)
{
    assert(widget);
    wb_dropdown_t *dropdown = WB_DROPDOWN(widget);
    return dropdown->dirty;
}


static void
deinit(wb_widget_t *widget)
{
    assert(widget);
    wb_dropdown_t *dropdown = WB_DROPDOWN(widget);
    for (int i = 0; i < WB_DROPDOWN_MAX_ITEMS && *dropdown->items[i]; i++) {
        wb_widget_deinit(dropdown->item_buttons[i]);
    }
    free(dropdown);
}


wb_widget_t *
wb_dropdown_init(const char **items, wb_dropdown_on_select_t on_select_cb, void *data)
{
    wb_dropdown_t *dropdown = calloc(1, sizeof(wb_dropdown_t));
    assert(dropdown);

    dropdown->selected = -1;
    dropdown->on_select = on_select_cb;
    dropdown->data = data;

    const char **it = items;
    for (int i = 0; i < WB_DROPDOWN_MAX_ITEMS && *it && **it; i++) {
        strncpy(dropdown->items[i], *it, WB_BUTTON_TITLE_SIZE - 1);
        dropdown->items[i][WB_BUTTON_TITLE_SIZE - 1] = '\0';
        dropdown->item_buttons[i] = wb_button_init(dropdown->items[i], item_on_click, dropdown);
        it++;
    }

    dropdown->dirty = true;

    wb_widget_t *widget = WB_WIDGET(dropdown);
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


int
wb_dropdown_get_selected(wb_dropdown_t *dropdown)
{
    assert(dropdown);
    return dropdown->selected;
}


static int
get_num_items(wb_dropdown_t *dropdown)
{
    assert(dropdown);
    int i = 0;
    for (; i < WB_DROPDOWN_MAX_ITEMS && *dropdown->items[i]; i++) {
    }
    return i;
}


void
wb_dropdown_set_selected(wb_dropdown_t *dropdown, int position)
{
    assert(dropdown);
    int n = get_num_items(dropdown);
    if (position < n) {
        dropdown->selected = position;
        dropdown->on_select(WB_WIDGET(dropdown), dropdown->data);
        dropdown->dirty = true;
    }
}


bool
wb_dropdown_get_open(wb_dropdown_t *dropdown)
{
    assert(dropdown);
    return dropdown->open;
}


void
wb_dropdown_set_open(wb_dropdown_t *dropdown, bool open)
{
    assert(dropdown);
    if (dropdown->open != open) {
        dropdown->open = open;
        if (!dropdown->open) {
            dropdown->pressed = false;
        }
        dropdown->dirty = true;
    }
}


bool
wb_dropdown_on_overlay(wb_dropdown_t *dropdown, double x, double y)
{
    if (dropdown->open) {

        if ((x >= dropdown->x && x <= dropdown->x + dropdown->width) &&
            (y >= dropdown->y && y <= dropdown->y + dropdown->height)) {
            return true;
        }

        for (int i = 0; i < WB_DROPDOWN_MAX_ITEMS && *dropdown->items[i]; i++) {
            wb_rect_t rect = wb_widget_get_geometry(dropdown->item_buttons[i]);
            bool inside_item = (x >= rect.x && x <= rect.x + rect.width) &&
                               (y >= rect.y && y <= rect.y + rect.width);
            if (inside_item) {
                return true;
            }
        }

        // int n = get_num_items(dropdown);
        // return (x >= dropdown->x && x <= dropdown->x + dropdown->width) &&
        //        (y >= dropdown->y && y <= dropdown->y +
        //                                      n * (dropdown->height - WB_BUTTON_BORDER_WIDTH) +
        //                                      dropdown->height);
    }
    return false;
}
