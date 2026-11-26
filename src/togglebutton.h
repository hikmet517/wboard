#ifndef WB_TOGGLEBUTTON_H
#define WB_TOGGLEBUTTON_H

#include "widget.h"

#include <stdbool.h>


typedef struct wb_togglebutton wb_togglebutton_t;

#define WB_TOGGLEBUTTON(widget) ((wb_togglebutton_t *)(widget))

typedef void(wb_togglebutton_on_change_t)(wb_widget_t *widget, void *data);


wb_widget_t *
wb_togglebutton_init(const char *title, wb_togglebutton_on_change_t on_change_cb, void *data);

bool
wb_togglebutton_get_active(wb_togglebutton_t *togglebutton);

void
wb_togglebutton_set_active(wb_togglebutton_t *togglebutton, bool active);


#endif /* WB_TOGGLEBUTTON_H */
