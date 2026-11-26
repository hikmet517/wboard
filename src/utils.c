#include "utils.h"
#include "main.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <linux/limits.h>


bool
dir_exists(const char *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0) {
        return S_ISDIR(stats.st_mode);
    }
    return false;
}


bool
get_layout_dir(char *path, int size)
{
    // search local dirs
    char temp[PATH_MAX];
    char *dirs[] = {".", "./build/data", "..", "../data"};
    for (int i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
        snprintf(temp, PATH_MAX - 1, "%s/layouts", dirs[i]);
        if (dir_exists(temp)) {
            strncpy(path, temp, size - 1);
            return true;
        }
    }

    // search xdg
    char default_xdg_data_dirs[] = "/usr/local/share/:/usr/share/";
    char *env_dirs = getenv("XDG_DATA_DIRS");
    char *dirs_to_use;

    if (env_dirs == NULL || env_dirs[0] == '\0') {
        dirs_to_use = default_xdg_data_dirs;
    } else {
        dirs_to_use = env_dirs;
    }

    char *token = strtok(dirs_to_use, ":");
    while (token != NULL) {
        snprintf(temp, PATH_MAX - 1, "%s/" APP_NAME "/layouts", token);
        if (dir_exists(temp)) {
            strncpy(path, temp, size - 1);
            return true;
        }
        token = strtok(NULL, ":");
    }

    return false;
}


bool
get_config_file(char *path, int size)
{
    const char *xdg = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");

    char temp[PATH_MAX];

    if (xdg && xdg[0] == '/') {
        snprintf(temp, sizeof(temp), "%s/" APP_NAME, xdg);
    } else {
        if (!home || home[0] != '/') {
            return false;
        }
        snprintf(temp, sizeof(temp), "%s/.config/" APP_NAME, home);
    }

    if (strlen(temp) >= (size_t)size) {
        return false;
    }

    // Create directory if necessary.
    if (mkdir(temp, 0700) != 0 && errno != EEXIST) {
        return false;
    }

    if (!dir_exists(temp)) {
        return false;
    }

    int len = snprintf(path, size, "%s/" APP_NAME ".conf", temp);

    if (len < 0 || len >= size)
        return false;

    return true;
}
