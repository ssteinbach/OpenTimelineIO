
extern "C"
{
typedef void* RationalTime;

RationalTime* OTIO_RationalTime_new(double value, double rate);
void OTIO_RationalTime_delete(RationalTime* t);
RationalTime* OTIO_RationalTime_add(RationalTime* a, RationalTime* b);
RationalTime* OTIO_RationalTime_rescaled_to(RationalTime* t, double rate);
double OTIO_RationalTime_get_value(RationalTime* t);
double OTIO_RationalTime_get_rate(RationalTime* t);
}
