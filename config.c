#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "inih/ini.h"

// display parameters
#define CONFIG_FILE "profile.ini"
#define FULLSCREEN 0
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

static int handler(void* user, const char* section, const char* name,
        const char* value) {
    configuration* pconfig = (configuration*)user;

    if (strncmp(section, "video", 5) == 0
            && strncmp(name, "fullscreen", 10) == 0) {
        pconfig->fullscreen = atoi(value);
    } else if (strncmp(section, "video", 5) ==
            0 && strncmp(name, "width", 5) == 0) {
        pconfig->width = atoi(value);
    } else if (strncmp(section, "video", 5) == 0
            && strncmp(name, "height", 6) == 0) {
        pconfig->height = atoi(value);
    } else {
        printf("Unknown configuration: %s:%s, value: %s\n",
                section, name, value);
        return 0;
    }
    return 1;
}

int configure(configuration* config)
{
    int res = ini_parse(CONFIG_FILE, handler, config);
    if (res < 0) {
        printf("can't load '%s'\n", CONFIG_FILE);
    } else if (res) {
        printf("parse error in %s on line %d.\n", CONFIG_FILE, res);
    }
    return res == 0;
}
