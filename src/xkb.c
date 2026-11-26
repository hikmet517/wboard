#include "xkb.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <xkbcommon/xkbcommon.h>


/* resources:
 * - uinput: https://www.kernel.org/doc/html/v4.16/input/uinput.html
 * - event-codes: https://docs.kernel.org/input/event-codes.html
 * - kbd: http://www.kbd-project.org/
 *   - /usr/share/kbd/keymaps/i386/qwerty/trq.map.gz
 * - libxkbcommon: https://xkbcommon.org/ (https://github.com/xkbcommon/libxkbcommon)
 *   - https://xkbcommon.org/doc/current/xkb-intro.html
 *   - https://xkbcommon.org/doc/current/md_doc_2quick-guide.html
 *   - https://xkbcommon.org/doc/current/keymap-text-format-v1-v2.html
 *   - https://xkbcommon.org/doc/current/faq.html
 *   - https://xorg.freedesktop.org/archive/current/doc/kbproto/xkbproto.html
 * - xkeyboard-config: https://gitlab.freedesktop.org/xkeyboard-config/xkeyboard-config
 *   - /usr/share/xkeyboard-config-2/keycodes/evdev
 * - ISO/IEC 9995: https://en.wikipedia.org/wiki/ISO/IEC_9995
 * - https://en.wiktionary.org/wiki/Category:Combining_Diacritical_Marks_block
 * - https://en.wiktionary.org/wiki/Appendix:Unicode/Combining_Diacritical_Marks
 */


typedef struct {
    xkb_keysym_t sym;
    char *title;
} key_name_title_t;


static const key_name_title_t KEY_TITLE_OVERRIDES[] = {
    {XKB_KEY_Zenkaku_Hankaku, "半角/\n全角"},
    {XKB_KEY_Eisu_toggle, "英数"},
    {XKB_KEY_Muhenkan, "無変換"},
    {XKB_KEY_Henkan_Mode, "変換"},
    {XKB_KEY_Hiragana, "ひらがな"},

    {XKB_KEY_BackSpace, "⌫"},
    {XKB_KEY_Tab, "Tab"},
    {XKB_KEY_ISO_Left_Tab, "Tab"},
    {XKB_KEY_Caps_Lock, "Caps Lock"},
    {XKB_KEY_Return, "Enter"},
    {XKB_KEY_Shift_L, "Shift"},
    {XKB_KEY_Shift_R, "Shift"},
    {XKB_KEY_Control_L, "Ctrl"},
    {XKB_KEY_Super_L, "Win"},
    {XKB_KEY_Alt_L, "Alt"},
    {XKB_KEY_ISO_Level3_Shift, "Alt Gr"},
    {XKB_KEY_Super_R, "Win"},
    {XKB_KEY_Control_R, "Ctrl"},
    {XKB_KEY_Escape, "Esc"},
    {XKB_KEY_Print, "Print\nScreen"},
    {XKB_KEY_Sys_Req, "Sys\nReq"},
    {XKB_KEY_Scroll_Lock, "Scroll\nLock"},
    {XKB_KEY_Delete, "Delete"},
    {XKB_KEY_Prior, "Page\nUp"},
    {XKB_KEY_Next, "Page\nDown"},
    {XKB_KEY_Up, "↑"},
    {XKB_KEY_Left, "←"},
    {XKB_KEY_Down, "↓"},
    {XKB_KEY_Right, "→"},
    {XKB_KEY_VoidSymbol, ""},

    {XKB_KEY_dead_grave, "◌̀"},
    {XKB_KEY_dead_acute, "◌́"},
    {XKB_KEY_dead_circumflex, "◌̂"},
    {XKB_KEY_dead_tilde, "◌̃"},
    {XKB_KEY_dead_perispomeni, "◌͂"},
    {XKB_KEY_dead_macron, "◌̄"},
    {XKB_KEY_dead_breve, "◌̆"},
    {XKB_KEY_dead_abovedot, "◌̇"},
    {XKB_KEY_dead_diaeresis, "◌̈"},
    {XKB_KEY_dead_abovering, "◌̊"},
    {XKB_KEY_dead_doubleacute, "◌̋"},
    {XKB_KEY_dead_caron, "◌̌"},
    {XKB_KEY_dead_cedilla, "◌̧"},
    {XKB_KEY_dead_ogonek, "◌̨"},
    {XKB_KEY_dead_iota, "◌ͅ"},
    {XKB_KEY_dead_voiced_sound, "◌゛"},
    {XKB_KEY_dead_semivoiced_sound, "◌゜"},
    {XKB_KEY_dead_belowdot, "◌̣"},
    {XKB_KEY_dead_hook, "◌̉"},
    {XKB_KEY_dead_horn, "◌̛"},
    {XKB_KEY_dead_stroke, "◌̷"},
    {XKB_KEY_dead_abovecomma, "◌̓"},
    {XKB_KEY_dead_psili, "◌̓"},
    {XKB_KEY_dead_abovereversedcomma, "◌̔"},
    {XKB_KEY_dead_dasia, "◌̔"},
    {XKB_KEY_dead_doublegrave, "◌̏"},
    {XKB_KEY_dead_belowring, "◌̥"},
    {XKB_KEY_dead_belowmacron, "◌̱"},
    {XKB_KEY_dead_belowcircumflex, "◌̭"},
    {XKB_KEY_dead_belowtilde, "◌̰"},
    {XKB_KEY_dead_belowbreve, "◌̮"},
    {XKB_KEY_dead_belowdiaeresis, "◌̤"},
    {XKB_KEY_dead_invertedbreve, "◌̑"},
    {XKB_KEY_dead_belowcomma, "◌̦"},
    {XKB_KEY_dead_currency, "◌¤"},

    {XKB_KEY_dead_lowline, "◌̲"},
    {XKB_KEY_dead_aboveverticalline, "◌̍"},
    {XKB_KEY_dead_belowverticalline, "◌̩"},
    {XKB_KEY_dead_longsolidusoverlay, "◌̸"},

    {XKB_KEY_dead_a, "◌ͣ"},
    {XKB_KEY_dead_A, "◌ᴬ"},
    {XKB_KEY_dead_e, "◌ͤ"},
    {XKB_KEY_dead_E, "◌ᴱ"},
    {XKB_KEY_dead_i, "◌ͥ"},
    {XKB_KEY_dead_I, "◌ᴵ"},
    {XKB_KEY_dead_o, "◌ͦ"},
    {XKB_KEY_dead_O, "◌ᴼ"},
    {XKB_KEY_dead_u, "◌ͧ"},
    {XKB_KEY_dead_U, "◌ᵁ"},
    {XKB_KEY_dead_small_schwa, "◌ᷪ"},
    {XKB_KEY_dead_schwa, "◌ᷪ"},
    {XKB_KEY_dead_capital_schwa, "◌Ə"},
    {XKB_KEY_dead_SCHWA, "◌Ə"},

    {XKB_KEY_dead_greek, "◌μ"},
    {XKB_KEY_dead_hamza, "◌ٔ"},
    {XKB_KEY_dead_apostrophe, "◌́"},

    {XKB_KEY_XF86Switch_VT_1, "VT 1"},
    {XKB_KEY_XF86Switch_VT_2, "VT 2"},
    {XKB_KEY_XF86Switch_VT_3, "VT 3"},
    {XKB_KEY_XF86Switch_VT_4, "VT 4"},
    {XKB_KEY_XF86Switch_VT_5, "VT 5"},
    {XKB_KEY_XF86Switch_VT_6, "VT 6"},
    {XKB_KEY_XF86Switch_VT_7, "VT 7"},
    {XKB_KEY_XF86Switch_VT_8, "VT 8"},
    {XKB_KEY_XF86Switch_VT_9, "VT 9"},
    {XKB_KEY_XF86Switch_VT_10, "VT 10"},
    {XKB_KEY_XF86Switch_VT_11, "VT 11"},
    {XKB_KEY_XF86Switch_VT_12, "VT 12"},

    {XKB_KEY_Terminate_Server, "Terminate\nServer"},

    {XKB_KEY_ISO_Level5_Latch, "Level 5"},
};


