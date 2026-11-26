#ifndef WB_UINPUT_H
#define WB_UINPUT_H

#include <stdbool.h>

#include <linux/input-event-codes.h>


bool
wb_uinput_init();

void
wb_uinput_emit_key(int code, int value);

void
wb_uinput_deinit();


/*
#include <unistd.h> // sleep
#include "uinput.h"

int main() {
    wb_uinput_init();

    sleep(1);  // allow the compositor to discover the device

    wb_uinput_emit_key(KEY_LEFTSHIFT, 1);  // press shift
    wb_uinput_emit_key(KEY_G, 1);          // press g
    wb_uinput_emit_key(KEY_G, 0);          // release g
    wb_uinput_emit_key(KEY_LEFTSHIFT, 0);  // release shift

    wb_uinput_deinit();
}
*/

#endif /* WB_UINPUT_H */
