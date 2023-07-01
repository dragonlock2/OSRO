#ifndef PROFILE_H
#define PROFILE_H

#include <stdbool.h>

#define ROOM_TEMP (25.0) // C

typedef enum {
    PROFILE_TYPE_MANUAL,
    PROFILE_TYPE_SAC305,
    PROFILE_TYPE_SN63PB37,
    PROFILE_TYPE_COUNT
} profile_type_t;

typedef struct {
    double temp;
    bool   done;
} profile_status_t;

void profile_set_temp(profile_type_t type, double temp);
const char *profile_name(profile_type_t type);
profile_status_t profile_status(profile_type_t type, double time);

#endif // PROFILE_H
