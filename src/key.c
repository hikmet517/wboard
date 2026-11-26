#include "key.h"

#include <string.h>


#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ABS(a) (((a) < 0) ? -(a) : (a))


wb_key_t
wb_key_create(const char *name, wb_key_shape_t shape)
{
    wb_key_t key = {
        .shape = shape,
    };
    strncpy(key.name, name, KEY_NAME_SIZE - 1);
    key.name[KEY_NAME_SIZE - 1] = '\0';
    return key;
}


wb_key_shape_t
wb_key_shape_rect_create(float x, float y, float width, float height)
{
    return (wb_key_shape_t){
        .shape_type = WB_KEY_SHAPE_RECT,
        .rect =
            {
                .x = x,
                .y = y,
                .width = width,
                .height = height,
            },
    };
}


wb_key_shape_t
wb_key_shape_enter_create(const wb_point_t *points)
{
    wb_key_shape_t key = {
        .shape_type = WB_KEY_SHAPE_ENTER,
    };
    memcpy(key.enter_points, points, sizeof(points[0]) * ENTER_NUM_OF_POINTS);
    return key;
}


static wb_rect_t
rect_from_two_points(const wb_point_t *point1, const wb_point_t *point2)
{
    return (wb_rect_t){
        .x = MIN(point1->x, point2->x),
        .y = MIN(point1->y, point2->y),
        .width = ABS(point1->x - point2->x),
        .height = ABS(point1->y - point2->y),
    };
}


static bool
rect_inside(const wb_rect_t *rect, const wb_point_t *point)
{
    return (point->x >= rect->x) && (point->x <= rect->x + rect->width) && (point->y >= rect->y) &&
           (point->y <= rect->y + rect->height);
}


bool
wb_key_inside(const wb_key_t *key, const wb_point_t *point)
{
    switch (key->shape.shape_type) {
        case WB_KEY_SHAPE_RECT:
            return rect_inside(&key->shape.rect, point);
        case WB_KEY_SHAPE_ENTER: {
            wb_rect_t rect1 =
                rect_from_two_points(&key->shape.enter_points[1], &key->shape.enter_points[5]);
            wb_rect_t rect2 =
                rect_from_two_points(&key->shape.enter_points[2], &key->shape.enter_points[4]);
            return rect_inside(&rect1, point) || rect_inside(&rect2, point);
        }
        default:
            return false;
    }
}
