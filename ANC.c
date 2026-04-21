#include "ANC.h"

ANC_State g_anc;

/* UART log ring buffer */
ANC_LogSample g_logBuffer[ANC_LOG_BUFFER_SIZE];
volatile unsigned int g_logWriteIdx = 0;
volatile unsigned int g_logReadIdx  = 0;
volatile unsigned int g_logOverflow = 0;

void ANC_Init(void)
{
    int i;

    for(i = 0; i < ANC_FILTER_LEN; i++)
    {
        g_anc.x_hist[i] = 0.0f;
        g_anc.w[i] = 0.0f;
    }

    g_anc.x = 0.0f;
    g_anc.d = 0.0f;
    g_anc.y = 0.0f;
    g_anc.e = 0.0f;
    g_anc.sample_count = 0;
    g_anc.uart_count = 0;

    g_logWriteIdx = 0;
    g_logReadIdx = 0;
    g_logOverflow = 0;

    Init_UART2(ANC_UART_BAUD);
    Puts_UART2("ANC START\r\n");
    Puts_UART2("d,e\r\n");
}

float ANC_ClampFloat(float x, float limit)
{
    if(x > limit)  return limit;
    if(x < -limit) return -limit;
    return x;
}

float ANC_ProcessSample(float d_in, float x_in)
{
    int i;
    float y = 0.0f;
    float e;

    for(i = ANC_FILTER_LEN - 1; i > 0; i--)
    {
        g_anc.x_hist[i] = g_anc.x_hist[i - 1];
    }
    g_anc.x_hist[0] = x_in;

    for(i = 0; i < ANC_FILTER_LEN; i++)
    {
        y += g_anc.w[i] * g_anc.x_hist[i];
    }

    e = d_in - y;

    for(i = 0; i < ANC_FILTER_LEN; i++)
    {
        g_anc.w[i] += ANC_BETA * e * g_anc.x_hist[i];
        g_anc.w[i] = ANC_ClampFloat(g_anc.w[i], ANC_WEIGHT_LIMIT);
    }

    g_anc.d = d_in;
    g_anc.x = x_in;
    g_anc.y = y;
    g_anc.e = e;
    g_anc.sample_count++;

    return e;
}

/* ISR-safe-ish buffer write: no sprintf, no UART */
void ANC_BufferPair(float d, float e)
{
    unsigned int nextIdx;

    g_anc.uart_count++;
    if(g_anc.uart_count < ANC_UART_DECIM)
        return;

    g_anc.uart_count = 0;

    nextIdx = (g_logWriteIdx + 1) % ANC_LOG_BUFFER_SIZE;

    /* Buffer full: drop sample and count overflow */
    if(nextIdx == g_logReadIdx)
    {
        g_logOverflow++;
        return;
    }

    g_logBuffer[g_logWriteIdx].d = d;
    g_logBuffer[g_logWriteIdx].e = e;
    g_logWriteIdx = nextIdx;
}

/* Call this in main loop, not ISR */
void ANC_StreamBufferedData(void)
{
    char msg[64];

    while(g_logReadIdx != g_logWriteIdx)
    {
        /* Optional: check UART ready if your support code has it */
        if(!IsTxReady_UART2())
            break;

        sprintf(msg, "%.6f,%.6f\r\n",
                g_logBuffer[g_logReadIdx].d,
                g_logBuffer[g_logReadIdx].e);

        Puts_UART2(msg);

        g_logReadIdx = (g_logReadIdx + 1) % ANC_LOG_BUFFER_SIZE;
    }
}