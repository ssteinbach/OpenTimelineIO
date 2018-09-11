#include <stdio.h>
#include "wrap.h"

int main(int argc, char**argv){
    printf("Hello\n");

    RationalTime* t1 = OTIO_RationalTime_new(1,24);
    RationalTime* t2 = OTIO_RationalTime_new(10,24);
    RationalTime* t3 = OTIO_RationalTime_add(t1, t2);
    RationalTime* t4 = OTIO_RationalTime_rescaled_to(t3, 30);

    printf("t3 = %g @ %g\n", OTIO_RationalTime_get_value(t3), OTIO_RationalTime_get_rate(t3));
    printf("t4 = %g @ %g\n", OTIO_RationalTime_get_value(t4), OTIO_RationalTime_get_rate(t4));

    OTIO_RationalTime_delete(t1);
    OTIO_RationalTime_delete(t2);
    OTIO_RationalTime_delete(t3);
    OTIO_RationalTime_delete(t4);

    return 0;
}
