#ifndef WB_DROPDOWN_H
#define WB_DROPDOWN_H

#include "widget.h"

#include <stdbool.h>


#define WB_DROPDOWN_MAX_ITEMS 32

typedef struct wb_dropdown wb_dropdown_t;

#define WB_DROPDOWN(widget) ((wb_dropdown_t *)(widget))

typedef void(wb_dropdown_on_select_t)(wb_widget_t *widget, void *data);


wb_widget_t *
wb_dropdown_init(const char **items, wb_dropdown_on_select_t on_select_cb, void *data);

int
wb_dropdown_get_selected(wb_dropdown_t *dropdown);

void
wb_dropdown_set_selected(wb_dropdown_t *dropdown, int position);

bool
wb_dropdown_get_open(wb_dropdown_t *dropdown);

void
wb_dropdown_set_open(wb_dropdown_t *dropdown, bool open);

bool
wb_dropdown_on_overlay(wb_dropdown_t *dropdown, double x, double y);


#endif /* WB_DROPDOWN_H */
