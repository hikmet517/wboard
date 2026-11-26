#ifndef WB_BUTTON_H
#define WB_BUTTON_H

#include "draw.h"
#include "widget.h"

#include <cairo.h>

#include <stdbool.h>


#define WB_BUTTON_TITLE_SIZE 64

typedef struct wb_button wb_button_t;

#define WB_BUTTON(widget) ((wb_button_t *)(widget))


static const wb_color_t WB_BUTTON_BG_COLOR = {0.9, 0.9, 0.9};
static const wb_color_t WB_BUTTON_HOVER_BG_COLOR = {0.8, 0.8, 0.8};
static const wb_color_t WB_BUTTON_PRESSED_BG_COLOR = {0.7, 0.7, 0.7};
static const wb_color_t WB_BUTTON_BORDER_COLOR = {0.5, 0.5, 0.5};
static const double WB_BUTTON_BORDER_WIDTH = 2.0;


typedef void(wb_button_on_click_t)(wb_widget_t *widget, void *data);


typedef struct wb_button wb_button_t;


wb_widget_t *
wb_button_init(const char *title, wb_button_on_click_t on_click_cb, void *data);


#endif /* WB_BUTTON_H */
