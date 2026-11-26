#ifndef WB_LABEL_H
#define WB_LABEL_H

#include "widget.h"


#define WB_LABEL_TITLE_SIZE 64

typedef struct wb_label wb_label_t;

#define WB_LABEL(widget) ((wb_label_t *)(widget))


wb_widget_t *
wb_label_init(const char *title);


#endif /* WB_LABEL_H */
