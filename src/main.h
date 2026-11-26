#ifndef WB_MAIN_H
#define WB_MAIN_H

#include "config.h"
#include "widget.h"
#include "xkb.h"

#include <cairo/cairo.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#define APP_NAME "wboard"

#define ARRAY_SIZE(arr) ((sizeof(arr)) / sizeof((arr)[0]))

#define BUFFER_COUNT 4

static const int HIDDEN_SIZE = 20;


typedef struct wb_app wb_app_t;

typedef struct wb_buffer {
    wb_app_t *app;

    struct wl_buffer *wl_buffer;

    void *data;
    size_t size;

    cairo_surface_t *cairo_surface;

    int width;
    int height;

    bool busy;
} wb_buffer_t;


typedef struct wb_widgets {
    union {
        wb_widget_t *array[16];
        struct {
            wb_widget_t *close_btn;
            wb_widget_t *anchor_lbl;
            wb_widget_t *anchor_top_tggl;
            wb_widget_t *anchor_bottom_tggl;
            wb_widget_t *anchor_left_tggl;
            wb_widget_t *anchor_right_tggl;
            wb_widget_t *width_lbl;
            wb_widget_t *decrease_width_btn;
            wb_widget_t *increase_width_btn;
            wb_widget_t *height_lbl;
            wb_widget_t *decrease_height_btn;
            wb_widget_t *increase_height_btn;
            wb_widget_t *board;
            wb_widget_t *layout_lbl;
            wb_widget_t *layout_drop;
            wb_widget_t *autohide_tggl;
        };
    };
} wb_widgets_t;


typedef struct wb_app {
    /* globals */
    struct wl_display *wl_display;
    struct wl_registry *wl_registry;
    struct wl_compositor *wl_compositor;
    struct wl_seat *wl_seat;
    struct wl_shm *wl_shm;
    struct wl_output *wl_output;

    struct wl_pointer *wl_pointer;
    struct wl_keyboard *wl_keyboard;

    struct wp_cursor_shape_manager_v1 *wp_cursor_shape_manager;
    struct wp_cursor_shape_device_v1 *wp_cursor_shape_device;

    struct wl_surface *wl_surface;

    struct zwlr_layer_shell_v1 *zwlr_layer_shell;
    struct zwlr_layer_surface_v1 *zwlr_layer_surface;

    wb_buffer_t buffers[BUFFER_COUNT];

    wb_xkb_t xkb;

    double pointer_x;
    double pointer_y;

    int width;
    int height;

    bool configured;
    bool closed;
    bool hidden;

    wb_widgets_t widgets;
    wb_config_t config;
} wb_app_t;


#endif /* WB_MAIN_H */
