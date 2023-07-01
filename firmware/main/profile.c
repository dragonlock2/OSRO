#include <string.h>
#include "profile.h"

/* private data */
#define MAX_POINTS (10)

typedef struct {
    double time; // sec
    double temp; // C
} profile_point_t;

typedef struct {
    char           *name;
    size_t          num_pts;
    profile_point_t pts[MAX_POINTS];
} profile_config_t;

static const profile_config_t configs[PROFILE_TYPE_COUNT] = {
    [PROFILE_TYPE_MANUAL] = {
        .name = "Manual",
    },
    [PROFILE_TYPE_SAC305] = { // https://aimsolder.com/sites/default/files/ws483_sac305_solder_paste_tds.pdf
        .name    = "SAC305",
        .num_pts = 5,
        .pts = {
            { .time = 0.0,   .temp = ROOM_TEMP },
            { .time = 90.0,  .temp = 150.0     },
            { .time = 165.0, .temp = 175.0     },
            { .time = 225.0, .temp = 245.0     },
            { .time = 270.0, .temp = ROOM_TEMP },
        }
    },
    [PROFILE_TYPE_SN63PB37] = { // https://www.kester.com/Portals/0/Documents/Knowledge%20Base/Standard_Profile.pdf
        .name    = "Sn63/Pb37",
        .num_pts = 5,
        .pts = {
            { .time = 0.0,   .temp = ROOM_TEMP },
            { .time = 90.0,  .temp = 150.0     },
            { .time = 180.0, .temp = 180.0     },
            { .time = 225.0, .temp = 230.0     },
            { .time = 270.0, .temp = ROOM_TEMP },
        }
    },
};

static struct {
    double manual_temp;
} profile_data;

/* private helpers */

/* public functions */
void profile_set_temp(profile_type_t type, double temp) {
    if (type == PROFILE_TYPE_MANUAL) {
        profile_data.manual_temp = temp;
    }
}

const char *profile_name(profile_type_t type) {
    const char *ret = NULL;
    if (type < PROFILE_TYPE_COUNT) {
        ret = configs[type].name;
    }
    return ret;
}

profile_status_t profile_status(profile_type_t type, double time) {
    profile_status_t ret = {
        .temp = ROOM_TEMP,
        .done = true,
    };
    if (type == PROFILE_TYPE_MANUAL) {
        ret.temp = profile_data.manual_temp;
        ret.done = false;
    } else if (type < PROFILE_TYPE_COUNT) {
        ret.temp = configs[type].pts[configs[type].num_pts - 1].temp;
        for (size_t i = 1; i < configs[type].num_pts; i++) {
            if (time < configs[type].pts[i].time) {
                double m = (configs[type].pts[i].temp - configs[type].pts[i - 1].temp) / 
                    (configs[type].pts[i].time - configs[type].pts[i - 1].time);
                ret.temp = configs[type].pts[i - 1].temp + m * (time - configs[type].pts[i - 1].time);
                ret.done = false;
                break;
            }
        }
    }
    return ret;
}
