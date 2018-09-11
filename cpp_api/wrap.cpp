
#include "wrap.h"

#include "opentime.h"

RationalTime* OTIO_RationalTime_new(double value, double rate) {
    return (RationalTime*)new opentime::RationalTime(value, rate);
}

void OTIO_RationalTime_delete(RationalTime* t) {
    delete (opentime::RationalTime*)t;
}

RationalTime* OTIO_RationalTime_add(RationalTime* a, RationalTime* b) {
    opentime::RationalTime* result = new opentime::RationalTime(*(opentime::RationalTime*)a);
    *result += *(opentime::RationalTime*)b;
    return (RationalTime*)result;
}

RationalTime* OTIO_RationalTime_rescaled_to(RationalTime* t, double rate) {
    opentime::RationalTime* result = new opentime::RationalTime(
        ((opentime::RationalTime*)t)->rescaled_to(rate)
    );
    return (RationalTime*)result;
}

double OTIO_RationalTime_get_value(RationalTime* t) {
    return ((opentime::RationalTime*)t)->value;
}

double OTIO_RationalTime_get_rate(RationalTime* t) {
    return ((opentime::RationalTime*)t)->rate;
}
