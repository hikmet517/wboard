#include "svg.h"
#include "key.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>

#include <stdbool.h>
#include <stdlib.h>

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


/* resources:
 * - https://gitlab.gnome.org/GNOME/libxml2
 * - https://gitlab.gnome.org/GNOME/libxml2/-/wikis/home
 * - https://gnome.pages.gitlab.gnome.org/libxml2/html/index.html
 * - https://gitlab.gnome.org/GNOME/libxml2/-/blob/master/example/tree1.c?ref_type=heads
 */


static const char *
skip_separators(const char *p)
{
    while (isspace((unsigned char)*p) || *p == ',') {
        p++;
    }
    return p;
}


static bool
read_number(const char **p, float *value)
{
    char *end;
    *p = skip_separators(*p);
    *value = strtof(*p, &end);

    if (end == *p) {
        return false;
    }

    *p = end;
    return true;
}


/*
 * Parses a subset of svg path. Only m, h, v, z commands are allowed.
 * Capacity must be 6 (or ENTER_NUM_OF_POINTS) to parse the shape of enter key.
 */
static int
parse_path(const char *d, wb_point_t *points, size_t capacity)
{
    size_t n = 0;
    float x = 0;
    float y = 0;
    float start_x = 0;
    float start_y = 0;
    char cmd = '\0';

    while (*(d = skip_separators(d))) {
        if (isalpha((unsigned char)*d)) {
            cmd = *d++;
        } else if (!cmd) {
            return -1;
        }

        switch (cmd) {
            case 'M':
            case 'm': {
                bool relative = cmd == 'm';

                if (!read_number(&d, &x) || !read_number(&d, &y)) {
                    return -1;
                }

                if (relative) {
                    x += n ? points[n - 1].x : 0;
                    y += n ? points[n - 1].y : 0;
                }

                start_x = x;
                start_y = y;

                if (n >= capacity) {
                    return -1;
                }

                points[n] = (wb_point_t){x, y};
                n++;

                // extra coordinate pairs after M/m imply L/l.
                cmd = relative ? 'l' : 'L';
                break;
            }

            case 'H':
            case 'h':
                if (!read_number(&d, &x)) {
                    return -1;
                }

                if (cmd == 'h')
                    x += n ? points[n - 1].x : 0;

                if (n >= capacity) {
                    return -1;
                }

                points[n] = (wb_point_t){x, n ? points[n - 1].y : y};
                n++;
                y = points[n - 1].y;
                break;

            case 'V':
            case 'v':
                if (!read_number(&d, &y)) {
                    return -1;
                }

                if (cmd == 'v')
                    y += n ? points[n - 1].y : 0;

                if (n >= capacity) {
                    return -1;
                }

                points[n] = (wb_point_t){n ? points[n - 1].x : x, y};
                n++;
                x = points[n - 1].x;
                break;

            case 'Z':
            case 'z':
                x = start_x;
                y = start_y;
                cmd = '\0';
                break;

            default:
                return -1;
        }
    }
    return n;
}

static bool
make_rect_shape(const xmlNode *node, wb_key_shape_t *shape)
{
    xmlChar *x = xmlGetProp(node, BAD_CAST("x"));
    xmlChar *y = xmlGetProp(node, BAD_CAST("y"));
    xmlChar *width = xmlGetProp(node, BAD_CAST("width"));
    xmlChar *height = xmlGetProp(node, BAD_CAST("height"));
    if (!x | !y | !width | !height) {
        return false;
    }
    *shape = wb_key_shape_rect_create(atof((char *)x), atof((char *)y), atof((char *)width),
                                      atof((char *)height));
    return true;
}


static bool
make_enter_shape(const xmlNode *node, wb_key_shape_t *shape)
{
    xmlChar *d = xmlGetProp(node, BAD_CAST("d"));
    if (!d) {
        return false;
    }
    wb_point_t points[ENTER_NUM_OF_POINTS];
    if (parse_path((const char *)d, points, ENTER_NUM_OF_POINTS) < 0) {
        return false;
    }
    *shape = wb_key_shape_enter_create(points);
    return true;
}


int
wb_svg_load(const char *filename, wb_key_t *keys, int capacity)
{
    xmlDoc *doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse %s\n", filename);
        goto error;
    }

    int n = 0;
    xmlNode *root = xmlDocGetRootElement(doc);
    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            xmlChar *id = xmlGetProp(node, BAD_CAST("id"));
            if (!id) {
                fprintf(stderr, "Invalid SVG, missing id: %s\n", filename);
                goto error;
            }

            wb_key_shape_t shape;
            if (xmlStrEqual(node->name, BAD_CAST("rect"))) {
                if (!make_rect_shape(node, &shape)) {
                    fprintf(stderr, "Invalid SVG, cannot parse rect: %s\n", filename);
                    goto error;
                }
            } else if (xmlStrEqual(node->name, BAD_CAST("path"))) {
                if (!make_enter_shape(node, &shape)) {
                    fprintf(stderr, "Invalid SVG, cannot parse enter: %s\n", filename);
                    goto error;
                }
            }
            if (n >= capacity) {
                goto error;
            }

            keys[n] = wb_key_create((const char *)id, shape);
            n++;
        }
    }

    xmlFreeDoc(doc);
    return n;

error:
    if (doc) {
        xmlFreeDoc(doc);
    }
    return -1;
}
