#include "draw.h"

#include <pango/pangocairo.h>

#include <math.h>


#define FONT_FAMILY "sans"


static void
get_pango_layout_bounds(PangoLayout *layout, PangoRectangle *bounds)
{
    PangoRectangle ink, logical;

    pango_layout_get_pixel_extents(layout, &ink, &logical);

    int left = MIN(ink.x, logical.x);
    int top = MIN(ink.y, logical.y);
    int right = MAX(ink.x + ink.width, logical.x + logical.width);
    int bottom = MAX(ink.y + ink.height, logical.y + logical.height);

    bounds->x = left;
    bounds->y = top;
    bounds->width = right - left;
    bounds->height = bottom - top;
}


static int
get_optimal_font_size(PangoLayout *layout, const PangoFontDescription *font_desc, double max_width,
                      double max_height)
{
    int min_size = 1;
    int max_size = 64;
    int best_size = min_size;

    PangoFontDescription *desc = pango_font_description_copy(font_desc);

    while (min_size <= max_size) {
        int mid_size = (min_size + max_size) / 2;

        pango_font_description_set_size(desc, mid_size * PANGO_SCALE);
        pango_layout_set_font_description(layout, desc);

        PangoRectangle bounds;
        get_pango_layout_bounds(layout, &bounds);

        if (bounds.width <= max_width && bounds.height <= max_height) {
            best_size = mid_size;
            min_size = mid_size + 1;
        } else {
            max_size = mid_size - 1;
        }
    }

    pango_font_description_free(desc);
    return best_size;
}


void
wb_draw_text(cairo_t *cr, double x, double y, double width, double height, const char *text,
             bool center)
{
    if (width == 0 || height == 0) {
        return;
    }

    PangoLayout *layout = pango_cairo_create_layout(cr);
    pango_layout_set_text(layout, text, -1);

    PangoFontDescription *desc = pango_font_description_new();
    pango_font_description_set_family(desc, FONT_FAMILY);
    // pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);

    int font_size = get_optimal_font_size(layout, desc, width, height);

    pango_font_description_set_size(desc, font_size * PANGO_SCALE);
    pango_layout_set_font_description(layout, desc);

    double text_x = x;
    double text_y = y;

    PangoRectangle bounds;
    get_pango_layout_bounds(layout, &bounds);
    if (center) {
        double x_offset = (width - bounds.width) / 2.0 - bounds.x;
        text_x += x_offset;
    }
    double y_offset = (height - bounds.height) / 2.0 - bounds.y;
    text_y += y_offset;

    cairo_user_to_device(cr, &text_x, &text_y);
    text_x = round(text_x);
    text_y = round(text_y);
    cairo_device_to_user(cr, &text_x, &text_y);


    // cairo_save(cr);
    // cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    // cairo_rectangle(cr, text_x, text_y, bounds.width, bounds.height);
    // cairo_stroke(cr);
    // cairo_restore(cr);


    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, text_x, text_y);
    pango_cairo_show_layout(cr, layout);

    pango_font_description_free(desc);
    g_object_unref(layout);
}


void
wb_draw_text_in_box(cairo_t *cr, double x, double y, double width, double height, const char *title,
                    const wb_color_t *bg_color, const wb_color_t *border_color, double border_width)
{
    if (width == 0 || height == 0) {
        return;
    }

    cairo_set_source_rgb(cr, bg_color->r, bg_color->g, bg_color->b);
    cairo_rectangle(cr, x + border_width / 2, y + border_width / 2, width - border_width,
                    height - border_width);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, border_color->r, border_color->g, border_color->b);
    cairo_set_line_width(cr, border_width);
    cairo_stroke(cr);

    float pad_x = width / 10;
    float pad_y = height / 10;
    wb_draw_text(cr, x + pad_x + border_width / 2, y + pad_y + border_width / 2,
                 width - 2 * pad_x - border_width, height - 2 * pad_y - border_width, title, true);
}
