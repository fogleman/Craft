#include "api.h"
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#define MAX_FILENAME_LENGTH 1024
#define MAX_ERROR_SIZE 255

#define MODULE_DIR "modules/"

// API functions
int api_add_block_type(lua_State *l)
{
    // add the given 'block type' to our engine and return the generated block ID
    const char *block_name = lua_tostring(l, 1);
    int block_id = 37;  // random id. we should have a real dynamic way to add block types in the engine
    lua_pushnumber(l, block_id);
    return 1;
}

luaL_Reg clua_regs[] =
{
    { "add_block_type", api_add_block_type },
    { NULL, NULL }
};


// structs
struct module_name_list {
    char *filename;
    struct module_name_list *next_ptr;
};

struct loaded_module_list {
    char *filename;
    lua_State *L;
    struct loaded_module_list *next_ptr;
};

struct loaded_module_list *loaded_mods;


// lua init/shutdown
int clua_init()
{
    struct module_name_list *mod, *mod_names;
    mod_names = NULL;
    loaded_mods = NULL;

    // get list of modules
#ifdef _WIN32
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;

    char sPath[2048];

    sprintf(sPath, "%s\\*.lua", MODULE_DIR);

    if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE) {
        dprintf(stderr, "Module folder not found: [%s]\n", sDir);
        return 1;
    }

    do {
        if(strcmp(fdFile.cFileName, ".") != 0
           && strcmp(fdFile.cFileName, "..") != 0)
        {
            sprintf(sPath, "%s\\%s", sDir, fdFile.cFileName);

            mod = malloc(sizeof(struct module_name_list));
            mod->filename = sPath;

            SGLIB_LIST_ADD(struct module_name_list, mod_names, mod, next_ptr);
        }
    } while (FindNextFile(hFind, &fdFile));

    FindClose(hFind);
#else
    DIR *d;
    struct dirent *dir;
    d = opendir(MODULE_DIR);

    if (d) {
        char *ext = ".lua";
        int ext_len = strlen(ext);

        while ((dir = readdir(d)) != NULL) {
            int name_len = strlen(dir->d_name);

            if ((name_len > ext_len) && (0 == strncmp(dir->d_name + (name_len - ext_len), ext, ext_len))) {
                char module_filename[MAX_FILENAME_LENGTH] = {0};

                if (MAX_FILENAME_LENGTH <= (strlen(MODULE_DIR) + strlen(dir->d_name))) {
                    fprintf(stderr, "Module name is too large: %s\n", dir->d_name);
                    continue;
                }

                strncpy(module_filename, MODULE_DIR, strlen(MODULE_DIR));
                strncat(module_filename, dir->d_name, MAX_FILENAME_LENGTH - strlen(MODULE_DIR) - 1);

                mod = malloc(sizeof(struct module_name_list));
                mod->filename = module_filename;

                SGLIB_LIST_ADD(struct module_name_list, mod_names, mod, next_ptr);
            }
        }

        closedir(d);
    }
#endif

    // load lua modules
    SGLIB_LIST_MAP_ON_ELEMENTS(struct module_name_list, mod_names, mod_name, next_ptr, {
        printf("Loading module %s\n", mod_name->filename);

        struct loaded_module_list *mod;
        mod = malloc(sizeof(struct loaded_module_list));

        // we have a separate lua state for every single module we load
        mod->L = luaL_newstate();
        luaL_openlibs(mod->L);

        // init our API
        lua_newtable(mod->L);
        luaL_setfuncs(mod->L, clua_regs, 0);
        lua_setglobal(mod->L, "api");

        if (luaL_loadfile(mod->L, mod_name->filename)) {
            fprintf(stderr, "Lua module %s failed to load\n", mod_name->filename);
            free(mod);
        }
        else {
            if (lua_pcall(mod->L, 0, 0, 0)) {
                fprintf(stderr, "Lua module %s failed to run\n", mod_name->filename);
                free(mod);
            }
            else {
                lua_getglobal(mod->L, "startup");

                if (lua_pcall(mod->L, 0, 0, 0)) {
                    fprintf(stderr, "Startup in %s failed\n", mod_name->filename);
                    free(mod);
                }
                else {
                    SGLIB_LIST_ADD(struct loaded_module_list, loaded_mods, mod, next_ptr)
                }
            }
        }
    });

    return 0;
}


int clua_close()
{
    // close lua on every loaded module
    SGLIB_LIST_MAP_ON_ELEMENTS(struct loaded_module_list, loaded_mods, mod, next_ptr, {
        lua_close(mod->L);
        free(mod);
    });

    return 0;
}
