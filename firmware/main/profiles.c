#include "profiles.h"

#include <stdbool.h>

static const profile_t manual = {
    .name = "Manual",
    .num_pts = 2,
    .pts = {
        { .time = 0.0,   .temp = 25.0, .wait = false },
        { .time = 300.0, .temp = 25.0, .wait = false },
    },
};

static const profile_t sac305 = {
    .name = "SAC305",
    .num_pts = 6,
    .pts = {
        { .time = 0.0,   .temp = 25.0,  .wait = false },
        { .time = 60.0,  .temp = 150.0, .wait = true  },
        { .time = 120.0, .temp = 180.0, .wait = true  },
        { .time = 20.0,  .temp = 255.0, .wait = true  },
        { .time = 15.0,  .temp = 255.0, .wait = false },
        { .time = 60.0,  .temp = 25.0,  .wait = false }
    },
};

static const profile_t sn63pb37 = {
    .name = "Sn63/Pb37",
    .num_pts = 6,
    .pts = {
        { .time = 0.0,   .temp = 25.0,  .wait = false },
        { .time = 60.0,  .temp = 150.0, .wait = true  },
        { .time = 120.0, .temp = 165.0, .wait = true  },
        { .time = 20.0,  .temp = 235.0, .wait = true  },
        { .time = 20.0,  .temp = 235.0, .wait = false },
        { .time = 60.0,  .temp = 25.0,  .wait = false }
    },
};

/* Public constants */
const size_t NUM_PROFILES = 3;

const profile_t* PROFILES[] = {
    &manual, // manual always idx 0
    &sac305,
    &sn63pb37,
};
