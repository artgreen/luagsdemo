
#ifndef LUAGSDEMO_STATUS_H
#define LUAGSDEMO_STATUS_H

#include <stdio.h>

// Define a struct to represent the status data
typedef struct {
    long ticks;
    char *name;
} Status;

// Function to create a new status object
Status *newStatus(long ticks, char *name);

// Function to free a status object
void freeStatus(Status *status);

#endif //LUAGSDEMO_STATUS_H

