#define _GNU_SOURCE

#include "main.h"
#include "board.h"
#include "button.h"
#include "config.h"
#include "draw.h"
#include "dropdown.h"
#include "label.h"
#include "togglebutton.h"
#include "uinput.h"
#include "utils.h"
#include "widget.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <linux/input-event-codes.h>
#include <linux/limits.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include "cursor-shape-v1-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

#include <xkbcommon/xkbcommon.h>

#include <cairo/cairo.h>


/*
 * Resources:
 * - https://wayland-book.com/
 * - https://wayland.app/protocols/wlr-layer-shell-unstable-v1
 * - https://en.wikipedia.org/wiki/ISO/IEC_9995
 */


static const wb_color_t BOARD_BG_COLOR = {0.8, 0.8, 0.8};
static const wb_color_t HIDDEN_BG_COLOR = {0.0, 0.8, 0.8};


#define NUM_LAYOUTS 6
const char *LAYOUT_NAMES[NUM_LAYOUTS + 1] = {
    "ANSI", "ISO B", "ISO E", "ISO J", "ISO K", "ISO L", NULL,
};
const char *LAYOUTS[NUM_LAYOUTS] = {"iso-a", "iso-b", "iso-e", "iso-j", "iso-k", "iso-l"};


static void
load_Layout_from_config(wb_app_t *app)
{
    char layout_dir[PATH_MAX - CONFIG_STRING_MAX - 16];
    if (get_layout_dir(layout_dir, sizeof(layout_dir))) {
        char layout_file[PATH_MAX];
        snprintf(layout_file, sizeof(layout_file), "%s/%s.svg", layout_dir, app->config.layout);
        if (!wb_board_load(WB_BOARD(app->widgets.board), layout_file)) {
            fprintf(stderr, "Failed to load board\n");
        }
    }
}


static void
layout_dropdown_on_select(wb_widget_t *widget, void *data)
{
    wb_app_t *app = data;
    wb_dropdown_t *dropdown = WB_DROPDOWN(widget);
    int selected = wb_dropdown_get_selected(dropdown);
    strcpy(app->config.layout, LAYOUTS[selected]);
    load_Layout_from_config(app);
}


static void
set_layout_from_config(wb_app_t *app)
{
    for (int i = 0; i < ARRAY_SIZE(LAYOUTS); i++) {
        if (strcmp(app->config.layout, LAYOUTS[i]) == 0) {
            wb_dropdown_set_selected(WB_DROPDOWN(app->widgets.layout_drop), i);
        }
    }
}


static void
close_button_on_click(wb_widget_t *widget, void *data)
{
    (void)widget;
    wb_app_t *app = data;
    if (app) {
        app->closed = true;
    }
}


static void
anchor_to_config(wb_app_t *app, uint32_t anchor)
{
    int i = 0;
    if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
        app->config.anchor[i++] = 'T';
    }
    if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) {
        app->config.anchor[i++] = 'B';
    }
    if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) {
        app->config.anchor[i++] = 'L';
    }
    if (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
        app->config.anchor[i++] = 'R';
    }
    app->config.anchor[i] = '\0';
}


static uint32_t
anchor_from_config(wb_app_t *app)
{
    uint32_t anchor = 0;
    const char *c = app->config.anchor;
    while (c && *c) {
        if (*c == 'T') {
            anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
        }
        if (*c == 'B') {
            anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
        }
        if (*c == 'L') {
            anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
        }
        if (*c == 'R') {
            anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
        }
        c++;
    }
    return anchor;
}


