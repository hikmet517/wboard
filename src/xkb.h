#ifndef WB_XKB_H
#define WB_XKB_H

#include <xkbcommon/xkbcommon.h>


#define WB_KEY_TITLE_SIZE 64
#define WB_KEY_SYMNAME_SIZE 64


typedef struct wb_xkb {
    struct xkb_state *state;
    struct xkb_context *context;
    struct xkb_keymap *keymap;
} wb_xkb_t;


typedef struct wb_xkb_mod {
    xkb_keycode_t keycode;
    xkb_mod_index_t mod_index;
} wb_xkb_mod_t;

#define WB_XKB_MOD_INVALID                                                                         \
    ((wb_xkb_mod_t){.keycode = XKB_KEYCODE_INVALID, .mod_index = XKB_MOD_INVALID})


bool
wb_xkb_mod_is_valid(wb_xkb_mod_t mod);

int
wb_xkb_get_all_mods(const wb_xkb_t *xkb, wb_xkb_mod_t *mods_out, int mods_size);

int
wb_xkb_get_mods(const wb_xkb_t *xkb, wb_xkb_mod_t *mods_out, int mods_size);

bool
wb_xkb_mod_depressed(const wb_xkb_t *xkb, wb_xkb_mod_t mod);

bool
wb_xkb_mod_latched(const wb_xkb_t *xkb, wb_xkb_mod_t mod);

bool
wb_xkb_mod_locked(const wb_xkb_t *xkb, wb_xkb_mod_t mod);

bool
wb_xkb_mod_effective(const wb_xkb_t *xkb, wb_xkb_mod_t mod);

int
wb_xkb_name_to_evdev(const wb_xkb_t *xkb, const char *name);

xkb_keycode_t
wb_xkb_get_keycode(const wb_xkb_t *xkb, const char *name);

xkb_keysym_t
wb_xkb_get_sym(const wb_xkb_t *xkb, const char *name);

int
wb_xkb_get_symname(const wb_xkb_t *xkb, const char *name, char *buffer, int size);

int
wb_xkb_get_utf8(const wb_xkb_t *xkb, const char *name, char *buffer, int size);

int
wb_xkb_get_title(const wb_xkb_t *xkb, const char *name, char *buffer, int size);


#endif /* WB_XKB_H */
