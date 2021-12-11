#ifndef OVEN_H
#define OVEN_H

#include <stdbool.h>

typedef struct {
    double current;
    double target;
    bool   running;
} temps_t;

void oven_setup();
bool oven_start(int idx, double temp);
void oven_stop();
temps_t oven_get_temps();

#endif // OVEN_H
