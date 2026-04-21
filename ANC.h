#ifndef ANC_H_INCLUDED
#define ANC_H_INCLUDED

#include "DSP_Config.h"
#include <stdio.h>
#include <math.h>

#define ANC_FILTER_LEN      50
#define ANC_BETA            0.01f

#define ANC_OUTPUT_GAIN     0.90f

#define ANC_UART_BAUD       115200
#define ANC_UART_DECIM      32

#define ANC_EPSILON         1.0e-8f
#define ANC_WEIGHT_LIMIT    4.0f
#define ANC_OUT_LIMIT       30000.0f

#define LEFT    0
#define RIGHT   1

/* -------- UART sample buffering -------- */

// Buffer scaled for the triple output, e[n], d[n], x[n]
#define ANC_LOG_BUFFER_SIZE 2048

// log sample for convinience
typedef struct
{
    float d;
    float x;
    float e;
} ANC_LogSample;

// state machine for non-blocking logger
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

extern ANC_State g_anc;

/* Buffer globals */
extern volatile unsigned int g_logWriteIdx;
extern volatile unsigned int g_logReadIdx;
extern volatile unsigned int g_logOverflow;
extern ANC_LogSample g_logBuffer[ANC_LOG_BUFFER_SIZE];

// Starts up the ANC controller 
void ANC_Init(void);

// Large function that processes the incoming noise into posable FFT like systems
float ANC_ProcessSample(float d_in, float x_in);

// Clamping function to retain magnitudes.
float ANC_ClampFloat(float x, float limit);

/* New buffered logging API to fix the slowness issue */
void ANC_BufferTriple(float d, float x, float e);

// data streaming over UART for Matlab interpretation
void ANC_StreamBufferedData(void);

#endif