
#ifndef LUAGSDEMO_STATUS_H
#define LUAGSDEMO_STATUS_H

#include <stdio.h>
#include "lstate.h"

// Define a struct to represent the status data
typedef struct {
    long ticks;
    char name[20];
} Status;

// Function to create a new status object
Status *newStatus(long ticks, char *name);

// Function to free a status object
void freeStatus(Status *status);

// Load the status Library
void load_status(lua_State *L);

#endif //LUAGSDEMO_STATUS_H

