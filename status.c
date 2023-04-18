
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path        "include"
#pragma noroot

segment "status";

#include <stdlib.h>
#include <string.h>
#include "status.h"

Status *newStatus(long ticks, char *name) {
    Status *status = (Status *)malloc(sizeof(Status));
    printf("newStatus: Created Status object at address %p\n", status);
    status->ticks = ticks;
    strcpy(status->name, name);
    return status;
}

void freeStatus(Status *status) {
    printf("freeStatus: Freed Status object at address %p\n", status);
    free(status);
}

