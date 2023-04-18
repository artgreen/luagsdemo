
#ifndef LUAGSDEMO_COLLECTION_H
#define LUAGSDEMO_COLLECTION_H

#include <stdio.h>
#include "lstate.h"

// Define a struct to represent the collection of data
typedef struct {
    int *data;
    size_t size;
} Collection;

// Function to create a new collection object
Collection *newCollection(size_t size);

// Function to free a collection object
void freeCollection(Collection *collection);

// Function to get the size of a collection
size_t getCollectionSize(Collection *collection);

// Function to set a value in a collection
void setCollectionValue(Collection *collection, size_t index, int value);

// Function to get a value from a collection
int getCollectionValue(Collection *collection, size_t index);

// Load the Lua Library
void load_collection(lua_State *L);

#endif //LUAGSDEMO_COLLECTION_H
