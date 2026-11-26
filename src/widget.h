#ifndef WB_WIDGET_H
#define WB_WIDGET_H

#include <cairo.h>
#include <stdbool.h>

#include "draw.h"


struct wb_widget_t;
typedef struct wb_widget wb_widget_t;


#define WB_WIDGET(custom) ((wb_widget_t *)(custom))

struct wb_widget {
    void (*draw)(const wb_widget_t *widget, cairo_t *cr);
    void (*mouse_move)(wb_widget_t *widget, double x, double y);
    void (*mouse_leave)(wb_widget_t *widget);
    void (*mouse_press)(wb_widget_t *widget, double x, double y);
    void (*mouse_release)(wb_widget_t *widget, double x, double y);
    void (*set_geometry)(wb_widget_t *widget, int x, int y, int width, int height);
    wb_rect_t (*get_geometry)(wb_widget_t *widget);
    bool (*get_dirty)(wb_widget_t *widget);
    void (*deinit)(wb_widget_t *widget);
};


void
wb_widget_draw(const wb_widget_t *widget, cairo_t *cr);

void
wb_widget_mouse_move(wb_widget_t *widget, double x, double y);

void
wb_widget_mouse_leave(wb_widget_t *widget);

void
wb_widget_mouse_press(wb_widget_t *widget, double x, double y);

void
wb_widget_mouse_release(wb_widget_t *widget, double x, double y);

void
wb_widget_set_geometry(wb_widget_t *widget, int x, int y, int width, int height);

wb_rect_t
wb_widget_get_geometry(wb_widget_t *widget);

bool
wb_widget_get_dirty(wb_widget_t *widget);

void
wb_widget_deinit(wb_widget_t *widget);


#endif /* WB_WIDGET_H */