static void
update_anchor_and_size(wb_app_t *app)
{
    uint32_t anchor = anchor_from_config(app);
    zwlr_layer_surface_v1_set_anchor(app->zwlr_layer_surface, anchor);

    bool left = (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    bool right = (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    bool top = (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP);
    bool bottom = (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
    bool leftright_missing = !left || !right;
    bool topbottom_missing = !top || !bottom;

    int width = (leftright_missing ? app->config.width : 0);
    int height = (topbottom_missing ? app->config.height : 0);
    if (app->hidden) {
        if (((!top && bottom && left && !right) || (!top && bottom && !left && right) ||
             (!top && bottom && left && right) || (!top && bottom && !left && !right)) ||
            ((top && !bottom && left && !right) || (top && !bottom && !left && right) ||
             (top && !bottom && left && right) || (top && !bottom && !left && !right))) {
            height = HIDDEN_SIZE;
        } else if ((!top && !bottom && left && !right) || (!top && !bottom && !left && right)) {
            width = HIDDEN_SIZE;
        } else {
            app->hidden = false;
        }
    }
    zwlr_layer_surface_v1_set_size(app->zwlr_layer_surface, width, height);
}


static void
anchor_top_button_on_toggle(wb_widget_t *widget, void *data)
{
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    bool active = wb_togglebutton_get_active(togglebutton);
    wb_app_t *app = data;
    uint32_t anchor = anchor_from_config(app);
    if (active) {
        anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
    } else {
        anchor &= (~ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP);
    }
    anchor_to_config(app, anchor);
    update_anchor_and_size(app);
}


static void
anchor_bottom_button_on_toggle(wb_widget_t *widget, void *data)
{
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    bool active = wb_togglebutton_get_active(togglebutton);
    wb_app_t *app = data;
    uint32_t anchor = anchor_from_config(app);
    if (active) {
        anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
    } else {
        anchor &= (~ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
    }
    anchor_to_config(app, anchor);
    update_anchor_and_size(app);
}


static void
anchor_left_button_on_toggle(wb_widget_t *widget, void *data)
{
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    bool active = wb_togglebutton_get_active(togglebutton);
    wb_app_t *app = data;
    uint32_t anchor = anchor_from_config(app);
    if (active) {
        anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
    } else {
        anchor &= (~ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    }
    anchor_to_config(app, anchor);
    update_anchor_and_size(app);
}


static void
anchor_right_button_on_toggle(wb_widget_t *widget, void *data)
{
    wb_app_t *app = data;
    uint32_t anchor = anchor_from_config(app);
    wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
    if (wb_togglebutton_get_active(togglebutton)) {
        anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
    } else {
        anchor &= (~ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    }
    anchor_to_config(app, anchor);
    update_anchor_and_size(app);
}


static void
autohide_on_toggle(wb_widget_t *widget, void *data)
{
    // wb_app_t *app = data;
    // wb_togglebutton_t *togglebutton = WB_TOGGLEBUTTON(widget);
}


static void
set_anchor_from_config(wb_app_t *app)
{
    const char *c = app->config.anchor;
    while (c && *c) {
        if (*c == 'T') {
            wb_togglebutton_set_active(WB_TOGGLEBUTTON(app->widgets.anchor_top_tggl), true);
        }
        if (*c == 'B') {
            wb_togglebutton_set_active(WB_TOGGLEBUTTON(app->widgets.anchor_bottom_tggl), true);
        }
        if (*c == 'L') {
            wb_togglebutton_set_active(WB_TOGGLEBUTTON(app->widgets.anchor_left_tggl), true);
        }
        if (*c == 'R') {
            wb_togglebutton_set_active(WB_TOGGLEBUTTON(app->widgets.anchor_right_tggl), true);
        }
        c++;
    }
}


static void
increase_width_on_click(wb_widget_t *widget, void *data)
{
    (void)widget;
    wb_app_t *app = data;
    int step = (app->width != 0) ? app->width / 25 : 10;
    app->config.width += step;
    update_anchor_and_size(app);
}


static void
decrease_width_on_click(wb_widget_t *widget, void *data)
{
    (void)widget;
    wb_app_t *app = data;
    int step = (app->width != 0) ? app->width / 25 : 10;
    if (app->config.width - step > HIDDEN_SIZE) {
        app->config.width -= step;
        update_anchor_and_size(app);
    }
}


static void
increase_height_on_click(wb_widget_t *widget, void *data)
{
    (void)widget;
    wb_app_t *app = data;
    int step = (app->height != 0) ? app->height / 25 : 10;
    app->config.height += step;
    update_anchor_and_size(app);
}


static void
decrease_height_on_click(wb_widget_t *widget, void *data)
{
    (void)widget;
    wb_app_t *app = data;
    int step = (app->height != 0) ? app->height / 25 : 10;
    if (app->config.height - step > HIDDEN_SIZE) {
        app->config.height -= step;
        update_anchor_and_size(app);
    }
}


static bool
get_widgets_dirty(wb_app_t *app)
{
    bool dirty = false;
    for (int i = 0; i < ARRAY_SIZE(app->widgets.array); i++) {
        if (!app->widgets.array[i]) {
            break;
        }
        if (wb_widget_get_dirty(app->widgets.array[i])) {
            dirty = true;
            break;
        }
    }
    return dirty;
}


static int
create_shm_file(size_t size)
{
    int fd = memfd_create(APP_NAME "-buffer", MFD_CLOEXEC);

    if (fd < 0)
        return -1;

    if (ftruncate(fd, (off_t)size) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}


static void
destroy_buffer(wb_buffer_t *buffer)
{
    if (buffer->cairo_surface)
        cairo_surface_destroy(buffer->cairo_surface);

    if (buffer->wl_buffer)
        wl_buffer_destroy(buffer->wl_buffer);

    if (buffer->data && buffer->data != MAP_FAILED)
        munmap(buffer->data, buffer->size);

    memset(buffer, 0, sizeof(*buffer));
}


static void
wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    // printf("wl_buffer_release\n");
    (void)wl_buffer;
    wb_buffer_t *buffer = data;

    buffer->busy = false;
    // wb_app_t *app = buffer->app;
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};


static bool
create_buffer(wb_app_t *app, wb_buffer_t *buffer)
{
    // printf("create_buffer\n");

    cairo_format_t cairo_format = CAIRO_FORMAT_ARGB32;
    int stride = cairo_format_stride_for_width(cairo_format, app->width);

    size_t size = (size_t)stride * app->height;

    int fd = create_shm_file(size);

    if (fd < 0) {
        perror("memfd_create/ftruncate");
        return false;
    }

    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return false;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(app->wl_shm, fd, (int32_t)size);

    if (!pool) {
        munmap(data, size);
        close(fd);
        return false;
    }

    struct wl_buffer *wl_buffer =
        wl_shm_pool_create_buffer(pool, 0, app->width, app->height, stride, WL_SHM_FORMAT_ARGB8888);

    wl_shm_pool_destroy(pool);
    close(fd);

    if (!wl_buffer) {
        munmap(data, size);
        return false;
    }

    buffer->app = app;
    buffer->wl_buffer = wl_buffer;
    buffer->data = data;
    buffer->size = size;
    buffer->width = app->width;
    buffer->height = app->height;
    buffer->busy = false;

    buffer->cairo_surface =
        cairo_image_surface_create_for_data(data, cairo_format, app->width, app->height, stride);

    if (cairo_surface_status(buffer->cairo_surface) != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "Failed to create Cairo surface\n");
        destroy_buffer(buffer);
        return false;
    }

    wl_buffer_add_listener(wl_buffer, &wl_buffer_listener, buffer);
    return true;
}


static void
draw_hidden(cairo_t *cr)
{
    cairo_set_source_rgb(cr, HIDDEN_BG_COLOR.r, HIDDEN_BG_COLOR.g, HIDDEN_BG_COLOR.b);
    cairo_paint(cr);
}


static void
draw_bg(cairo_t *cr)
{
    cairo_set_source_rgb(cr, BOARD_BG_COLOR.r, BOARD_BG_COLOR.g, BOARD_BG_COLOR.b);
    cairo_paint(cr);
}


static void
draw_content(wb_app_t *app, cairo_t *cr)
{
    // printf("drawing\n");
    if (app->hidden) {
        draw_hidden(cr);
    } else {
        draw_bg(cr);
        for (int i = 0; i < ARRAY_SIZE(app->widgets.array); i++) {
            if (!app->widgets.array[i]) {
                break;
            }
            wb_widget_draw(app->widgets.array[i], cr);
        }
    }
}


void
draw_frame(wb_app_t *app)
{
    // printf("draw_frame()\n");
    if (!app->configured)
        return;

    wb_buffer_t *buffer = NULL;
    for (int i = 0; i < BUFFER_COUNT; ++i) {
        if (!app->buffers[i].busy) {
            buffer = &app->buffers[i];
            break;
        }
    }

    // no available buffer. the next wl_buffer_release event will trigger a redraw.
    if (!buffer) {
        // printf("no buffers\n");
        return;
    }

    // lazily create buffers.
    if (!buffer->wl_buffer) {
        if (!create_buffer(app, buffer)) {
            app->closed = true;
            return;
        }
        cairo_t *cr = cairo_create(buffer->cairo_surface);
        cairo_destroy(cr);
    }

    // draw content
    cairo_t *cr = cairo_create(buffer->cairo_surface);
    draw_content(app, cr);
    cairo_destroy(cr);
    cairo_surface_flush(buffer->cairo_surface);

    // the compositor must not access this buffer again until we receive wl_buffer.release.
    buffer->busy = true;
    wl_surface_attach(app->wl_surface, buffer->wl_buffer, 0, 0);
    wl_surface_damage(app->wl_surface, 0, 0, app->width, app->height);
    wl_surface_commit(app->wl_surface);
}


static void
wl_pointer_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial,
                 struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    wb_app_t *app = data;
    app->pointer_x = wl_fixed_to_double(surface_x);
    app->pointer_y = wl_fixed_to_double(surface_y);
    printf("wl_pointer_enter, serial: %d; surface: %d, x, y: %f, %f\n", serial,
           wl_proxy_get_id((struct wl_proxy *)surface), app->pointer_x, app->pointer_y);

    wp_cursor_shape_device_v1_set_shape(app->wp_cursor_shape_device, serial,
                                        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT);

    if (wb_togglebutton_get_active(WB_TOGGLEBUTTON(app->widgets.autohide_tggl)) && app->hidden) {
        app->hidden = false;
        update_anchor_and_size(app);
        draw_frame(app);
    } else {
        for (int i = 0; i < ARRAY_SIZE(app->widgets.array); i++) {
            if (!app->widgets.array[i]) {
                break;
            }
            wb_widget_mouse_move(app->widgets.array[i], app->pointer_x, app->pointer_y);
        }
        if (get_widgets_dirty(app)) {
            draw_frame(app);
        }
    }
}

static void
wl_pointer_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial,
                 struct wl_surface *surface)
{
    printf("wl_pointer_leave, surface: %d\n", wl_proxy_get_id((struct wl_proxy *)surface));

    wb_app_t *app = data;

    if (wb_togglebutton_get_active(WB_TOGGLEBUTTON(app->widgets.autohide_tggl)) && !app->hidden) {
        app->hidden = true;
        update_anchor_and_size(app);
        draw_frame(app);
    } else {
        for (int i = 0; i < ARRAY_SIZE(app->widgets.array); i++) {
            if (!app->widgets.array[i]) {
                break;
            }
            wb_widget_mouse_leave(app->widgets.array[i]);
        }
        if (get_widgets_dirty(app)) {
            draw_frame(app);
        }
    }
}

static void
wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x,
                  wl_fixed_t surface_y)
{
    wb_app_t *app = data;
    app->pointer_x = wl_fixed_to_double(surface_x);
    app->pointer_y = wl_fixed_to_double(surface_y);
    // printf("wl_pointer_motion, time: %d; x: %f, y: %f\n", time, pointer_x, pointer_y);

    if (wb_dropdown_on_overlay(WB_DROPDOWN(app->widgets.layout_drop), app->pointer_x,
                               app->pointer_y)) {
        wb_widget_mouse_move(app->widgets.layout_drop, app->pointer_x, app->pointer_y);
    } else {
        for (int i = 0; i < ARRAY_SIZE(app->widgets.array); i++) {
            if (!app->widgets.array[i]) {
                break;
            }
            wb_widget_mouse_move(app->widgets.array[i], app->pointer_x, app->pointer_y);
        }
    }
    if (get_widgets_dirty(app)) {
        draw_frame(app);
    }
}

static void
wl_pointer_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time,
                  uint32_t button, uint32_t state)
{
    // printf("wl_pointer_button, serial: %d; time: %d; button: %d, state: %d\n", serial, time,
    // button, state);
    wb_app_t *app = data;
    if (button == BTN_LEFT) {
        if (wb_dropdown_on_overlay(WB_DROPDOWN(app->widgets.layout_drop), app->pointer_x,
                                   app->pointer_y)) {
            if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
                wb_widget_mouse_press(app->widgets.layout_drop, app->pointer_x, app->pointer_y);
            } else if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
                wb_widget_mouse_release(app->widgets.layout_drop, app->pointer_x, app->pointer_y);
            }
        } else {
            wb_dropdown_set_open(WB_DROPDOWN(app->widgets.layout_drop), false);
            for (int i = 0; i < ARRAY_SIZE(app->widgets.array); i++) {
                if (!app->widgets.array[i]) {
                    break;
                }
                if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
                    wb_widget_mouse_press(app->widgets.array[i], app->pointer_x, app->pointer_y);
                } else if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
                    wb_widget_mouse_release(app->widgets.array[i], app->pointer_x, app->pointer_y);
                }
            }
        }
        if (get_widgets_dirty(app)) {
            draw_frame(app);
        }
    }
}

