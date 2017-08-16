#include "opentime.h"

#include <stdio.h>
#include <assert.h>

/** Unit tests for libopentime
 * 
 * \file test_opentime.c
 */

void
test_add()
{
    struct OT_RationalTime_t test = {10, 24};
    struct OT_RationalTime_t one  = {1, 24};
    struct OT_RationalTime_t test_result;

    test_result = OT_rt_add_rt(test, one);
    assert(test_result.value == 11.0f);
}

void
test_subtract()
{
    struct OT_RationalTime_t test = {10, 24};
    struct OT_RationalTime_t one  = {1, 12};
    struct OT_RationalTime_t test_result;

    test_result = OT_rt_subtract_rt(test, one);
    assert(test_result.value == 8.0f);
}

void
test_totimebase()
{
    struct OT_RationalTime_t test = {10, 12};
    struct OT_RationalTime_t test_24 = {10, 24};
    struct OT_RationalTime_t test_result;

    test_result = OT_rt_to_time_base_f(test, 24.0f);
    assert(test_result.value == 20);

    test_result = OT_rt_to_time_base_rt(test, test_24);
    assert(test_result.value == 20);
}

void
test_rationaltime()
{
    test_totimebase();
    test_add();
    test_subtract();
}

void
test_apply()
{
    struct OT_RationalTime_t test = {10, 24};
    struct OT_RationalTime_t test_result;
    struct OT_TimeTransform_t xform = { {2, 12}, 2.0f };

    test_result = OT_tt_apply_transform_rt(xform, test);
    assert(test_result.value == 28.0f);
}

void
test_timetransform()
{
    test_apply();
}

void
test_clamp()
{
    struct OT_TimeRange_t test = { 
        {0, 24},
        {10, 24},
        BOUND_STRATEGY_CLAMP,
        BOUND_STRATEGY_FREE
    };

    struct OT_RationalTime_t below = {-1, 24};
    struct OT_RationalTime_t above = {11, 24};
    
    struct OT_RationalTime_t result = OT_tr_clamp_rt(test, below);
    assert(result.value == test.start.value);

    result = OT_tr_clamp_rt(test, above);
    assert(result.value == above.value);
}

void
test_extend()
{
    struct OT_TimeRange_t test = { 
        {0, 24},
        {10, 24},
        BOUND_STRATEGY_CLAMP,
        BOUND_STRATEGY_FREE
    };

    struct OT_RationalTime_t below = {-1, 24};
    struct OT_RationalTime_t above = {11, 24};

    struct OT_TimeRange_t result = test;

    result = OT_tr_extend_rt(test, below);
    result = OT_tr_extend_rt(result, above);
    assert(result.start.value == below.value);
    assert(OT_tr_end_point(result).value == above.value);

    result = OT_tr_extend_tr(test, result);
    assert(result.start.value == below.value);
    assert(OT_tr_end_point(result).value == above.value);
}

void
test_timerange()
{
    test_clamp();
    test_extend();
}

int
main()
{
    test_rationaltime();
    test_timetransform();
    test_timerange();

    return 0;
}
