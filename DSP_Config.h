#ifndef DSP_CONFIG_H_INCLUDED
#define DSP_CONFIG_H_INCLUDED

#include "LCDK_Support_DSP.h"

#define DSPBOARDTYPE_TI_OMAPL138_LCDK

/* Pick one sample rate */
#define SampleRateSetting AIC3106Fs8kHz

/* Use line input for WAV playback from PC */
#define CodecType LCDK_LineInput

#endif