#include "opentime/errorStatus.h"

namespace opentime {
    
std::string ErrorStatus::outcome_to_string(Outcome o) {
    switch(o) {
    case OK:
        return std::string();
    case INVALID_TIMECODE_RATE:
        return "invalid timecode rate";
    case NON_DROPFRAME_RATE:
        return "rate is not a dropframe rate";
    case INVALID_TIMECODE_STRING:
        return "string is not a valid timecode string";
    case TIMECODE_RATE_MISMATCH:
        return "timecode specifies a frame higher than its rate";
    case INVALID_TIME_STRING:
        return "invalid time string";
    case NEGATIVE_VALUE:
        return "value cannot be negative here";
    default:
        return "unknown/illegal ErrorStatus::Outcomde code";
    };
}

}

    
        
            
        