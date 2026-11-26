import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass

# resources:
# - ISO-IEC_9995: https://en.wikipedia.org/wiki/ISO/IEC_9995#Key_arrangements
# - keycodes: /usr/share/xkeyboard-config-2/keycodes/evdev
# - layout examples: https://commons.wikimedia.org/wiki/Category:SVG_keyboard_layouts


BASE_WIDTH = 32
KEY_WIDTH = BASE_WIDTH
KEY_HEIGHT = BASE_WIDTH

SPACE = 1
SPACE_X = SPACE
SPACE_Y = SPACE

ALPHANUM_WIDTH = 15 * KEY_WIDTH + 14 * SPACE_X

FN_GROUP_SPACE_X = (ALPHANUM_WIDTH - 13 * KEY_WIDTH - 12 * SPACE_X) / 3
ESC_SPACE = FN_GROUP_SPACE_X + SPACE_X
FN_SPACE_Y = 3 * SPACE_X

BKSP_BIG_WIDTH = 2 * KEY_WIDTH + SPACE_X
BKSP_SMALL_WIDTH = BASE_WIDTH
TAB_WIDTH = 1.5 * KEY_WIDTH
CAPS_WIDTH = 1.75 * KEY_WIDTH
ISO_ENTER_WIDTH = 3 * KEY_WIDTH + SPACE_X - TAB_WIDTH
ANSI_ENTER_WIDTH = 4 * KEY_WIDTH + 2 * SPACE_X - CAPS_WIDTH
LFSH_BIG_WIDTH = 2.25 * KEY_WIDTH
RTSH_BIG_WIDTH = 5 * KEY_WIDTH + 3 * SPACE_X - LFSH_BIG_WIDTH
LFSH_SMALL_WIDTH = 4 * KEY_WIDTH + 2 * SPACE_X - RTSH_BIG_WIDTH
RTSH_SMALL_WIDTH = 3 * KEY_WIDTH + 1 * SPACE_X - LFSH_SMALL_WIDTH  # iso-b, iso-j
LCTL_WIDTH = 1.75 * KEY_WIDTH
LWIN_WIDTH = (3 * KEY_WIDTH + 2 * SPACE_X) / 2 - SPACE_X
LALT_WIDTH = (3 * KEY_WIDTH + 2 * SPACE_X) / 2 - SPACE_X
ALGR_WIDTH = 1.5 * KEY_WIDTH
RWIN_WIDTH = BASE_WIDTH
MENU_WIDTH = BASE_WIDTH
RCTL_BIG_WIDTH = LCTL_WIDTH
RCTL_SMALL_WIDTH = 1.2 * KEY_WIDTH  # iso-j
SPCE_WIDTH = (
    ALPHANUM_WIDTH
    - 7 * SPACE_X
    - LCTL_WIDTH
    - LWIN_WIDTH
    - LALT_WIDTH
    - ALGR_WIDTH
    - RWIN_WIDTH
    - MENU_WIDTH
    - RCTL_BIG_WIDTH
)
SPCE_SMALLER_WIDTH = (
    ALPHANUM_WIDTH
    - 9 * SPACE_X
    - LCTL_WIDTH
    - LWIN_WIDTH
    - LALT_WIDTH
    - 3 * KEY_WIDTH
    - RWIN_WIDTH
    - MENU_WIDTH
    - RCTL_BIG_WIDTH
)
SPCE_SMALLEST_WIDTH = (
    ALPHANUM_WIDTH
    - 10 * SPACE_X
    - LCTL_WIDTH
    - LWIN_WIDTH
    - LALT_WIDTH
    - 4 * KEY_WIDTH
    - RWIN_WIDTH
    - MENU_WIDTH
    - RCTL_SMALL_WIDTH
)  # iso-j
BKSL_ANSI_WIDTH = 3 * KEY_WIDTH + SPACE_X - TAB_WIDTH

FILL_COLOR = "#ebebeb"


@dataclass
class Key:
    id: str
    width: float