static void
wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis,
                wl_fixed_t value)
{
    // printf("wl_pointer_axis, time: %d; axis: %d, value: %f\n", time, axis,
    // wl_fixed_to_double(value));
}

static void
wl_pointer_frame(void *data, struct wl_pointer *wl_pointer)
{
    // printf("wl_pointer_frame\n");
}

static void
wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source)
{
    // printf("wl_pointer_axis_source: %d\n", axis_source);
}

static void
wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis)
{
    // printf("wl_pointer_axis_stop, time: %d; axis: %d\n", time, axis);
}

static void
wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete)
{
    // printf("wl_pointer_axis_discrete, axis: %d, discrete: %d\n", axis, discrete);
}

static const struct wl_pointer_listener wl_pointer_listener = {
    .enter = wl_pointer_enter,
    .leave = wl_pointer_leave,
    .motion = wl_pointer_motion,
    .button = wl_pointer_button,
    .axis = wl_pointer_axis,
    .frame = wl_pointer_frame,
    .axis_source = wl_pointer_axis_source,
    .axis_stop = wl_pointer_axis_stop,
    .axis_discrete = wl_pointer_axis_discrete,
};


static void
wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd,
                   uint32_t size)
{
    wb_app_t *app = data;
    // printf("wl_keyboard_keymap, format: %d, size: %d\n", format, size);
    char *map_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (map_shm == MAP_FAILED) {
        close(fd);
        fprintf(stderr, "Unable to mmap keymap: %s", strerror(errno));
        return;
    }

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        munmap(map_shm, size);
        close(fd);
        return;
    }

    struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
        app->xkb.context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map_shm, size);
    close(fd);

    struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
    xkb_keymap_unref(app->xkb.keymap);
    xkb_state_unref(app->xkb.state);
    app->xkb.keymap = xkb_keymap;
    app->xkb.state = xkb_state;

    draw_frame(app);
}

