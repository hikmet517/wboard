#include "label.h"
#include "draw.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


typedef struct wb_label {
    wb_widget_t parent;

    bool dirty;
    char title[WB_LABEL_TITLE_SIZE];
    int x;
    int y;
    int width;
    int height;
} wb_label_t;


static void
draw(const wb_widget_t *widget, cairo_t *cr)
{
    assert(widget);
    wb_label_t *label = WB_LABEL(widget);
    if (label->width == 0 || label->height == 0) {
        return;
    }

    wb_draw_text(cr, label->x, label->y, label->width, label->height, label->title, false);
    label->dirty = false;
}


static void
set_geometry(wb_widget_t *widget, int x, int y, int width, int height)
{
    assert(widget);
    wb_label_t *label = WB_LABEL(widget);

    label->x = x;
    label->y = y;
    label->width = width;
    label->height = height;
    label->dirty = true;
}


static wb_rect_t
get_geometry(wb_widget_t *widget)
{
    assert(widget);
    wb_label_t *label = WB_LABEL(widget);
    return (wb_rect_t){
        .x = label->x,
        .y = label->y,
        .width = label->width,
        .height = label->height,
    };
}


static bool
get_dirty(wb_widget_t *widget)
{
    assert(widget);
    wb_label_t *label = WB_LABEL(widget);
    return label->dirty;
}


static void
deinit(wb_widget_t *widget)
{
    assert(widget);
    wb_label_t *label = WB_LABEL(widget);
    free(label);
}


wb_widget_t *
wb_label_init(const char *title)
{
    wb_label_t *label = calloc(1, sizeof(wb_label_t));
    assert(label);

    label->dirty = true;
    strncpy(label->title, title, WB_LABEL_TITLE_SIZE - 1);
    label->title[WB_LABEL_TITLE_SIZE - 1] = '\0';

    wb_widget_t *widget = WB_WIDGET(label);
    widget->draw = draw;
    widget->set_geometry = set_geometry;
    widget->get_geometry = get_geometry;
    widget->get_dirty = get_dirty;
    widget->deinit = deinit;

    return widget;
}
