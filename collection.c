#pragma memorymodel 1
#pragma noroot
#include <stdlib.h>
#include "collection.h"
Collection *newCollection(size_t size) {
    Collection *c;
    if (size == 0 || size > (size_t)-1 / sizeof(lua_Integer)) return NULL;
    c = malloc(sizeof(*c));
    if (c == NULL) return NULL;
    c->data = calloc(size, sizeof(lua_Integer));
    if (c->data == NULL) { free(c); return NULL; }
    c->size = size;
    return c;
}
void freeCollection(Collection *c) {
    if (c != NULL) { free(c->data); free(c); }
}
