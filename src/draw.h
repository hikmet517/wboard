#ifndef WB_DRAW_H
#define WB_DRAW_H

#include <cairo.h>
#include <stdbool.h>


typedef struct wb_point {
    float x;
    float y;
} wb_point_t;


typedef struct wb_rect {
    float x;
    float y;
    float width;
    float height;
} wb_rect_t;


typedef struct wb_color {
    double r, g, b;
} wb_color_t;


void
wb_draw_text(cairo_t *cr, double x, double y, double width, double height, const char *text,
             bool center);

void
wb_draw_text_in_box(cairo_t *cr, double x, double y, double width, double height, const char *title,
                    const wb_color_t *bg_color, const wb_color_t *border_color,
                    double border_width);


#endif /* WB_DRAW_H */