bool
wb_xkb_mod_is_valid(wb_xkb_mod_t mod)
{
    return mod.keycode != XKB_KEYCODE_INVALID && mod.mod_index != XKB_MOD_INVALID;
}


int
wb_xkb_get_all_mods(const wb_xkb_t *xkb, wb_xkb_mod_t *mods_out, int mods_size)
{
    if (!xkb->keymap) {
        return -2;
    }

    int idx = 0;
    for (xkb_mod_index_t mod = 0; mod < xkb_keymap_num_mods(xkb->keymap); mod++) {
        for (xkb_keycode_t key = xkb_keymap_min_keycode(xkb->keymap);
             key <= xkb_keymap_max_keycode(xkb->keymap) && idx >= 0; key++) {

            struct xkb_state *const state = xkb_state_new(xkb->keymap);
            assert(state);

            // xkb_state_update_mask(
            //     state, xkb_state_serialize_mods(xkb->state, XKB_STATE_MODS_DEPRESSED),
            //     xkb_state_serialize_mods(xkb->state, XKB_STATE_MODS_LATCHED),
            //     xkb_state_serialize_mods(xkb->state, XKB_STATE_MODS_LOCKED),
            //     xkb_state_serialize_layout(xkb->state, XKB_STATE_LAYOUT_DEPRESSED),
            //     xkb_state_serialize_layout(xkb->state, XKB_STATE_LAYOUT_LATCHED),
            //     xkb_state_serialize_layout(xkb->state, XKB_STATE_LAYOUT_LOCKED));
            //
            // xkb_mod_mask_t s1 = xkb_state_serialize_mods(state, XKB_STATE_MODS_EFFECTIVE);
            // xkb_state_update_key(state, key, XKB_KEY_DOWN);
            // xkb_mod_mask_t s2 = xkb_state_serialize_mods(state, XKB_STATE_MODS_EFFECTIVE);
            // xkb_state_update_key(state, key, XKB_KEY_UP);
            // xkb_mod_mask_t s3 = xkb_state_serialize_mods(state, XKB_STATE_MODS_EFFECTIVE);
            // if ((s1 != s2) || (s2 != s3) || (s1 != s3)) {
            //     if (idx < mods_size) {
            //         mods_out[idx++] = (wb_xkb_key_mod_t){
            //             .keycode = key,
            //             .mod_index = mod,
            //         };
            //     } else {
            //         idx = -3;
            //     }
            // }


            if ((xkb_state_update_key(state, key, XKB_KEY_DOWN) &&
                 xkb_state_mod_index_is_active(state, mod, XKB_STATE_MODS_EFFECTIVE)) ||
                (xkb_state_update_key(state, key, XKB_KEY_UP) &&
                 xkb_state_mod_index_is_active(state, mod, XKB_STATE_MODS_EFFECTIVE))) {
                if (idx < mods_size) {
                    mods_out[idx++] = (wb_xkb_mod_t){
                        .keycode = key,
                        .mod_index = mod,
                    };
                } else {
                    idx = -3;
                }
            }

            xkb_state_unref(state);
        }
    }
    return idx;
}


/* derived from:
 * https://xkbcommon.org/doc/current/faq.html#how-to-get-the-keys-that-trigger-modifiers
 * Get key codes where they are effective when pressed and not effective when released
 */
