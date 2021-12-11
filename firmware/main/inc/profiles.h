#ifndef PROFILES_H
#define PROFILES_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    double time; // time to spend reaching temp, linearly ramped
    double temp; // °C
    bool   wait; // wait if doesn't reach temp when time up
} point_t;

typedef struct {
    const char* name;
    const size_t num_pts;
    const point_t pts[];
} profile_t;

extern const size_t NUM_PROFILES;
extern const profile_t* PROFILES[];

#endif // PROFILES_H
