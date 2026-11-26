#ifndef WB_CONFIG_H
#define WB_CONFIG_H

#include <stdbool.h>

#define CONFIG_STRING_MAX 128
#define CONFIG_LINE_MAX 256


typedef struct wb_config {
    char anchor[CONFIG_STRING_MAX];
    int width;
    int height;
    char layout[CONFIG_STRING_MAX];
} wb_config_t;


bool
config_save(const char *filename, const wb_config_t *config);

bool
config_load(const char *filename, wb_config_t *config);


#endif /* WB_CONFIG_H */