static void
wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
                  struct wl_surface *surface, struct wl_array *keys)
{
    // printf("wl_keyboard_enter, serial: %d; surface: %d\n", serial, wl_proxy_get_id((struct
    // wl_proxy *)surface));
}

static void
wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
                  struct wl_surface *surface)
{
    // printf("wl_keyboard_leave, serial: %d; surface: %d\n", serial, wl_proxy_get_id((struct
    // wl_proxy *)surface));
}

static void
wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time,
                uint32_t key, uint32_t state)
{
    // printf("wl_keyboard_key, serial: %d; time: %d; key: %d; state: %d\n", serial, time, key + 8,
    // state);
}

static void
wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
                      uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked,
                      uint32_t group)
{
    wb_app_t *app = data;
    // printf("wl_keyboard_modifiers, serial: %d; group: %d\n", serial, group);
    xkb_state_update_mask(app->xkb.state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
    draw_frame(app);
}

static void
wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay)
{
    // printf("wl_keyboard_repeat_info, rate: %d keys/sec; delay: %d ms\n", rate, delay);
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter,
    .leave = wl_keyboard_leave,
    .key = wl_keyboard_key,
    .modifiers = wl_keyboard_modifiers,
    .repeat_info = wl_keyboard_repeat_info,
};


