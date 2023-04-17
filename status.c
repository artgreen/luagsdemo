
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path        "include"
#pragma noroot

segment "status";

#include <stdlib.h>
#include "status.h"

Status *newStatus(long ticks, char *name) {
    Status *status = (Status *)malloc(sizeof(Status));
    status->ticks = ticks;
    status->name = name;
    return status;
}

void freeStatus(Status *status) {
    free(status);
}

