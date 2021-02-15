#pragma once

#include <fstream>
#include <cstdint>

#include "TROOT.h"

// TP with the fields saved to disk from ProtoDUNE ptmp TPs
struct TPFromPTMP
{
    uint32_t detid;
    uint32_t channel;
    uint64_t timestamp;
    uint32_t adc_sum;
    uint32_t time_over_threshold;

};

std::ifstream& operator>>(std::ifstream& in, TPFromPTMP& tp);

// Copied from this talk, given on 2021-02-02:
// https://indico.fnal.gov/event/47053/contributions/207915/attachments/139684/175399/DS_data_products.pdf
//
struct TP
{
    // Default ctor needed by ROOT
    TP() {}
  
    TP(TPFromPTMP& ptmp)
        : time_start(ptmp.timestamp)
        , time_peak(ptmp.time_over_threshold/2) // Made up, semi-constant
        , time_over_threshold(ptmp.time_over_threshold)
        , channel(ptmp.channel)
        , adc_integral(ptmp.adc_sum)
        , adc_peak(ptmp.adc_sum/10) // Made up, semi-constant
        , detid(ptmp.detid)
        , type(1)           // Made up, constant
        , algorithm(1)      // Made up, constant
        , version(1)        // Made up, constant
        , flag(0x0)         // Made up, constant
    {}
  
    uint64_t time_start;
    uint64_t time_peak;
    uint32_t time_over_threshold;
    uint32_t channel;
    uint32_t adc_integral;
    uint16_t adc_peak;
    uint32_t detid;
    uint32_t type;
    uint16_t algorithm;
    uint16_t version;
    uint32_t flag;

    ClassDef(TP, 1)
};

// Local Variables:
// c-basic-offset: 4
// End:

