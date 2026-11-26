#include "board.h"
#include "draw.h"
#include "key.h"
#include "main.h"
#include "svg.h"
#include "uinput.h"
#include "xkb.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <pango/pangocairo.h>
#include <xkbcommon/xkbcommon.h>


static const wb_color_t KEY_BG_COLOR = {1.0, 1.0, 1.0};
static const wb_color_t KEY_BG_COLOR_PRESSED = {0.82, 0.82, 0.82};
static const wb_color_t KEY_BG_COLOR_LATCHED = {0.65, 1.0, 0.5};
static const wb_color_t KEY_BG_COLOR_LOCKED = {1.0, 1.0, 0.5};
static const wb_color_t KEY_BG_COLOR_HOVER = {0.9, 0.9, 0.9};
static const wb_color_t KEY_BORDER_COLOR = {0.5, 0.5, 0.5};
static const double KEY_BORDER_WIDTH = 2.0;


typedef struct wb_board {
    wb_widget_t parent;

    const wb_xkb_t *xkb;
    wb_key_t keys[128];
    int num_keys;
    int x;
    int y;
    int width;
    int height;
    bool dirty;
} wb_board_t;


static float
get_board_width(wb_board_t *board)
{
    assert(board);

    float max_width = 0;
    for (int i = 0; i < board->num_keys; i++) {
        if (board->keys[i].shape.shape_type == WB_KEY_SHAPE_RECT) {
            wb_rect_t *rect = &board->keys[i].shape.rect;
            float width = rect->x + rect->width;
            if (width > max_width) {
                max_width = width;
            }
        }
    }
    return max_width;
}


static float
get_board_height(wb_board_t *board)
{
    assert(board);

    float max_height = 0;
    for (int i = 0; i < board->num_keys; i++) {
        if (board->keys[i].shape.shape_type == WB_KEY_SHAPE_RECT) {
            wb_rect_t *rect = &board->keys[i].shape.rect;
            float height = rect->y + rect->height;
            if (height > max_height) {
                max_height = height;
            }
        }
    }
    return max_height;
}


static void
scale_board(wb_board_t *board)
{
    assert(board);

    if (board->width == 0 || board->height == 0) {
        return;
    }

    float scale_x = board->width / get_board_width(board);
    float scale_y = board->height / get_board_height(board);
    for (int i = 0; i < board->num_keys; i++) {
        switch (board->keys[i].shape.shape_type) {
            case WB_KEY_SHAPE_RECT:
                board->keys[i].shape.rect.x *= scale_x;
                board->keys[i].shape.rect.y *= scale_y;
                board->keys[i].shape.rect.width *= scale_x;
                board->keys[i].shape.rect.height *= scale_y;
                break;
            case WB_KEY_SHAPE_ENTER: {
                for (int j = 0; j < ENTER_NUM_OF_POINTS; j++) {
                    board->keys[i].shape.enter_points[j].x *= scale_x;
                    board->keys[i].shape.enter_points[j].y *= scale_y;
                }
                break;
            }
        }
    }
}


static wb_color_t
key_get_color(const wb_key_t *key, const wb_xkb_t *xkb, wb_xkb_mod_t *mods, int mods_size)
{
    xkb_keycode_t keycode = wb_xkb_get_keycode(xkb, key->name);
    if (keycode != XKB_KEYCODE_INVALID) {
        for (int i = 0; i < mods_size; i++) {
            if (mods[i].keycode == keycode) {
                if (wb_xkb_mod_depressed(xkb, mods[i])) {
                    return KEY_BG_COLOR_PRESSED;
                }
                if (wb_xkb_mod_latched(xkb, mods[i])) {
                    return KEY_BG_COLOR_LATCHED;
                }
                if (wb_xkb_mod_locked(xkb, mods[i]) == 1) {
                    return KEY_BG_COLOR_LOCKED;
                }
                if (key->hover) {
                    return KEY_BG_COLOR_HOVER;
                }
                return KEY_BG_COLOR;
            }
        }
    }
    if (key->state == WB_KEY_PRESSED) {
        return KEY_BG_COLOR_PRESSED;
    }
    if (key->hover) {
        return KEY_BG_COLOR_HOVER;
    }

    return KEY_BG_COLOR;
}


static void
key_emit(const wb_key_t *key, const wb_xkb_t *xkb, int value)
{
    assert(key);
    assert(xkb);
    int keycode = wb_xkb_name_to_evdev(xkb, key->name);
#ifndef NDEBUG
    char symname[WB_KEY_SYMNAME_SIZE] = {0};
    wb_xkb_get_symname(xkb, key->name, symname, sizeof(symname));
    printf("KEY %s: %s, (code: %d, sym: %s)\n", value ? "PRESS" : "RELEASE", key->name, keycode + 8,
           symname);
#endif
    wb_uinput_emit_key(keycode, value);
}


