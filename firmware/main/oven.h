#ifndef OVEN_H
#define OVEN_H

#include <stdbool.h>
#include "profile.h"

typedef struct {
    double current;
    double target;
    bool   running;
} oven_status_t;

void oven_init(void);
void oven_start(profile_type_t profile, double temp);
void oven_stop(void);
void oven_status(oven_status_t *status);

#endif // OVEN_H