ALPHA_KEYS = {
    "iso-a": [
        [
            Key("TLDE", KEY_WIDTH),
            Key("AE01", KEY_WIDTH),
            Key("AE02", KEY_WIDTH),
            Key("AE03", KEY_WIDTH),
            Key("AE04", KEY_WIDTH),
            Key("AE05", KEY_WIDTH),
            Key("AE06", KEY_WIDTH),
            Key("AE07", KEY_WIDTH),
            Key("AE08", KEY_WIDTH),
            Key("AE09", KEY_WIDTH),
            Key("AE10", KEY_WIDTH),
            Key("AE11", KEY_WIDTH),
            Key("AE12", KEY_WIDTH),
            Key("BKSP", BKSP_BIG_WIDTH),
        ],
        [
            Key("TAB", TAB_WIDTH),
            Key("AD01", KEY_WIDTH),
            Key("AD02", KEY_WIDTH),
            Key("AD03", KEY_WIDTH),
            Key("AD04", KEY_WIDTH),
            Key("AD05", KEY_WIDTH),
            Key("AD06", KEY_WIDTH),
            Key("AD07", KEY_WIDTH),
            Key("AD08", KEY_WIDTH),
            Key("AD09", KEY_WIDTH),
            Key("AD10", KEY_WIDTH),
            Key("AD11", KEY_WIDTH),
            Key("AD12", KEY_WIDTH),
            Key("BKSL", BKSL_ANSI_WIDTH),
        ],
        [
            Key("CAPS", CAPS_WIDTH),
            Key("AC01", KEY_WIDTH),
            Key("AC02", KEY_WIDTH),
            Key("AC03", KEY_WIDTH),
            Key("AC04", KEY_WIDTH),
            Key("AC05", KEY_WIDTH),
            Key("AC06", KEY_WIDTH),
            Key("AC07", KEY_WIDTH),
            Key("AC08", KEY_WIDTH),
            Key("AC09", KEY_WIDTH),
            Key("AC10", KEY_WIDTH),
            Key("AC11", KEY_WIDTH),
            Key("RTRN", ANSI_ENTER_WIDTH),
        ],
        [
            Key("LFSH", LFSH_BIG_WIDTH),
            Key("AB01", KEY_WIDTH),
            Key("AB02", KEY_WIDTH),
            Key("AB03", KEY_WIDTH),
            Key("AB04", KEY_WIDTH),
            Key("AB05", KEY_WIDTH),
            Key("AB06", KEY_WIDTH),
            Key("AB07", KEY_WIDTH),
            Key("AB08", KEY_WIDTH),
            Key("AB09", KEY_WIDTH),
            Key("AB10", KEY_WIDTH),
            Key("RTSH", RTSH_BIG_WIDTH),
        ],
        [
            Key("LCTL", LCTL_WIDTH),
            Key("LWIN", LWIN_WIDTH),
            Key("LALT", LALT_WIDTH),
            Key("SPCE", SPCE_WIDTH),
            Key("ALGR", ALGR_WIDTH),
            Key("RWIN", RWIN_WIDTH),
            Key("MENU", MENU_WIDTH),
            Key("RCTL", RCTL_BIG_WIDTH),
        ],
    ],
    "iso-e": [
        [
            Key("TLDE", KEY_WIDTH),
            Key("AE01", KEY_WIDTH),
            Key("AE02", KEY_WIDTH),
            Key("AE03", KEY_WIDTH),
            Key("AE04", KEY_WIDTH),
            Key("AE05", KEY_WIDTH),
            Key("AE06", KEY_WIDTH),
            Key("AE07", KEY_WIDTH),
            Key("AE08", KEY_WIDTH),
            Key("AE09", KEY_WIDTH),
            Key("AE10", KEY_WIDTH),
            Key("AE11", KEY_WIDTH),
            Key("AE12", KEY_WIDTH),
            Key("BKSP", BKSP_BIG_WIDTH),
        ],
        [
            Key("TAB", TAB_WIDTH),
            Key("AD01", KEY_WIDTH),
            Key("AD02", KEY_WIDTH),
            Key("AD03", KEY_WIDTH),
            Key("AD04", KEY_WIDTH),
            Key("AD05", KEY_WIDTH),
            Key("AD06", KEY_WIDTH),
            Key("AD07", KEY_WIDTH),
            Key("AD08", KEY_WIDTH),
            Key("AD09", KEY_WIDTH),
            Key("AD10", KEY_WIDTH),
            Key("AD11", KEY_WIDTH),
            Key("AD12", KEY_WIDTH),
        ],
        [
            Key("CAPS", CAPS_WIDTH),
            Key("AC01", KEY_WIDTH),
            Key("AC02", KEY_WIDTH),
            Key("AC03", KEY_WIDTH),
            Key("AC04", KEY_WIDTH),
            Key("AC05", KEY_WIDTH),
            Key("AC06", KEY_WIDTH),
            Key("AC07", KEY_WIDTH),
            Key("AC08", KEY_WIDTH),
            Key("AC09", KEY_WIDTH),
            Key("AC10", KEY_WIDTH),
            Key("AC11", KEY_WIDTH),
            Key("AC12", KEY_WIDTH),
            Key("RTRN", ISO_ENTER_WIDTH),
        ],
        [
            Key("LFSH", LFSH_SMALL_WIDTH),
            Key("LSGT", KEY_WIDTH),
            Key("AB01", KEY_WIDTH),
            Key("AB02", KEY_WIDTH),
            Key("AB03", KEY_WIDTH),
            Key("AB04", KEY_WIDTH),
            Key("AB05", KEY_WIDTH),
            Key("AB06", KEY_WIDTH),
            Key("AB07", KEY_WIDTH),
            Key("AB08", KEY_WIDTH),
            Key("AB09", KEY_WIDTH),
            Key("AB10", KEY_WIDTH),
            Key("RTSH", RTSH_BIG_WIDTH),
        ],
        [
            Key("LCTL", LCTL_WIDTH),
            Key("LWIN", LWIN_WIDTH),
            Key("LALT", LALT_WIDTH),
            Key("SPCE", SPCE_WIDTH),
            Key("ALGR", ALGR_WIDTH),
            Key("RWIN", RWIN_WIDTH),
            Key("MENU", MENU_WIDTH),
            Key("RCTL", RCTL_BIG_WIDTH),
        ],
    ],
    "iso-b": [
        [
            Key("TLDE", KEY_WIDTH),
            Key("AE01", KEY_WIDTH),
            Key("AE02", KEY_WIDTH),
            Key("AE03", KEY_WIDTH),
            Key("AE04", KEY_WIDTH),
            Key("AE05", KEY_WIDTH),
            Key("AE06", KEY_WIDTH),
            Key("AE07", KEY_WIDTH),
            Key("AE08", KEY_WIDTH),
            Key("AE09", KEY_WIDTH),
            Key("AE10", KEY_WIDTH),
            Key("AE11", KEY_WIDTH),
            Key("AE12", KEY_WIDTH),
            Key("BKSP", BKSP_BIG_WIDTH),
        ],
        [
            Key("TAB", TAB_WIDTH),
            Key("AD01", KEY_WIDTH),
            Key("AD02", KEY_WIDTH),
            Key("AD03", KEY_WIDTH),
            Key("AD04", KEY_WIDTH),
            Key("AD05", KEY_WIDTH),
            Key("AD06", KEY_WIDTH),
            Key("AD07", KEY_WIDTH),
            Key("AD08", KEY_WIDTH),
            Key("AD09", KEY_WIDTH),
            Key("AD10", KEY_WIDTH),
            Key("AD11", KEY_WIDTH),
            Key("AD12", KEY_WIDTH),
        ],
        [
            Key("CAPS", CAPS_WIDTH),
            Key("AC01", KEY_WIDTH),
            Key("AC02", KEY_WIDTH),
            Key("AC03", KEY_WIDTH),
            Key("AC04", KEY_WIDTH),
            Key("AC05", KEY_WIDTH),
            Key("AC06", KEY_WIDTH),
            Key("AC07", KEY_WIDTH),
            Key("AC08", KEY_WIDTH),
            Key("AC09", KEY_WIDTH),
            Key("AC10", KEY_WIDTH),
            Key("AC11", KEY_WIDTH),
            Key("AC12", KEY_WIDTH),
            Key("RTRN", ISO_ENTER_WIDTH),
        ],
        [
            Key("LFSH", LFSH_SMALL_WIDTH),
            Key("LSGT", KEY_WIDTH),
            Key("AB01", KEY_WIDTH),
            Key("AB02", KEY_WIDTH),
            Key("AB03", KEY_WIDTH),
            Key("AB04", KEY_WIDTH),
            Key("AB05", KEY_WIDTH),
            Key("AB06", KEY_WIDTH),
            Key("AB07", KEY_WIDTH),
            Key("AB08", KEY_WIDTH),
            Key("AB09", KEY_WIDTH),
            Key("AB10", KEY_WIDTH),
            Key("AB11", KEY_WIDTH),
            Key("RTSH", RTSH_SMALL_WIDTH),
        ],
        [
            Key("LCTL", LCTL_WIDTH),
            Key("LWIN", LWIN_WIDTH),
            Key("LALT", LALT_WIDTH),
            Key("SPCE", SPCE_WIDTH),
            Key("ALGR", ALGR_WIDTH),
            Key("RWIN", RWIN_WIDTH),
            Key("MENU", MENU_WIDTH),
            Key("RCTL", RCTL_BIG_WIDTH),
        ],
    ],
    "iso-l": [
        [
            Key("TLDE", KEY_WIDTH),
            Key("AE01", KEY_WIDTH),
            Key("AE02", KEY_WIDTH),
            Key("AE03", KEY_WIDTH),
            Key("AE04", KEY_WIDTH),
            Key("AE05", KEY_WIDTH),
            Key("AE06", KEY_WIDTH),
            Key("AE07", KEY_WIDTH),
            Key("AE08", KEY_WIDTH),
            Key("AE09", KEY_WIDTH),
            Key("AE10", KEY_WIDTH),
            Key("AE11", KEY_WIDTH),
            Key("AE12", KEY_WIDTH),
            Key("AE13", KEY_WIDTH),
            Key("BKSP", BKSP_SMALL_WIDTH),
        ],
        [
            Key("TAB", TAB_WIDTH),
            Key("AD01", KEY_WIDTH),
            Key("AD02", KEY_WIDTH),
            Key("AD03", KEY_WIDTH),
            Key("AD04", KEY_WIDTH),
            Key("AD05", KEY_WIDTH),
            Key("AD06", KEY_WIDTH),
            Key("AD07", KEY_WIDTH),
            Key("AD08", KEY_WIDTH),
            Key("AD09", KEY_WIDTH),
            Key("AD10", KEY_WIDTH),
            Key("AD11", KEY_WIDTH),
            Key("AD12", KEY_WIDTH),
        ],
        [
            Key("CAPS", CAPS_WIDTH),
            Key("AC01", KEY_WIDTH),
            Key("AC02", KEY_WIDTH),
            Key("AC03", KEY_WIDTH),
            Key("AC04", KEY_WIDTH),
            Key("AC05", KEY_WIDTH),
            Key("AC06", KEY_WIDTH),
            Key("AC07", KEY_WIDTH),
            Key("AC08", KEY_WIDTH),
            Key("AC09", KEY_WIDTH),
            Key("AC10", KEY_WIDTH),
            Key("AC11", KEY_WIDTH),
            Key("RTRN", ISO_ENTER_WIDTH),
        ],
        [
            Key("LFSH", LFSH_BIG_WIDTH),
            Key("AB01", KEY_WIDTH),
            Key("AB02", KEY_WIDTH),
            Key("AB03", KEY_WIDTH),
            Key("AB04", KEY_WIDTH),
            Key("AB05", KEY_WIDTH),
            Key("AB06", KEY_WIDTH),
            Key("AB07", KEY_WIDTH),
            Key("AB08", KEY_WIDTH),
            Key("AB09", KEY_WIDTH),
            Key("AB10", KEY_WIDTH),
            Key("RTSH", RTSH_BIG_WIDTH),
        ],
        [
            Key("LCTL", LCTL_WIDTH),
            Key("LWIN", LWIN_WIDTH),
            Key("LALT", LALT_WIDTH),
            Key("SPCE", SPCE_WIDTH),
            Key("ALGR", ALGR_WIDTH),
            Key("RWIN", RWIN_WIDTH),
            Key("MENU", MENU_WIDTH),
            Key("RCTL", RCTL_BIG_WIDTH),
        ],
    ],
    "iso-k": [
        [
            Key("TLDE", KEY_WIDTH),
            Key("AE01", KEY_WIDTH),
            Key("AE02", KEY_WIDTH),
            Key("AE03", KEY_WIDTH),
            Key("AE04", KEY_WIDTH),
            Key("AE05", KEY_WIDTH),
            Key("AE06", KEY_WIDTH),
            Key("AE07", KEY_WIDTH),
            Key("AE08", KEY_WIDTH),
            Key("AE09", KEY_WIDTH),
            Key("AE10", KEY_WIDTH),
            Key("AE11", KEY_WIDTH),
            Key("AE12", KEY_WIDTH),
            Key("AE13", KEY_WIDTH),
            Key("BKSP", BKSP_SMALL_WIDTH),
        ],
        [
            Key("TAB", TAB_WIDTH),
            Key("AD01", KEY_WIDTH),
            Key("AD02", KEY_WIDTH),
            Key("AD03", KEY_WIDTH),
            Key("AD04", KEY_WIDTH),
            Key("AD05", KEY_WIDTH),
            Key("AD06", KEY_WIDTH),
            Key("AD07", KEY_WIDTH),
            Key("AD08", KEY_WIDTH),
            Key("AD09", KEY_WIDTH),
            Key("AD10", KEY_WIDTH),
            Key("AD11", KEY_WIDTH),
            Key("AD12", KEY_WIDTH),
        ],
        [
            Key("CAPS", CAPS_WIDTH),
            Key("AC01", KEY_WIDTH),
            Key("AC02", KEY_WIDTH),
            Key("AC03", KEY_WIDTH),
            Key("AC04", KEY_WIDTH),
            Key("AC05", KEY_WIDTH),
            Key("AC06", KEY_WIDTH),
            Key("AC07", KEY_WIDTH),
            Key("AC08", KEY_WIDTH),
            Key("AC09", KEY_WIDTH),
            Key("AC10", KEY_WIDTH),
            Key("AC11", KEY_WIDTH),
            Key("RTRN", ISO_ENTER_WIDTH),
        ],
        [
            Key("LFSH", LFSH_BIG_WIDTH),
            Key("AB01", KEY_WIDTH),
            Key("AB02", KEY_WIDTH),
            Key("AB03", KEY_WIDTH),
            Key("AB04", KEY_WIDTH),
            Key("AB05", KEY_WIDTH),
            Key("AB06", KEY_WIDTH),
            Key("AB07", KEY_WIDTH),
            Key("AB08", KEY_WIDTH),
            Key("AB09", KEY_WIDTH),
            Key("AB10", KEY_WIDTH),
            Key("RTSH", RTSH_BIG_WIDTH),
        ],
        [
            Key("LCTL", LCTL_WIDTH),
            Key("LWIN", LWIN_WIDTH),
            Key("LALT", LALT_WIDTH),
            Key("HNGL", KEY_WIDTH),
            Key("SPCE", SPCE_SMALLER_WIDTH),
            Key("HJCV", KEY_WIDTH),
            Key("ALGR", KEY_WIDTH),
            Key("RWIN", RWIN_WIDTH),
            Key("MENU", MENU_WIDTH),
            Key("RCTL", RCTL_BIG_WIDTH),
        ],
    ],
    "iso-j": [
        [
            Key("TLDE", KEY_WIDTH),
            Key("AE01", KEY_WIDTH),
            Key("AE02", KEY_WIDTH),
            Key("AE03", KEY_WIDTH),
            Key("AE04", KEY_WIDTH),
            Key("AE05", KEY_WIDTH),
            Key("AE06", KEY_WIDTH),
            Key("AE07", KEY_WIDTH),
            Key("AE08", KEY_WIDTH),
            Key("AE09", KEY_WIDTH),
            Key("AE10", KEY_WIDTH),
            Key("AE11", KEY_WIDTH),
            Key("AE12", KEY_WIDTH),
            Key("AE13", KEY_WIDTH),
            Key("BKSP", BKSP_SMALL_WIDTH),
        ],
        [
            Key("TAB", TAB_WIDTH),
            Key("AD01", KEY_WIDTH),
            Key("AD02", KEY_WIDTH),
            Key("AD03", KEY_WIDTH),
            Key("AD04", KEY_WIDTH),
            Key("AD05", KEY_WIDTH),
            Key("AD06", KEY_WIDTH),
            Key("AD07", KEY_WIDTH),
            Key("AD08", KEY_WIDTH),
            Key("AD09", KEY_WIDTH),
            Key("AD10", KEY_WIDTH),
            Key("AD11", KEY_WIDTH),
            Key("AD12", KEY_WIDTH),
        ],
        [
            Key("CAPS", CAPS_WIDTH),
            Key("AC01", KEY_WIDTH),
            Key("AC02", KEY_WIDTH),
            Key("AC03", KEY_WIDTH),
            Key("AC04", KEY_WIDTH),
            Key("AC05", KEY_WIDTH),
            Key("AC06", KEY_WIDTH),
            Key("AC07", KEY_WIDTH),
            Key("AC08", KEY_WIDTH),
            Key("AC09", KEY_WIDTH),
            Key("AC10", KEY_WIDTH),
            Key("AC11", KEY_WIDTH),
            Key("AC12", KEY_WIDTH),
            Key("RTRN", ISO_ENTER_WIDTH),
        ],
        [
            Key("LFSH", LFSH_BIG_WIDTH),
            Key("AB01", KEY_WIDTH),
            Key("AB02", KEY_WIDTH),
            Key("AB03", KEY_WIDTH),
            Key("AB04", KEY_WIDTH),
            Key("AB05", KEY_WIDTH),
            Key("AB06", KEY_WIDTH),
            Key("AB07", KEY_WIDTH),
            Key("AB08", KEY_WIDTH),
            Key("AB09", KEY_WIDTH),
            Key("AB10", KEY_WIDTH),
            Key("AB11", KEY_WIDTH),
            Key("RTSH", RTSH_SMALL_WIDTH),
        ],
        [
            Key("LCTL", LCTL_WIDTH),
            Key("LWIN", LWIN_WIDTH),
            Key("LALT", LALT_WIDTH),
            Key("MUHE", KEY_WIDTH),
            Key("SPCE", SPCE_SMALLEST_WIDTH),
            Key("HENK", KEY_WIDTH),
            Key("HIRA", KEY_WIDTH),
            Key("ALGR", KEY_WIDTH),
            Key("RWIN", RWIN_WIDTH),
            Key("MENU", MENU_WIDTH),
            Key("RCTL", RCTL_SMALL_WIDTH),
        ],
    ],
}


