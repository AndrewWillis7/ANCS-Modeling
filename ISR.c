#include "ANC.h"

/* Standard LCDK codec unions */
volatile union {
    Uint32 ABC;
    Int16 Channel[2];
} CodecDataIn, CodecDataOut;

// DOUBLE CHECK WE AREN'T OVERRIDING THE DEFAULTS
interrupt void Codec_ISR()
{
    float d_norm;
    float x_norm;
    float e_norm;
    float out_f;

    if(CheckForOverrun())
        return;

    /* Read stereo input from codec */
    CodecDataIn.ABC = ReadCodecData();

    /* Project mapping:
       LEFT  = d(n) = primary = desired + noise
       RIGHT = x(n) = reference = correlated noise
    */
    d_norm = ((float)CodecDataIn.Channel[LEFT])  / 32768.0f;
    x_norm = ((float)CodecDataIn.Channel[RIGHT]) / 32768.0f;

    /* LMS ANC core */
    e_norm = ANC_ProcessSample(d_norm, x_norm);

    /* Log d(n), e(n) over UART for MATLAB */
    ANC_LogPair(d_norm, e_norm);

    /* Output e(n) to both channels for monitoring */
    out_f = ANC_OUTPUT_GAIN * e_norm * ANC_OUT_LIMIT;
    out_f = ANC_ClampFloat(out_f, ANC_OUT_LIMIT);

    CodecDataOut.Channel[LEFT]  = (Int16)out_f;
    CodecDataOut.Channel[RIGHT] = (Int16)out_f;

    WriteCodecData(CodecDataOut.ABC);
}