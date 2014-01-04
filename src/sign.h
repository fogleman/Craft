#ifndef _sign_h_
#define _sign_h_

#define MAX_SIGN_LENGTH 64

typedef struct {
    int x;
    int y;
    int z;
    int face;
    char text[MAX_SIGN_LENGTH];
} Sign;

typedef struct {
    unsigned int capacity;
    unsigned int size;
    Sign *data;
} SignList;

void sign_list_alloc(SignList *list, int capacity);
void sign_list_free(SignList *list);
void sign_list_grow(SignList *list);
void sign_list_add(
    SignList *list, int x, int y, int z, int face, const char *text);
int sign_list_remove(SignList *list, int x, int y, int z, int face);
int sign_list_remove_all(SignList *list, int x, int y, int z);

#endif