def make_rect_elem(id, x, y, width=KEY_WIDTH, height=KEY_HEIGHT, fill_color=FILL_COLOR):
    elem = ET.Element("rect")
    elem.attrib["id"] = id
    elem.attrib["x"] = str(x)
    elem.attrib["y"] = str(y)
    elem.attrib["width"] = str(width)
    elem.attrib["height"] = str(height)
    elem.attrib["fill"] = fill_color
    return elem


def make_iso_enter(x1, y1, x2, y2, width, height, fill_color=FILL_COLOR) -> ET.Element:
    #           width                            width
    # (x1,y1)->*──────┐            (x1,y1)----->*─────┐
    #          │      │                         │     │
    # (x2,y2)->└─*    │ height     (x2,y2)->*───┘     │ height
    #            │    │                     │         │
    #            └────┘                     └─────────┘
    elem = ET.Element("path")
    elem.attrib["id"] = "RTRN"
    d = f"m {x1} {y1} h {width} v {height} h {(x2-x1)-width} v {(y2-y1)-height} h {x1-x2} z"
    elem.attrib["d"] = d
    elem.attrib["fill"] = fill_color
    return elem


def add_alphanum(
    keys: list[list[Key]], root: ET.Element, x_offset: float, y_offset: float
):
    x = x_offset
    y = y_offset
    temp: list[list[ET.Element]] = []
    for row in keys:
        temp.append([])
        for key in row:
            elem = None
            if key.id == "RTRN" and key.width == ISO_ENTER_WIDTH:
                x1 = float(temp[-2][-1].attrib["x"]) + KEY_WIDTH + SPACE_X
                y1 = float(temp[-2][-1].attrib["y"])
                x2 = x
                y2 = y
                if x1 < x2:
                    y2 -= SPACE_Y
                elem = make_iso_enter(
                    x1, y1, x2, y2, key.width, KEY_HEIGHT * 2 + SPACE_X
                )
            else:
                elem = make_rect_elem(key.id, x, y, key.width)
            temp[-1].append(elem)
            x += key.width + SPACE_X
        x = x_offset
        y += KEY_HEIGHT + SPACE_Y

    for row in temp:
        for elem in row:
            root.append(elem)


