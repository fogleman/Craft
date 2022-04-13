#include <stdlib.h>
#include <string.h>
#include "sign.h"

// Allocate room within the given list structure for "capacity" signs
// Allocates memory.
// Arguments:
// - list: the already initialized sign list to allocate data for
// - capacity: the number of signs to allocate room for
// Returns:
// - modifies the structure pointed to by list
void sign_list_alloc(SignList *list, int capacity) {
    list->capacity = capacity;
    list->size = 0;
    list->data = (Sign *)calloc(capacity, sizeof(Sign));
}

// Free the SignList's data.
// Arguments:
// - list: list pointer whose data will be free'd
// Returns: none
void sign_list_free(SignList *list) {
    free(list->data);
}

// Grow the SignList's data and capacity so there is more room for signs.
// Allocates memory.
// Arguments:
// - list: sign list to grow
// Returns:
// - modifies the structure pointed to by list
void sign_list_grow(SignList *list) {
    SignList new_list;
    sign_list_alloc(&new_list, list->capacity * 2);
    memcpy(new_list.data, list->data, list->size * sizeof(Sign));
    free(list->data);
    list->capacity = new_list.capacity;
    list->data = new_list.data;
}

// Meant to be called by sign_list_add().
// Actually adds a sign and doesn't check for signs on the same block and face
// which should be removed.
// Note: may grow the sign list, allocating memory.
// Arguments:
// - list: SignList to modify and add to
// - sign: sign data to read
// Returns:
// - modifies the structure pointed to by list
void _sign_list_add(SignList *list, Sign *sign) {
    if (list->size == list->capacity) {
        sign_list_grow(list);
    }
    Sign *e = list->data + list->size++;
    memcpy(e, sign, sizeof(Sign));
}

// Add or overwrite a sign into the sign list at block position and face.
// Does not create multiple signs in one location (at the same block & face).
// Note: may allocate memory when growing the sign list.
// Arguments:
// - list: sign list to add the sign to
// - x: block x position to remove signs from
// - y: block y position to remove signs from
// - z: block z position to remove signs from
// - face: the face of the block to remove signs from
// - text: the signs text (a maximum length of MAX_SIGN_LENGTH chars will be used)
// Returns:
// - modifies the structure pointed to by list
void sign_list_add(
    SignList *list, int x, int y, int z, int face, const char *text)
{
    sign_list_remove(list, x, y, z, face);
    Sign sign;
    sign.x = x;
    sign.y = y;
    sign.z = z;
    sign.face = face;
    strncpy(sign.text, text, MAX_SIGN_LENGTH);
    sign.text[MAX_SIGN_LENGTH - 1] = '\0';
    _sign_list_add(list, &sign);
}

// Remove all signs from the sign list at the given block position and face.
// Arguments:
// - list: sign list to search and remove signs from
// - x: block x position to remove signs from
// - y: block y position to remove signs from
// - z: block z position to remove signs from
// - face: the face of the block to remove signs from
// Returns:
// - returns the number of signs removed
// - modifies the structure pointed to by list
int sign_list_remove(SignList *list, int x, int y, int z, int face) {
    int result = 0;
    for (int i = 0; i < list->size; i++) {
        Sign *e = list->data + i;
        if (e->x == x && e->y == y && e->z == z && e->face == face) {
            Sign *other = list->data + (--list->size);
            memcpy(e, other, sizeof(Sign));
            i--;
            result++;
        }
    }
    return result;
}

// Remove all signs from the sign list at the given block position.
// Arguments:
// - list: sign list to search and remove signs from
// - x: block x position to remove signs from
// - y: block y position to remove signs from
// - z: block z position to remove signs from
// Returns:
// - returns the number of signs removed
// - modifies the structure pointed to by list
int sign_list_remove_all(SignList *list, int x, int y, int z) {
    int result = 0;
    for (int i = 0; i < list->size; i++) {
        Sign *e = list->data + i;
        if (e->x == x && e->y == y && e->z == z) {
            Sign *other = list->data + (--list->size);
            memcpy(e, other, sizeof(Sign));
            i--;
            result++;
        }
    }
    return result;
}

