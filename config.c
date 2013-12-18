#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h"
#include "inih/ini.h"
#include "util.h"

static const char *config_file = "profile.ini";

/* Default-initializes configuration settings. */
static void default_config(configuration *config);

/* Finds the GLFW_KEY_* constant associated with *value in key_mapping above.
 * If val_len == 1, value[0] is returned.
 * Returns -1 if *value is not mapped.
 * Plain linear search, so watch the size of the mapping.
 */
static int get_key(const char *value, size_t val_len);

/* inih parse handler. */
static int handler(void *user, const char *section, const char *name,
        const char *value);

void default_config(configuration *config) {
    config->fullscreen = 0;
    config->width = 1024;
    config->height = 768;
    config->vsync = 1;
    config->forward = 'W';
    config->backward = 'S';
    config->strafe_left = 'A';
    config->strafe_right = 'D';
    config->quit = 'Q';
    config->jump = GLFW_KEY_SPACE;
    config->observe = 'O';
    config->observe_inset = 'P';
    config->fly = GLFW_KEY_TAB;
    config->cycle_block = 'E';
    config->zoom = GLFW_KEY_LEFT_SHIFT;
    config->ortho_view = 'F';
    config->x_inc = 'X';
    config->y_inc = 'V';
    config->z_inc = 'N';
    config->x_dec = 'Z';
    config->y_dec = 'C';
    config->z_dec = 'B';
    config->chat = 'T';
    config->command = '/';
    strncpy(config->name, "[unnamed]", MAX_NAME_LENGTH);
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
    { "Space", GLFW_KEY_SPACE },
    { "LShift", GLFW_KEY_LEFT_SHIFT },
    { "RShift", GLFW_KEY_RIGHT_SHIFT },
    { "LCtrl", GLFW_KEY_LEFT_CONTROL },
    { "RCtrl", GLFW_KEY_RIGHT_CONTROL },
    { "LAlt", GLFW_KEY_LEFT_ALT },
    { "RAlt", GLFW_KEY_RIGHT_ALT },
    { "Tab", GLFW_KEY_TAB },
    { "PgUp", GLFW_KEY_PAGE_UP },
    { "PgDown", GLFW_KEY_PAGE_DOWN },
    { "Home", GLFW_KEY_HOME },
    { "End", GLFW_KEY_END },
};

static const size_t key_mappings = sizeof(key_mapping) / sizeof(key_mapping[0]);

int get_key(const char *value, size_t val_len) {
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

int handler(void *user, const char *section, const char *name,
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
        } else if (strncmp(name, "vsync", 5) == 0) {
            pconfig->vsync = atoi(value);
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
            } else if (strncmp(name, "strafe left", 11) == 0) {
                pconfig->strafe_left = key;
                handled = 1;
            } else if (strncmp(name, "strafe right", 12) == 0) {
                pconfig->strafe_right = key;
                handled = 1;
            } else if (strncmp(name, "jump", 4) == 0) {
                pconfig->jump = key;
                handled = 1;
            } else if (strncmp(name, "observe", 7) == 0) {
                pconfig->observe = key;
                handled = 1;
            } else if (strncmp(name, "observe inset", 13) == 0) {
                pconfig->observe_inset = key;
                handled = 1;
            } else if (strncmp(name, "fly", 3) == 0) {
                pconfig->fly = key;
                handled = 1;
            } else if (strncmp(name, "cycle block", 11) == 0) {
                pconfig->cycle_block = key;
                handled = 1;
            } else if (strncmp(name, "zoom", 4) == 0) {
                pconfig->zoom = key;
                handled = 1;
            } else if (strncmp(name, "ortho view", 10) == 0) {
                pconfig->ortho_view = key;
                handled = 1;
            } else if (strncmp(name, "increase x", 10) == 0) {
                pconfig->x_inc = key;
                handled = 1;
            } else if (strncmp(name, "increase y", 10) == 0) {
                pconfig->y_inc = key;
                handled = 1;
            } else if (strncmp(name, "increase z", 10) == 0) {
                pconfig->z_inc = key;
                handled = 1;
            } else if (strncmp(name, "decrease x", 10) == 0) {
                pconfig->x_dec = key;
                handled = 1;
            } else if (strncmp(name, "decrease y", 10) == 0) {
                pconfig->y_dec = key;
                handled = 1;
            } else if (strncmp(name, "decrease z", 10) == 0) {
                pconfig->z_dec = key;
                handled = 1;
            } else if (strncmp(name, "chat", 4) == 0) {
                pconfig->chat = key;
                handled = 1;
            } else if (strncmp(name, "command", 7) == 0) {
                pconfig->command = key;
                handled = 1;
            } else if (strncmp(name, "quit", 4) == 0) {
                pconfig->quit = key;
                handled = 1;
            }
        }
    } else if (strncmp(section, "player", 6) == 0) {
        if (strncmp(name, "name", 4) == 0) {
            if (strncicmp(value, "[unnamed]", 9)) {
                strncpy(pconfig->name, value, MAX_NAME_LENGTH);
            }
            handled = 1;
        }
    }

    if (!handled) {
        fprintf(stderr, "Unknown configuration: %s:%s = %s.\n",
                section, name, value);
    }

    return 1;
}

int configure(configuration *config) {
    default_config(config);
    int res = ini_parse(config_file, handler, config);
    if (res < 0) {
        fprintf(stderr, "Can't load %s, using default settings.\n", config_file);
    } else if (res) {
        fprintf(stderr, "Parse error in %s on line %d.\n", config_file, res);
    }
    return res > 0;
}