static void
wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities)
{
    wb_app_t *app = data;
    // printf("wl_seat_capabilities\n");

    bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;
    if (have_pointer && !app->wl_pointer) {
        app->wl_pointer = wl_seat_get_pointer(wl_seat);
        wl_pointer_add_listener(app->wl_pointer, &wl_pointer_listener, data);
        app->wp_cursor_shape_device =
            wp_cursor_shape_manager_v1_get_pointer(app->wp_cursor_shape_manager, app->wl_pointer);
    } else if (!have_pointer && app->wl_pointer) {
        wl_pointer_destroy(app->wl_pointer);
        app->wl_pointer = NULL;
    }

    bool have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;
    if (have_keyboard && !app->wl_keyboard) {
        app->wl_keyboard = wl_seat_get_keyboard(wl_seat);
        wl_keyboard_add_listener(app->wl_keyboard, &wl_keyboard_listener, data);
    } else if (!have_keyboard && app->wl_keyboard) {
        wl_keyboard_destroy(app->wl_keyboard);
        app->wl_keyboard = NULL;
    }
}

static void
wl_seat_name(void *data, struct wl_seat *seat, const char *name)
{
    // printf("wl_seat_name: %s\n", name);
}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name,
};


static void
wl_registry_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t version)
{
    // printf("wl_registry_global, interface: '%s', version: %u, name: %u\n", interface, version,
    // name);
    wb_app_t *app = data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        app->wl_compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        app->wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 5);
        wl_seat_add_listener(app->wl_seat, &wl_seat_listener, app);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        app->wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, wp_cursor_shape_manager_v1_interface.name) == 0) {
        app->wp_cursor_shape_manager =
            wl_registry_bind(registry, name, &wp_cursor_shape_manager_v1_interface, 1);
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        app->zwlr_layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
    }
}

