#include "ANC.h"

// Union object for describing the IO
volatile union {
    Uint32 ABC;
    Int16 Channel[2];
} CodecDataIn, CodecDataOut;

interrupt void Codec_ISR()
{
    // Generic public variables for setup and return
    float d_norm;
    float x_norm;
    float e_norm;
    float out_f;

    if(CheckForOverrun())
        return;

    CodecDataIn.ABC = ReadCodecData();

    // Normalization for each channel, brings values to [-1,1]
    d_norm = ((float)CodecDataIn.Channel[LEFT])  / 32768.0f;
    x_norm = ((float)CodecDataIn.Channel[RIGHT]) / 32768.0f;

    // Normalized error
    e_norm = ANC_ProcessSample(d_norm, x_norm);

    /* Store for later UART transmission */
    ANC_BufferTriple(d_norm, x_norm, e_norm);

    /* Scale the output off of gain and clamp properly to [-1,1] */
    out_f = ANC_OUTPUT_GAIN * e_norm * ANC_OUT_LIMIT;
    out_f = ANC_ClampFloat(out_f, ANC_OUT_LIMIT);

    /* output clamped and scaled values to the LineOut */
    CodecDataOut.Channel[LEFT]  = (Int16)out_f;
    CodecDataOut.Channel[RIGHT] = (Int16)out_f;

    WriteCodecData(CodecDataOut.ABC);
}