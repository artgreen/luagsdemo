
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path        "include"
#pragma noroot

segment "collection";

#include <stdlib.h>
#include "collection.h"

Collection *newCollection(size_t size) {
    Collection *collection = (Collection *)malloc(sizeof(Collection));
    collection->data = (int *)calloc(size, sizeof(int));
    collection->size = size;
    return collection;
}

void freeCollection(Collection *collection) {
    free(collection->data);
    free(collection);
}

size_t getCollectionSize(Collection *collection) {
    return collection->size;
}

void setCollectionValue(Collection *collection, size_t index, int value) {
    collection->data[index] = value;
}

int getCollectionValue(Collection *collection, size_t index) {
    return collection->data[index];
}
