#ifndef WB_UTILS_H
#define WB_UTILS_H

#include <stdbool.h>


bool
dir_exists(const char *path);

bool
get_layout_dir(char *path, int size);

bool
get_config_file(char *path, int size);


#endif /* WB_UTILS_H */