int
wb_xkb_get_mods(const wb_xkb_t *xkb, wb_xkb_mod_t *mods_out, int mods_size)
{
    if (!xkb->keymap) {
        return -2;
    }

    int idx = 0;
    for (xkb_mod_index_t mod = 0; mod < xkb_keymap_num_mods(xkb->keymap); mod++) {
        for (xkb_keycode_t key = xkb_keymap_min_keycode(xkb->keymap);
             key <= xkb_keymap_max_keycode(xkb->keymap) && idx >= 0; key++) {

            struct xkb_state *const state = xkb_state_new(xkb->keymap);
            assert(state);

            if ((xkb_state_update_key(state, key, XKB_KEY_DOWN) &&
                 xkb_state_mod_index_is_active(state, mod, XKB_STATE_MODS_EFFECTIVE)) &&
                (xkb_state_update_key(state, key, XKB_KEY_UP) &&
                 !xkb_state_mod_index_is_active(state, mod, XKB_STATE_MODS_EFFECTIVE))) {
                if (idx < mods_size) {
                    // bool exists = false;
                    // for (int i = 0; i < idx; i++) {
                    //     if (mods_out[i].keycode == key) {
                    //         exists = true;
                    //         break;
                    //     }
                    // }
                    // if (!exists) {
                    //     mods_out[idx++] = (wb_xkb_key_mod_t){
                    //         .keycode = key,
                    //         .mod_index = mod,
                    //     };
                    // }
                    mods_out[idx++] = (wb_xkb_mod_t){
                        .keycode = key,
                        .mod_index = mod,
                    };
                } else {
                    idx = -3;
                }
            }

            xkb_state_unref(state);
        }
    }
    return idx;
}


bool
wb_xkb_mod_depressed(const wb_xkb_t *xkb, wb_xkb_mod_t mod)
{
    return xkb_state_mod_index_is_active(xkb->state, mod.mod_index, XKB_STATE_MODS_DEPRESSED) == 1;
}

bool
wb_xkb_mod_latched(const wb_xkb_t *xkb, wb_xkb_mod_t mod)
{
    return xkb_state_mod_index_is_active(xkb->state, mod.mod_index, XKB_STATE_MODS_LATCHED) == 1;
}

bool
wb_xkb_mod_locked(const wb_xkb_t *xkb, wb_xkb_mod_t mod)
{
    return xkb_state_mod_index_is_active(xkb->state, mod.mod_index, XKB_STATE_MODS_LOCKED) == 1;
}


bool
wb_xkb_mod_effective(const wb_xkb_t *xkb, wb_xkb_mod_t mod)
{
    return xkb_state_mod_index_is_active(xkb->state, mod.mod_index, XKB_STATE_MODS_EFFECTIVE) == 1;
}


int
wb_xkb_name_to_evdev(const wb_xkb_t *xkb, const char *name)
{
    xkb_keycode_t code = xkb_keymap_key_by_name(xkb->keymap, name);
    if (code == XKB_KEYCODE_INVALID || code < 8) {
        fprintf(stderr, "xkb_name_to_evdev: invalid keycode, key: %s\n", name);
        return -1;
    }

    return (int)(code - 8);
}


xkb_keycode_t
wb_xkb_get_keycode(const wb_xkb_t *xkb, const char *name)
{
    if (!xkb->keymap)
        return XKB_KEYCODE_INVALID;

    xkb_keycode_t keycode = xkb_keymap_key_by_name(xkb->keymap, name);
    if (keycode == XKB_KEYCODE_INVALID) {
        fprintf(stderr, "xkb_get_keycode: invalid keycode, key: %s\n", name);
    }

    return keycode;
}