static void
wl_registry_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    (void)data;
    (void)registry;
    (void)name;
    // printf("wl_registry_global_remove: %u\n", name);
}

static const struct wl_registry_listener wl_registry_listener = {
    .global = wl_registry_global,
    .global_remove = wl_registry_global_remove,
};


static void
zwlr_layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *surface, uint32_t serial,
                             uint32_t width, uint32_t height)
{
    printf("zwlr_layer_surface_configure, width: %d, height: %d\n", width, height);
    zwlr_layer_surface_v1_ack_configure(surface, serial);

    wb_app_t *app = data;

    if (app->width != width || app->height != height) {
        for (int i = 0; i < BUFFER_COUNT; ++i) {
            destroy_buffer(&app->buffers[i]);
        }
        app->width = width;
        app->height = height;
    }

    if (!app->hidden && app->width != HIDDEN_SIZE && app->height != HIDDEN_SIZE) {

        int padding = 3;
        int button_width = 36;
        int button_height = 30;

        // close button
        wb_widget_set_geometry(app->widgets.close_btn, app->width - button_width - padding, padding,
                               button_width, button_height);

        // anchor widgets
        int anchor_beg_x = 3 * padding;
        int anchor_beg_y = padding;
        int anchor_lbl_width = 58;
        wb_widget_set_geometry(app->widgets.anchor_lbl, anchor_beg_x, anchor_beg_y,
                               anchor_lbl_width, button_height);
        wb_widget_set_geometry(app->widgets.anchor_left_tggl,
                               anchor_beg_x + anchor_lbl_width + padding, anchor_beg_y,
                               button_width, button_height);
        wb_widget_set_geometry(app->widgets.anchor_top_tggl,
                               anchor_beg_x + anchor_lbl_width + padding + button_width + padding,
                               anchor_beg_y, button_width, button_height);
        wb_widget_set_geometry(app->widgets.anchor_bottom_tggl,
                               anchor_beg_x + anchor_lbl_width + padding +
                                   2 * (button_width + padding),
                               anchor_beg_y, button_width, button_height);
        wb_widget_set_geometry(app->widgets.anchor_right_tggl,
                               anchor_beg_x + anchor_lbl_width + padding +
                                   3 * (button_width + padding),
                               anchor_beg_y, button_width, button_height);
        int anchor_end_x =
            anchor_beg_x + anchor_lbl_width + padding + 3 * (button_width + padding) + button_width;

        // width widgets
        int width_beg_x = anchor_end_x + 8 * padding;
        int width_beg_y = padding;
        int width_lbl_width = 50;
        wb_widget_set_geometry(app->widgets.width_lbl, width_beg_x, width_beg_y, width_lbl_width,
                               button_height);
        wb_widget_set_geometry(app->widgets.decrease_width_btn,
                               width_beg_x + width_lbl_width + padding, width_beg_y, button_width,
                               button_height);
        wb_widget_set_geometry(app->widgets.increase_width_btn,
                               width_beg_x + width_lbl_width + padding + button_width + padding,
                               width_beg_y, button_width, button_height);
        int width_end_x = width_beg_x + width_lbl_width + 2 * (padding + button_width);

        // height widgets
        int height_beg_x = width_end_x + 8 * padding;
        int height_beg_y = padding;
        int height_lbl_width = 55;
        wb_widget_set_geometry(app->widgets.height_lbl, height_beg_x, height_beg_y,
                               height_lbl_width, button_height);
        wb_widget_set_geometry(app->widgets.decrease_height_btn,
                               height_beg_x + height_lbl_width + padding, height_beg_y,
                               button_width, button_height);
        wb_widget_set_geometry(app->widgets.increase_height_btn,
                               height_beg_x + height_lbl_width + padding + button_width + padding,
                               height_beg_y, button_width, button_height);
        int height_end_x = height_beg_x + height_lbl_width + 2 * (padding + button_width);

        // layout widgets
        int layout_beg_x = height_end_x + 8 * padding;
        int layout_beg_y = padding;
        int layout_lbl_width = 52;
        int layout_drp_width = 100;
        wb_widget_set_geometry(app->widgets.layout_lbl, layout_beg_x, layout_beg_y,
                               layout_lbl_width, button_height);
        wb_widget_set_geometry(app->widgets.layout_drop, layout_beg_x + layout_lbl_width + padding,
                               layout_beg_y, layout_drp_width, button_height);
        int layout_end_x = layout_beg_x + layout_lbl_width + padding + layout_drp_width;

        int autohide_beg_x = layout_end_x + 8 * padding;
        int autohide_beg_y = padding;
        int autohide_width = 64;
        wb_widget_set_geometry(app->widgets.autohide_tggl, autohide_beg_x, autohide_beg_y,
                               autohide_width, button_height);

        // board
        wb_widget_set_geometry(app->widgets.board, padding, button_height + 4 * padding,
                               app->width - 2 * padding, app->height - button_height - 5 * padding);
        wb_rect_t rect = wb_widget_get_geometry(app->widgets.board);
        printf("configure rect: %f, %f, %f, %f\n", rect.x, rect.y, rect.width, rect.height);
    }

    app->configured = true;
    draw_frame(app);
}


