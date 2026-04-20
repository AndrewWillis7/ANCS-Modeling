#ifndef ANC_H_INCLUDED
#define ANC_H_INCLUDED

#include "DSP_Config.h"
#include <stdio.h>
#include <math.h>

/* ===================== USER SETTINGS ===================== */
/* Change these between runs */
#define ANC_FILTER_LEN      10      /* 10 or 50 */
#define ANC_BETA            0.01f   /* 0.001, 0.01, or 0.1 */

/* Audio output scaling */
#define ANC_OUTPUT_GAIN     0.90f

/* UART logging */
#define ANC_UART_BAUD       115200
#define ANC_UART_DECIM      8       /* log every 8th sample */

/* Safety limits */
#define ANC_EPSILON         1.0e-8f
#define ANC_WEIGHT_LIMIT    4.0f
#define ANC_OUT_LIMIT       30000.0f

#define LEFT    0
#define RIGHT   1

// simple state machine
typedef struct
{
    float x_hist[ANC_FILTER_LEN];
    float w[ANC_FILTER_LEN];

    float x;
    float d;
    float y;
    float e;

    unsigned long sample_count;
    unsigned long uart_count;
} ANC_State;

/* Global state */
extern ANC_State g_anc;

/* API */
void ANC_Init(void);
float ANC_ProcessSample(float d_in, float x_in);
void ANC_LogPair(float d, float e);
float ANC_ClampFloat(float x, float limit);

#endif