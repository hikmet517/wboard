#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool
config_save(const char *filename, const wb_config_t *config)
{
    FILE *file = fopen(filename, "w");
    if (!file) {
        return false;
    }

    int ret = fprintf(file,
                      "anchor=%s\n"
                      "width=%d\n"
                      "height=%d\n"
                      "layout=%s\n",
                      config->anchor, config->width, config->height, config->layout);

    if (ret < 0 || fclose(file) != 0) {
        return false;
    }

    return true;
}


static void
trim(char *str)
{
    // trim leading
    char *start = str;
    while (*start == ' ' || *start == '\t') {
        start++;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    // trim trailing
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r' ||
                       str[len - 1] == '\n')) {
        str[--len] = '\0';
    }
}


bool
config_load(const char *filename, wb_config_t *config)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        return false;
    }

    char line[CONFIG_LINE_MAX];
    wb_config_t tmp = *config;

    while (fgets(line, sizeof(line), file)) {
        // reject lines that exceed buffer.
        if (!strchr(line, '\n') && !feof(file)) {
            fclose(file);
            return false;
        }

        trim(line);

        // ignore empty lines and comments.
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        char *equals = strchr(line, '=');
        if (!equals) {
            continue;
        }

        *equals = '\0';

        char *key = line;
        char *value = equals + 1;

        trim(key);
        trim(value);

        if (strcmp(key, "anchor") == 0) {
            if (strlen(value) >= sizeof(tmp.anchor)) {
                goto error;
            }

            strcpy(tmp.anchor, value);
        } else if (strcmp(key, "width") == 0) {
            char *end;
            errno = 0;
            long v = strtol(value, &end, 10);

            if (errno || *end || v < 1 || v > 10000) {
                goto error;
            }

            tmp.width = (int)v;
        } else if (strcmp(key, "height") == 0) {
            char *end;
            errno = 0;
            long v = strtol(value, &end, 10);

            if (errno || *end || v < 1 || v > 10000) {
                goto error;
            }

            tmp.height = (int)v;
        } else if (strcmp(key, "layout") == 0) {
            if (strlen(value) >= sizeof(tmp.layout)) {
                goto error;
            }

            strcpy(tmp.layout, value);
        }
        // unknown keys are ignored.
    }

    if (ferror(file)) {
        goto error;
    }

    fclose(file);
    *config = tmp;

    return true;

error:
    fclose(file);
    return false;
}