static void
zwlr_layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *surface)
{
    (void)surface;
    wb_app_t *app = data;
    app->closed = true;
}


static const struct zwlr_layer_surface_v1_listener zwlr_layer_surface_listener = {
    .configure = zwlr_layer_surface_configure,
    .closed = zwlr_layer_surface_closed,
};


static bool
init(wb_app_t *app)
{
    assert(app);


    // config
    char config_file[PATH_MAX];
    if (get_config_file(config_file, sizeof(config_file))) {
        if (!config_load(config_file, &app->config) != 0) {
            fprintf(stderr, "Cannot load config: %s\n", config_file);
        }
    }
    // default configs
    if (app->config.width == 0) {
        app->config.width = 900;
    }
    if (app->config.height == 0) {
        app->config.height = 300;
    }
    if (app->config.anchor[0] == '\0') {
        strcpy(app->config.anchor, "B");
    }
    if (app->config.layout[0] == '\0') {
        strcpy(app->config.layout, "iso-e");
    }


    // xkb
    app->xkb.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!app->xkb.context) {
        fprintf(stderr, "Failed to initialize xkb context\n");
        return false;
    }

    // display
    app->wl_display = wl_display_connect(NULL);
    if (!app->wl_display) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        return false;
    }

    // registry
    app->wl_registry = wl_display_get_registry(app->wl_display);
    if (!app->wl_registry) {
        fprintf(stderr, "Failed to obtain Wayland registry\n");
        return false;
    }
    wl_registry_add_listener(app->wl_registry, &wl_registry_listener, app);
    wl_display_roundtrip(app->wl_display);

    // check
    if (!app->wl_compositor || !app->wl_seat || !app->wl_shm) {
        fprintf(stderr, "Failed to get some globals\n");
        return false;
    }

    // surface
    app->wl_surface = wl_compositor_create_surface(app->wl_compositor);
    app->zwlr_layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        app->zwlr_layer_shell, app->wl_surface, NULL, /* compositor chooses output */
        ZWLR_LAYER_SHELL_V1_LAYER_TOP, APP_NAME);
    zwlr_layer_surface_v1_set_exclusive_zone(app->zwlr_layer_surface, 0);
    zwlr_layer_surface_v1_set_keyboard_interactivity(
        app->zwlr_layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);
    zwlr_layer_surface_v1_add_listener(app->zwlr_layer_surface, &zwlr_layer_surface_listener, app);


    // widgets
    // close button widget
    app->widgets.close_btn = wb_button_init("⨯", close_button_on_click, app);

    // anchor widgets
    app->widgets.anchor_lbl = wb_label_init("Anchor:");
    app->widgets.anchor_top_tggl = wb_togglebutton_init("↑", anchor_top_button_on_toggle, app);
    app->widgets.anchor_bottom_tggl =
        wb_togglebutton_init("↓", anchor_bottom_button_on_toggle, app);
    app->widgets.anchor_left_tggl = wb_togglebutton_init("←", anchor_left_button_on_toggle, app);
    app->widgets.anchor_right_tggl = wb_togglebutton_init("→", anchor_right_button_on_toggle, app);
    set_anchor_from_config(app);

    // width widgets
    app->widgets.width_lbl = wb_label_init("width:");
    app->widgets.decrease_width_btn = wb_button_init("-", decrease_width_on_click, app);
    app->widgets.increase_width_btn = wb_button_init("+", increase_width_on_click, app);

    // height widgets
    app->widgets.height_lbl = wb_label_init("height:");
    app->widgets.decrease_height_btn = wb_button_init("-", decrease_height_on_click, app);
    app->widgets.increase_height_btn = wb_button_init("+", increase_height_on_click, app);

    // board widget
    app->widgets.board = wb_board_init(&app->xkb);

    // layout widgets
    app->widgets.layout_lbl = wb_label_init("layout:");
    app->widgets.layout_drop = wb_dropdown_init(LAYOUT_NAMES, layout_dropdown_on_select, app);
    set_layout_from_config(app);

    app->widgets.autohide_tggl = wb_togglebutton_init("Autohide", autohide_on_toggle, app);

    wl_surface_commit(app->wl_surface);

    return true;
}


