#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h"
#include "inih/ini.h"
#include "util.h"

// display parameters
#define CONFIG_FILE "profile.ini"
#define FULLSCREEN 0
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

// key bindings
#define CRAFT_KEY_FORWARD 'W'
#define CRAFT_KEY_BACKWARD 'S'
#define CRAFT_KEY_LEFT 'A'
#define CRAFT_KEY_RIGHT 'D'

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
        if (strncmp(name, "forward", 7) == 0) {
            if (strncicmp(value, "up", 2) == 0) {
                pconfig->forward = GLFW_KEY_UP;
            } else {
                pconfig->forward = toupper(value[0]);
            }
            handled = 1;
        } else if (strncmp(name, "backward", 8) == 0) {
            if (strncicmp(value, "down", 4) == 0) {
                pconfig->backward = GLFW_KEY_DOWN;
            } else {
                pconfig->backward = toupper(value[0]);
            }
            handled = 1;
        } else if (strncmp(name, "left", 4) == 0) {
            if (strncicmp(value, "left", 4) == 0) {
                pconfig->strafe_left = GLFW_KEY_LEFT;
            } else {
                pconfig->strafe_left = toupper(value[0]);
            }
            handled = 1;
        } else if (strncmp(name, "right", 5) == 0) {
            if (strncicmp(value, "right", 5) == 0) {
                pconfig->strafe_right = GLFW_KEY_RIGHT;
            } else {
                pconfig->strafe_right = toupper(value[0]);
            }
            handled = 1;
        }
    }

    if (!handled) {
        printf("Unknown configuration: %s:%s = %s\n",
                section, name, value);
    }

    return 1;
}

int configure(configuration *config) {
    int res = ini_parse(CONFIG_FILE, handler, config);
    if (res < 0) {
        printf("can't load '%s'\n", CONFIG_FILE);
    } else if (res) {
        printf("parse error in %s on line %d.\n", CONFIG_FILE, res);
    }
    return res == 0;
}
