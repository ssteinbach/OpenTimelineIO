#ifndef LIBOPENTIME_H_11
#define LIBOPENTIME_H_11

#include <stdbool.h>

/// A pont in time.
struct OT_RationalTime_t {
    // the sample number
    float value;

    // time base - samples per second
    float base;
}OT_RationalTime_t;
static const struct OT_RationalTime_t RT_ZERO = {0.0f, 24.0f};

/// Transform for OT_RationalTime_t
struct OT_TimeTransform_t {
    struct OT_RationalTime_t offset;
    float scale;
};
static const struct OT_TimeTransform_t TT_IDENTITY = { {1.0f, 24.0f}, 1.0f};

typedef enum BoundStrategy {
    BOUND_STRATEGY_FREE  = 0,
    BOUND_STRATEGY_CLAMP = 1,
} BoundStrategy;

struct OT_TimeRange_t {
    struct OT_RationalTime_t start;
    struct OT_RationalTime_t duration;

    BoundStrategy start_bound;
    BoundStrategy end_bound;
};

// Premade Zero-Ranges for extending.
static const struct OT_TimeRange_t TR_ZERO_RANGE_NO_BOUNDS = { 
    {0, 24},
    {0, 24},
    BOUND_STRATEGY_FREE,
    BOUND_STRATEGY_FREE 
};
static const struct OT_TimeRange_t TR_ZERO_RANGE_CLAMPED = { 
    {0, 24},
    {0, 24},
    BOUND_STRATEGY_CLAMP,
    BOUND_STRATEGY_CLAMP 
};


// @{ Convert Time bases
struct OT_RationalTime_t
OT_rt_to_time_base_f(
        const struct OT_RationalTime_t source,
        const float target_base
);


struct OT_RationalTime_t
OT_rt_to_time_base_rt(
        const struct OT_RationalTime_t source,
        const struct OT_RationalTime_t target
);
// @}

// @{ Arithmetic
// will preserve the timebase of the first argument
struct OT_RationalTime_t
OT_rt_add_rt(
        const struct OT_RationalTime_t first,
        const struct OT_RationalTime_t second
);

struct OT_RationalTime_t
OT_rt_subtract_rt(
        const struct OT_RationalTime_t target,
        const struct OT_RationalTime_t other
);

/// TODO: make it consistent which is being applied and which is being applied
struct OT_RationalTime_t
OT_tt_apply_transform_rt(
        const struct OT_TimeTransform_t xform,
        const struct OT_RationalTime_t  target
);

struct OT_TimeTransform_t
OT_tt_apply_transform_tt(
        const struct OT_TimeTransform_t xform,
        const struct OT_TimeTransform_t second
);

struct OT_TimeRange_t
OT_tt_apply_transform_tr(
        const struct OT_TimeRange_t range,
        const struct OT_TimeTransform_t xform
);
// @}

// @{ Comparison
struct OT_RationalTime_t
OT_rt_max_rt(
        const struct OT_RationalTime_t target,
        const struct OT_RationalTime_t first
);

struct OT_RationalTime_t
OT_rt_min_rt(
        const struct OT_RationalTime_t target,
        const struct OT_RationalTime_t first
);

struct OT_RationalTime_t
OT_tr_end_point(const struct OT_TimeRange_t target);

struct OT_TimeRange_t
OT_tr_extend_rt(
        const struct OT_TimeRange_t target,
        const struct OT_RationalTime_t point
);

struct OT_TimeRange_t
OT_tr_extend_tr(
        const struct OT_TimeRange_t target,
        const struct OT_TimeRange_t extend_by
);


struct OT_RationalTime_t
OT_tr_clamp_rt(
        const struct OT_TimeRange_t target,
        const struct OT_RationalTime_t point
);

bool
OT_rt_is_greater_than_rt(
        const struct OT_RationalTime_t first,
        const struct OT_RationalTime_t second
);

bool
OT_rt_is_less_than_rt(
        const struct OT_RationalTime_t first,
        const struct OT_RationalTime_t second
);
// @}

// @{ Conversion Functions

/// TODO: implement this to match this python code?
/**
def from_frames(frame, fps):
    """
    Turn a frame number and fps into a time object.

    For any integer fps value, the time_scale will be the fps.
    For any common non-integer fps value (e.g. 29.97, 23.98) the time scale will
    be 600.
    """
    if int(fps) == fps:
        return RationalTime(frame, int(fps))
    elif int(fps*600) == fps*600:
        return RationalTime(frame*600/fps, 600)
    raise ValueError(
        "Non-standard frames per second ({}) not supported.".format(fps)
    )
*/
struct OT_RationalTime_t
rt_from_frames(
        float frame,
        float fps
);

float
OT_rt_to_frames(
        const struct OT_RationalTime_t input,
        const float fps
);

struct OT_RationalTime_t
OT_rt_from_timecode(
        const char* timecode_str,
        float base
);

void
OT_rt_to_timecode(
        const struct OT_RationalTime_t input,
        char* result
);


// @}
#endif
