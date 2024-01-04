#include "error_codes.h"

namespace core
{

const char* errorMsg(ErrorCode code)
{
    switch (code) {
    case WORKER_BUSY:
        return "Worker busy";
    case GAIN_NOT_FOUND:
        return "Gain not found";
    case ALREADY_RECORDING:
        return "Already recording";
    case ALREADY_NOT_RECORDING:
        return "Already not recording";
    case INVALID_INPUT_DEVICE:
        return "Invalid input device";
    case INVALID_FILTER:
        return "Invalid filter";
    case INVALID_FILTER_OFFSET:
        return "Invalid filter offset";
    case INVALID_CW_OFFSET:
        return "Invalid cw offset";
    case INVALID_DEMOD:
        return "Invalid demod";
    case VFO_NOT_FOUND:
        return "Vfo not found";
    case DEMOD_IS_OFF:
        return "Demod is off";
    case NOT_RUNNING:
        return "Not running";
    case COULDNT_CREATE_FILE:
        return "Couldn't create file";
    case SNIFFER_ALREADY_ACTIVE:
        return "Sniffer already active";
    case SNIFFER_ALREADY_INACTIVE:
        return "Sniffer already inactive";
    case INSUFFICIENT_BUFFER_SIZE:
        return "Insufficient buffer size";
    case RDS_ALREADY_ACTIVE:
        return "Rds already active";
    case RDS_ALREADY_INACTIVE:
        return "Rds already inactive";
    case UNKNOWN_ERROR:
    default:
        return "Unknown error";
    }
}

} // namespace core
