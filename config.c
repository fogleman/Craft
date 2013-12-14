#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h"
#include "inih/ini.h"
#include "util.h"

static const char *config_file = "profile.ini";

void default_config(configuration *config) {
    config->fullscreen = 0;
    config->width = 1024;
    config->height = 768;

    config->forward = 'W';
    config->backward = 'S';
    config->strafe_left = 'A';
    config->strafe_right = 'D';
}

/* *alias is the entry value in the configuration file.
 * key is the GLFW_KEY_* constant to map to *alias.
 */
typedef struct {
    const char *alias;
    int key;
} key_map;

static const key_map key_mapping[] = {
    { "Up", GLFW_KEY_UP },
    { "Down", GLFW_KEY_DOWN },
    { "Left", GLFW_KEY_LEFT },
    { "Right", GLFW_KEY_RIGHT },
};

static const size_t key_mappings = sizeof(key_mapping) / sizeof(key_mapping[0]);

/* Finds the GLFW_KEY_* constant associated with *value in key_mapping above.
 * If val_len == 1, value[0] is returned.
 * Returns -1 if *value is not mapped.
 * Plain linear search, so watch the size of the mapping.
 */
static int get_key(const char *value, size_t val_len) {
    if (val_len == 1) {
        return value[0];
    }
    for (size_t i = 0; i < key_mappings; ++i) {
        if (strncicmp(value, key_mapping[i].alias, val_len) == 0) {
            return key_mapping[i].key;
        }
    }
    return -1;
}

static int handler(void *user, const char *section, const char *name,
        const char *value) {
    configuration *pconfig = (configuration*)user;

    if (!*value) {
        return 0;
    }

    int handled = 0;
    if (strncmp(section, "video", 5) == 0) {
        if (strncmp(name, "fullscreen", 10) == 0) {
            pconfig->fullscreen = atoi(value);
            handled = 1;
        } else if (strncmp(name, "width", 5) == 0) {
            pconfig->width = atoi(value);
            handled = 1;
        } else if (strncmp(name, "height", 6) == 0) {
            pconfig->height = atoi(value);
            handled = 1;
        }
    } else if (strncmp(section, "controls", 8) == 0) {
        int key = get_key(value, strlen(value));
        if (key != -1) {
            if (strncmp(name, "forward", 7) == 0) {
                pconfig->forward = key;
                handled = 1;
            } else if (strncmp(name, "backward", 8) == 0) {
                pconfig->backward = key;
                handled = 1;
            } else if (strncmp(name, "left", 4) == 0) {
                pconfig->strafe_left = key;
                handled = 1;
            } else if (strncmp(name, "right", 5) == 0) {
                pconfig->strafe_right = key;
                handled = 1;
            }
        }
    }

    if (!handled) {
        printf("Unknown configuration: %s:%s = %s\n",
                section, name, value);
    }

    return 1;
}

int configure(configuration *config) {
    default_config(config);
    int res = ini_parse(config_file, handler, config);
    if (res < 0) {
        printf("Can't load %s, using default settings.\n", config_file);
    } else if (res) {
        printf("Parse error in %s on line %d.\n", config_file, res);
    }
    return res == 0;
}