static wb_xkb_mod_t
key_get_mod(const wb_key_t *key, const wb_xkb_t *xkb, wb_xkb_mod_t *mods, int mods_size)
{
    assert(key);
    assert(xkb);
    xkb_keycode_t keycode = wb_xkb_get_keycode(xkb, key->name);
    if (keycode == XKB_KEYCODE_INVALID) {
        return WB_XKB_MOD_INVALID;
    }

    for (int i = 0; i < mods_size; i++) {
        if (keycode == mods[i].keycode) {
            return mods[i];
        }
    }

    return WB_XKB_MOD_INVALID;
}


static void
draw(const wb_widget_t *widget, cairo_t *cr)
{
    assert(widget);
    wb_board_t *board = WB_BOARD(widget);
    if (board->width == 0 || board->height == 0) {
        return;
    }

    wb_xkb_mod_t mods[64];
    int mods_size = wb_xkb_get_all_mods(board->xkb, mods, ARRAY_SIZE(mods));

    for (int i = 0; i < board->num_keys; i++) {
        wb_key_t *key = &board->keys[i];
        float key_x, key_y, key_w, key_h;
        wb_color_t color = key_get_color(&board->keys[i], board->xkb, mods, mods_size);
        cairo_set_source_rgb(cr, color.r, color.g, color.b);
        wb_key_shape_t *shape = &key->shape;
        switch (shape->shape_type) {
            case WB_KEY_SHAPE_RECT: {
                /* draw rectangular button shape */
                key_x = board->x + shape->rect.x + KEY_BORDER_WIDTH / 2;
                key_y = board->y + shape->rect.y + KEY_BORDER_WIDTH / 2;
                key_w = shape->rect.width - KEY_BORDER_WIDTH;
                key_h = shape->rect.height - KEY_BORDER_WIDTH;
                cairo_rectangle(cr, key_x, key_y, key_w, key_h);
                break;
            }
            case WB_KEY_SHAPE_ENTER: {
                wb_point_t *points = shape->enter_points;
                key_x = board->x + points[0].x + KEY_BORDER_WIDTH / 2;
                key_y = board->y + points[0].y + KEY_BORDER_WIDTH / 2;
                key_w = points[1].x - points[0].x - KEY_BORDER_WIDTH / 2;
                key_h = points[5].y - points[0].y - KEY_BORDER_WIDTH / 2;
                cairo_move_to(cr, key_x, key_y);
                cairo_line_to(cr, board->x + points[1].x - KEY_BORDER_WIDTH / 2,
                              board->y + points[1].y + KEY_BORDER_WIDTH / 2);
                cairo_line_to(cr, board->x + points[2].x - KEY_BORDER_WIDTH / 2,
                              board->y + points[2].y - KEY_BORDER_WIDTH / 2);
                cairo_line_to(cr, board->x + points[3].x + KEY_BORDER_WIDTH / 2,
                              board->y + points[3].y - KEY_BORDER_WIDTH / 2);
                if ((points[2].x - points[0].x) > (points[2].x - points[4].x)) {
                    cairo_line_to(cr, board->x + points[4].x + KEY_BORDER_WIDTH / 2,
                                  board->y + points[4].y - KEY_BORDER_WIDTH / 2);
                    cairo_line_to(cr, board->x + points[5].x + KEY_BORDER_WIDTH / 2,
                                  board->y + points[5].y - KEY_BORDER_WIDTH / 2);
                } else {
                    cairo_line_to(cr, board->x + points[4].x + KEY_BORDER_WIDTH / 2,
                                  board->y + points[4].y + KEY_BORDER_WIDTH / 2);
                    cairo_line_to(cr, board->x + points[5].x + KEY_BORDER_WIDTH / 2,
                                  board->y + points[5].y + KEY_BORDER_WIDTH / 2);
                }
                cairo_close_path(cr);
                break;
            }
        }
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, KEY_BORDER_COLOR.r, KEY_BORDER_COLOR.g, KEY_BORDER_COLOR.b);
        cairo_set_line_width(cr, KEY_BORDER_WIDTH);
        cairo_stroke(cr);

        /* draw text */
        char title[WB_KEY_TITLE_SIZE] = {0};
        wb_xkb_get_title(board->xkb, key->name, title, WB_KEY_TITLE_SIZE);

        float pad_x = key_w / 10;
        float pad_y = key_h / 10;
        wb_draw_text(cr, key_x + pad_x + KEY_BORDER_WIDTH / 2, key_y + pad_y + KEY_BORDER_WIDTH / 2,
                     key_w - 2 * pad_x - KEY_BORDER_WIDTH, key_h - 2 * pad_y - KEY_BORDER_WIDTH,
                     title, true);
    }
    board->dirty = false;
}


