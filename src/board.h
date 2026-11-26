#ifndef WB_BOARD_H
#define WB_BOARD_H

#include "widget.h"
#include "xkb.h"

#include <cairo.h>

#include <stdbool.h>


typedef struct wb_board wb_board_t;


#define WB_BOARD(widget) ((wb_board_t *)(widget))


wb_widget_t *
wb_board_init(const wb_xkb_t *xkb);

bool
wb_board_load(wb_board_t *board, const char *svg_file);


#endif /* WB_BOARD_H */