static void
destroy(wb_app_t *app)
{
    char config_file[PATH_MAX];
    if (get_config_file(config_file, sizeof(config_file))) {
        if (!config_save(config_file, &app->config) != 0) {
            fprintf(stderr, "Cannot save config: %s\n", config_file);
        }
    }

    for (int i = 0; i < BUFFER_COUNT; ++i)
        destroy_buffer(&app->buffers[i]);

    if (app->wl_pointer)
        wl_pointer_destroy(app->wl_pointer);

    if (app->wl_keyboard)
        wl_keyboard_destroy(app->wl_keyboard);

    if (app->wl_seat)
        wl_seat_destroy(app->wl_seat);

    if (app->zwlr_layer_surface)
        zwlr_layer_surface_v1_destroy(app->zwlr_layer_surface);

    if (app->wl_surface)
        wl_surface_destroy(app->wl_surface);

    if (app->wl_shm)
        wl_shm_destroy(app->wl_shm);

    if (app->wl_compositor)
        wl_compositor_destroy(app->wl_compositor);

    if (app->wl_registry)
        wl_registry_destroy(app->wl_registry);

    if (app->wl_display)
        wl_display_disconnect(app->wl_display);

    for (int i = 0; i < ARRAY_SIZE(app->widgets.array); i++) {
        if (!app->widgets.array[i]) {
            break;
        }
        wb_widget_deinit(app->widgets.array[i]);
    }
}


int
main(int argc, char *argv[])
{
    wb_app_t app = {0};

    if (!wb_uinput_init()) {
        fprintf(stderr, "Failed to init uinput\n");
        return EXIT_FAILURE;
    }

    if (!init(&app)) {
        destroy(&app);
        return EXIT_FAILURE;
    }

    while (!app.closed) {
        if (wl_display_dispatch(app.wl_display) < 0) {
            fprintf(stderr, "Wayland connection lost\n");
            break;
        }
    }

    destroy(&app);
    wb_uinput_deinit();

    return EXIT_SUCCESS;
}
