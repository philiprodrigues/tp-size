#include "TPClasses.h"

std::ifstream& operator>>(std::ifstream& in, TPFromPTMP& tp)
{
    in >> std::hex >> tp.detid >> std::dec;
    in >> tp.channel;
    in >> tp.timestamp;
    in >> tp.adc_sum;
    in >> tp.time_over_threshold;
    return in;
}

ClassImp(TP)