def add_edit_fn_keys(root: ET.Element, alpha_width, x=0, y=0):
    root.append(make_rect_elem("ESC", x, y))
    x += KEY_WIDTH + ESC_SPACE
    for j in range(3):
        for i in range(4):
            root.append(make_rect_elem(f"FK{j*4+i+1:02}", x, y))
            x += KEY_WIDTH + SPACE_X
        x += FN_GROUP_SPACE_X

    x = alpha_width + FN_SPACE_Y
    root.append(make_rect_elem("PRSC", x, y))
    x += KEY_WIDTH + SPACE_X
    root.append(make_rect_elem("SCLK", x, y))
    x += KEY_WIDTH + SPACE_X
    root.append(make_rect_elem("PAUS", x, y))
    x += KEY_WIDTH + SPACE_X

    x = alpha_width + FN_SPACE_Y
    y += KEY_HEIGHT + FN_SPACE_Y
    root.append(make_rect_elem("INS", x, y))
    x += KEY_WIDTH + SPACE_X
    root.append(make_rect_elem("HOME", x, y))
    x += KEY_WIDTH + SPACE_X
    root.append(make_rect_elem("PGUP", x, y))
    x += KEY_WIDTH + SPACE_X

    x = alpha_width + FN_SPACE_Y
    y += KEY_HEIGHT + SPACE_Y
    root.append(make_rect_elem("DELE", x, y))
    x += KEY_WIDTH + SPACE_X
    root.append(make_rect_elem("END", x, y))
    x += KEY_WIDTH + SPACE_X
    root.append(make_rect_elem("PGDN", x, y))
    x += KEY_WIDTH + SPACE_X

    x = alpha_width + FN_SPACE_Y + KEY_WIDTH + SPACE_X
    y += (KEY_HEIGHT + SPACE_Y) * 2
    root.append(make_rect_elem("UP", x, y))

    x = alpha_width + FN_SPACE_Y
    y += KEY_HEIGHT + SPACE_Y
    root.append(make_rect_elem("LEFT", x, y))
    x += KEY_WIDTH + SPACE_X
    root.append(make_rect_elem("DOWN", x, y))
    x += KEY_WIDTH + SPACE_X
    root.append(make_rect_elem("RGHT", x, y))


def make_root_elem() -> ET.Element:
    root = ET.Element("svg")
    root.attrib["xmlns"] = "http://www.w3.org/2000/svg"
    return root


def write_tree(tree: ET.ElementTree, filename: str):
    ET.indent(tree)
    tree.write(filename)


def get_element_width(elem: ET.Element):
    width = 0
    for c in elem.iter():
        if c.tag == "rect":
            w = float(c.get("width", 0)) + float(c.get("x", 0))
            if w > width:
                width = w
    return width


def main():
    out_dir = "."
    if len(sys.argv) > 1:
        out_dir = sys.argv[1]
    for name, keys in ALPHA_KEYS.items():
        root = make_root_elem()
        add_alphanum(keys, root, 0, KEY_WIDTH + FN_SPACE_Y)
        alpha_width = get_element_width(root)
        add_edit_fn_keys(root, alpha_width)
        write_tree(ET.ElementTree(root), f"{out_dir}/{name}.svg")


if __name__ == "__main__":
    main()