static void
mouse_move(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_board_t *board = WB_BOARD(widget);

    wb_point_t point = (wb_point_t){.x = x - board->x, .y = y - board->y};
    for (int i = 0; i < board->num_keys; i++) {
        if (wb_key_inside(&board->keys[i], &point)) {
            if (board->keys[i].hover == false) {
                board->keys[i].hover = true;
                board->dirty = true;
            }
        } else {
            if (board->keys[i].hover) {
                board->keys[i].hover = false;
                board->dirty = true;
            }
        }
    }
}


static void
mouse_leave(wb_widget_t *widget)
{
    assert(widget);
    wb_board_t *board = WB_BOARD(widget);

    for (int i = 0; i < board->num_keys; i++) {
        if (board->keys[i].hover) {
            board->keys[i].hover = false;
            board->dirty = true;
        }
    }
}


static void
mouse_press(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_board_t *board = WB_BOARD(widget);

    wb_point_t point = (wb_point_t){.x = x - board->x, .y = y - board->y};
    for (int i = 0; i < board->num_keys; i++) {
        wb_key_t *key = &board->keys[i];
        if (wb_key_inside(key, &point)) {
            if (key->state != WB_KEY_PRESSED) {
                key->state = WB_KEY_PRESSED;
                board->dirty = true;
            }
        }
    }
}


static void
mouse_release(wb_widget_t *widget, double x, double y)
{
    assert(widget);
    wb_board_t *board = WB_BOARD(widget);

    wb_xkb_mod_t mods[64];
    int mods_size = wb_xkb_get_mods(board->xkb, mods, ARRAY_SIZE(mods));

    wb_point_t point = (wb_point_t){.x = x - board->x, .y = y - board->y};
    for (int i = 0; i < board->num_keys; i++) {
        wb_key_t *key = &board->keys[i];
        if (wb_key_inside(key, &point) && key->state == WB_KEY_PRESSED) {
            wb_xkb_mod_t key_mod = key_get_mod(key, board->xkb, mods, mods_size);

            // Press logic
            key_emit(key, board->xkb, 1);
            if (!wb_xkb_mod_is_valid(key_mod)) {
                // if there is a mod key held (like shift), release it
                for (int j = 0; j < board->num_keys; j++) {
                    wb_xkb_mod_t mod = key_get_mod(&board->keys[j], board->xkb, mods, mods_size);
                    if (wb_xkb_mod_depressed(board->xkb, mod)) {
                        key_emit(&board->keys[j], board->xkb, 0);
                    }
                }
            }

            // Release logic
            if (wb_xkb_mod_is_valid(key_mod)) {
                if (wb_xkb_mod_depressed(board->xkb, key_mod)) {
                    key_emit(key, board->xkb, 0);
                }
            } else {
                key_emit(key, board->xkb, 0);
            }
            key->state = WB_KEY_RELEASED;
            board->dirty = true;
        }
    }
}


static void
set_geometry(wb_widget_t *widget, int x, int y, int width, int height)
{
    assert(widget);
    wb_board_t *board = WB_BOARD(widget);
    board->x = x;
    board->y = y;
    board->width = width;
    board->height = height;
    scale_board(board);
    board->dirty = true;
}


static wb_rect_t
get_geometry(wb_widget_t *widget)
{
    assert(widget);
    wb_board_t *board = WB_BOARD(widget);
    return (wb_rect_t){
        .x = board->x,
        .y = board->y,
        .width = board->width,
        .height = board->height,
    };
}


static bool
get_dirty(wb_widget_t *widget)
{
    assert(widget);
    wb_board_t *board = WB_BOARD(widget);
    return board->dirty;
}


static void
deinit(wb_widget_t *widget)
{
    assert(widget);

    wb_board_t *board = WB_BOARD(widget);
    free(board);
}


wb_widget_t *
wb_board_init(const wb_xkb_t *xkb)
{
    wb_board_t *board = calloc(1, sizeof(wb_board_t));
    assert(board);

    board->xkb = xkb;
    board->dirty = true;

    wb_widget_t *widget = WB_WIDGET(board);
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


bool
wb_board_load(wb_board_t *board, const char *svg_file)
{
    assert(board);

    memset(board->keys, 0, sizeof(board->keys));
    board->num_keys = wb_svg_load(svg_file, board->keys, ARRAY_SIZE(board->keys));
    if (board->num_keys < 0) {
        fprintf(stderr, "Failed to load svg\n");
        return false;
    }
    scale_board(board);
    board->dirty = true;
    return true;
}
