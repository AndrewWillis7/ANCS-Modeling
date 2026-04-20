#include "ANC.h"

ANC_State g_anc;

// class shenanigans
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

    Init_UART2(ANC_UART_BAUD);
    Puts_UART2("ANC START\r\n");
    Puts_UART2("d,e\r\n");
}

// This should fix Randy's Bug due to integer overflow
float ANC_ClampFloat(float x, float limit)
{
    if(x > limit)  return limit;
    if(x < -limit) return -limit;
    return x;
}

// This is the algorithm from class
float ANC_ProcessSample(float d_in, float x_in)
{
    int i;
    float y = 0.0f;
    float e;

    /* Shift reference history */
    for(i = ANC_FILTER_LEN - 1; i > 0; i--)
    {
        g_anc.x_hist[i] = g_anc.x_hist[i - 1];
    }
    g_anc.x_hist[0] = x_in;

    /* FIR output: y(n) = w^T(n)x(n) */
    for(i = 0; i < ANC_FILTER_LEN; i++)
    {
        y += g_anc.w[i] * g_anc.x_hist[i];
    }

    /* Error/output: e(n) = d(n) - y(n) */
    e = d_in - y;

    /* LMS update: w(n+1) = w(n) + beta * e(n) * x(n) */
    for(i = 0; i < ANC_FILTER_LEN; i++)
    {
        g_anc.w[i] += ANC_BETA * e * g_anc.x_hist[i];

        /* coefficient clamp for lab robustness */
        g_anc.w[i] = ANC_ClampFloat(g_anc.w[i], ANC_WEIGHT_LIMIT);
    }

    g_anc.d = d_in;
    g_anc.x = x_in;
    g_anc.y = y;
    g_anc.e = e;
    g_anc.sample_count++;

    return e;
}

// logger instance, should fix blocking issues? If not, delete
void ANC_LogPair(float d, float e)
{
    char msg[64];

    g_anc.uart_count++;
    if(g_anc.uart_count < ANC_UART_DECIM)
        return;

    g_anc.uart_count = 0;

    sprintf(msg, "%.6f,%.6f\r\n", d, e);
    Puts_UART2(msg);
}