xkb_keysym_t
wb_xkb_get_sym(const wb_xkb_t *xkb, const char *name)
{
    if (!xkb->keymap || !xkb->state)
        return XKB_KEY_NoSymbol;

    xkb_keycode_t keycode = xkb_keymap_key_by_name(xkb->keymap, name);
    if (keycode == XKB_KEYCODE_INVALID) {
        fprintf(stderr, "xkb_get_sym: invalid keycode, key: %s\n", name);
        return XKB_KEY_NoSymbol;
    }

    xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb->state, keycode);
    if (sym == XKB_KEY_NoSymbol) {
        fprintf(stderr, "xkb_get_sym: no symbol, key: %s\n", name);
    }

    return sym;
}


int
wb_xkb_get_symname(const wb_xkb_t *xkb, const char *name, char *buffer, int size)
{
    if (!xkb->keymap || !xkb->state)
        return -2;

    xkb_keycode_t keycode = xkb_keymap_key_by_name(xkb->keymap, name);
    if (keycode == XKB_KEYCODE_INVALID) {
        fprintf(stderr, "xkb_get_symname: invalid keycode, key: %s\n", name);
        return -3;
    }

    xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb->state, keycode);
    if (sym == XKB_KEY_NoSymbol) {
        fprintf(stderr, "xkb_get_symname: no symbol, key: %s\n", name);
        return -4;
    }

    int bytes_written = xkb_keysym_get_name(sym, buffer, size);
    if (bytes_written == -1) {
        fprintf(stderr, "xkb_get_symname: cannot get symname, key: %s\n", name);
    }

    return bytes_written;
}


int
wb_xkb_get_utf8(const wb_xkb_t *xkb, const char *name, char *buffer, int size)
{
    if (!xkb->keymap || !xkb->state)
        return -2;

    xkb_keycode_t keycode = xkb_keymap_key_by_name(xkb->keymap, name);
    if (keycode == XKB_KEYCODE_INVALID) {
        fprintf(stderr, "xkb_get_utf8: invalid keycode, key: %s\n", name);
        return -3;
    }

    xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb->state, keycode);
    if (sym == XKB_KEY_NoSymbol) {
        fprintf(stderr, "xkb_get_utf8: no symbol, key: %s\n", name);
        return -4;
    }

    int bytes_written = xkb_keysym_to_utf8(sym, buffer, size);
    if (bytes_written == -1) {
        fprintf(stderr, "xkb_get_utf8: buffer too small, key: %s\n", name);
    }

    return bytes_written;
}


int
wb_xkb_get_title(const wb_xkb_t *xkb, const char *name, char *buffer, int size)
{
    if (!xkb->keymap || !xkb->state)
        return -2;

    xkb_keycode_t keycode = xkb_keymap_key_by_name(xkb->keymap, name);
    if (keycode == XKB_KEYCODE_INVALID) {
        fprintf(stderr, "xkb_get_title: invalid keycode, key: %s\n", name);
        return -3;
    }

    xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb->state, keycode);
    if (sym == XKB_KEY_NoSymbol) {
        // fprintf(stderr, "xkb_get_title: no symbol, key: %s\n", name);
        return -4;
    }

    int n = sizeof(KEY_TITLE_OVERRIDES) / sizeof(KEY_TITLE_OVERRIDES[0]);
    bool overridden = false;
    for (int i = 0; i < n; i++) {
        if (KEY_TITLE_OVERRIDES[i].sym == sym) {
            strncpy(buffer, KEY_TITLE_OVERRIDES[i].title, size - 1);
            buffer[size - 1] = '\0';
            overridden = true;
            break;
        }
    }

    if (!overridden) {
        int bytes_written = xkb_keysym_to_utf8(sym, buffer, size);
        if (bytes_written == -1) {
            fprintf(stderr, "xkb_get_title: buffer too small, key: %s\n", name);
        } else if (bytes_written == 0) {
            bytes_written = xkb_keysym_get_name(sym, buffer, size);
            if (bytes_written == -1) {
                fprintf(stderr, "xkb_get_title: keysym is invalid, key: %s\n", name);
            }
        }
        return bytes_written;
    }

    return true;
}
