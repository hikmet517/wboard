#ifndef WB_KEY_H
#define WB_KEY_H

#include "draw.h"

#include <stdbool.h>
#include <stdint.h>


#define ENTER_NUM_OF_POINTS 6


typedef enum wb_key_shape_type {
    WB_KEY_SHAPE_RECT = 0,
    WB_KEY_SHAPE_ENTER,
} wb_key_shape_type_t;


typedef struct wb_key_shape {
    wb_key_shape_type_t shape_type;
    union {
        wb_rect_t rect;
        wb_point_t enter_points[ENTER_NUM_OF_POINTS];
    };
} wb_key_shape_t;


typedef enum wb_key_state {
    WB_KEY_RELEASED = 0,
    WB_KEY_PRESSED,
} wb_key_state_t;


#define KEY_NAME_SIZE 8

typedef struct wb_key {
    wb_key_shape_t shape;     /* rectangle or enter-shape */
    wb_key_state_t state;     /* physical state: released, pressed */
    bool hover;               /* mouse is over the button */
    char name[KEY_NAME_SIZE]; /* key name (e.g. AD04) */
} wb_key_t;


wb_key_t
wb_key_create(const char *name, wb_key_shape_t shape);


wb_key_shape_t
wb_key_shape_rect_create(float x, float y, float width, float height);


/*
 * Points must be clockwise, starting from top-left.
 * Only 6 (ENTER_NUM_OF_POINTS) points are allowed.
 */
wb_key_shape_t
wb_key_shape_enter_create(const wb_point_t *points);


bool
wb_key_inside(const wb_key_t *key, const wb_point_t *point);


#endif /* WB_KEY_H */
