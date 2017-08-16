/**
 *
 * opentime library
 *
 * A simple, dependencyless library for manipulating time.
 *
 */

#include "opentime.h"

#include <math.h>

// @{ Convert Time bases
struct OT_RationalTime_t
OT_rt_to_time_base_f(
        const struct OT_RationalTime_t source,
        const float target_base
)
{
    struct OT_RationalTime_t result;
    result.value = source.value * (target_base / source.base);
    result.base = target_base;

    return result;
}

struct OT_RationalTime_t
OT_rt_to_time_base_rt(
        const struct OT_RationalTime_t source,
        const struct OT_RationalTime_t target
)
{
    return OT_rt_to_time_base_f(source, target.base);
}
// @}

// @{ Arithmetic
// will preserve the timebase of the first argument
struct OT_RationalTime_t
OT_rt_add_rt(
        const struct OT_RationalTime_t first,
        const struct OT_RationalTime_t second
)
{
    struct OT_RationalTime_t result;

    result.value = first.value + OT_rt_to_time_base_rt(second, first).value;
    result.base  = first.base;

    return result;
}

struct OT_RationalTime_t
OT_rt_subtract_rt(
        const struct OT_RationalTime_t target,
        const struct OT_RationalTime_t other
)
{
    struct OT_RationalTime_t result;

    result.value = target.value - OT_rt_to_time_base_rt(other, target).value;
    result.base = target.base;

    return result;
}

/// @TODO: make it consistent which is being applied and which is being applied to
struct OT_RationalTime_t
OT_tt_apply_transform_rt(
        const struct OT_TimeTransform_t xform,
        const struct OT_RationalTime_t  target
)
{
    struct OT_RationalTime_t result;

    result = OT_rt_add_rt(target, xform.offset);
    result.value *= xform.scale;
    result.base  = target.base;

    return result;
}

struct OT_TimeTransform_t
OT_tt_apply_transform_tt(
        const struct OT_TimeTransform_t xform,
        const struct OT_TimeTransform_t second
)
{
    struct OT_TimeTransform_t result = TT_IDENTITY;

    result.offset = OT_rt_add_rt(xform.offset, second.offset);
    result.scale  = xform.scale * second.scale;

    return result;
}

struct OT_TimeRange_t
OT_tt_apply_transform_tr(
        const struct OT_TimeRange_t range,
        const struct OT_TimeTransform_t xform
)
{
    struct OT_TimeRange_t result;

    result.start = OT_rt_add_rt(range.start, xform.offset);
    result.start.value *= xform.scale;
    result.duration.value *= xform.scale;

    return result;
}
// @}

// @{ Comparison
struct OT_RationalTime_t
OT_rt_max_rt(
        const struct OT_RationalTime_t target,
        const struct OT_RationalTime_t first
)
{
    struct OT_RationalTime_t result;

    // first to "target"'s timebase
    struct OT_RationalTime_t retimed = OT_rt_to_time_base_rt(first, target);

    result.value = fmax(target.value, retimed.value);
    result.base  = retimed.base;

    return result;
}

struct OT_RationalTime_t
OT_rt_min_rt(
        const struct OT_RationalTime_t target,
        const struct OT_RationalTime_t first
)
{
    struct OT_RationalTime_t result = RT_ZERO;

    // first to "target"'s timebase
    struct OT_RationalTime_t retimed = OT_rt_to_time_base_rt(first, target);

    result.value = fmin(target.value, retimed.value);
    result.base  = retimed.base;

    return result;
}

struct OT_RationalTime_t
OT_tr_end_point(const struct OT_TimeRange_t target)
{
    return OT_rt_add_rt(target.start, target.duration);
}

struct OT_TimeRange_t
OT_tr_extend_rt(
        const struct OT_TimeRange_t target,
        const struct OT_RationalTime_t point
)
{
    struct OT_TimeRange_t result = target;

    result.start = OT_rt_min_rt(target.start, point);
    result.duration = OT_rt_subtract_rt(
            OT_rt_max_rt(OT_tr_end_point(target), point), 
            target.start
    );

    return result;
}

struct OT_TimeRange_t
OT_tr_extend_tr(
        const struct OT_TimeRange_t target,
        const struct OT_TimeRange_t extend_by
)
{
    struct OT_TimeRange_t result = target;

    result.start = OT_rt_min_rt(target.start, extend_by.start);
    result.duration = OT_rt_max_rt(target.duration, extend_by.duration);

    return result;
}

struct OT_RationalTime_t
OT_tr_clamp_rt(
        const struct OT_TimeRange_t target,
        const struct OT_RationalTime_t point
)
{
    struct OT_RationalTime_t result = point;

    if (target.start_bound == BOUND_STRATEGY_CLAMP)
    {
        result = OT_rt_max_rt(target.start, point);
    }

    if (target.end_bound == BOUND_STRATEGY_CLAMP)
    {
        result = OT_rt_min_rt(OT_tr_end_point(target), result);
    }

    return result;
}

bool
OT_rt_is_greater_than_rt(
        const struct OT_RationalTime_t first,
        const struct OT_RationalTime_t second
)
{
    struct OT_RationalTime_t second_in_first_tb = OT_rt_to_time_base_rt(second, first);

    return (first.value > second.value);
}

bool
OT_rt_is_less_than_rt(
        const struct OT_RationalTime_t first,
        const struct OT_RationalTime_t second
)
{
    return !OT_rt_is_greater_than_rt(first, second);
}
// @}

// @{ Conversion Functions

/// @TODO: implement this to match this python code?
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
rt_from_frames(float frame, float fps)
{
    struct OT_RationalTime_t result = {frame, fps};

    return result;
}

float
OT_rt_to_frames(
        const struct OT_RationalTime_t input,
        const float fps
)
{
    const struct OT_RationalTime_t retimed = OT_rt_to_time_base_f(input, fps);
    return retimed.value;
}

struct OT_RationalTime_t
OT_rt_from_timecode(
        const char* timecode_str,
        float base
)
{
    /// @TODO implement me
    /* int n = sscanf() */
    struct OT_RationalTime_t result;
    return result;
}

void
OT_rt_to_timecode(const struct OT_RationalTime_t input, char* result)
{
    /* return ""; */
}


// @}
