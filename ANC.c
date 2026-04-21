#include "ANC.h"

ANC_State g_anc;

/* UART log ring buffer */
ANC_LogSample g_logBuffer[ANC_LOG_BUFFER_SIZE];
volatile unsigned int g_logWriteIdx = 0;
volatile unsigned int g_logReadIdx  = 0;
volatile unsigned int g_logOverflow = 0;

/* =========================================================
   ANC_Init
   ---------------------------------------------------------
   Initializes:
   - LMS state variables
   - delay line history
   - filter weights
   - UART logging buffer indices
   - UART itself for sending data to MATLAB
   ========================================================= */
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
    Puts_UART2("d,x,e\r\n");
}

/* =========================================================
   ANC_ClampFloat
   ---------------------------------------------------------
   Limits a float value to the range [-limit, +limit].

   Used to keep LMS weights from growing too large and
   blowing up the filter.
   ========================================================= */
float ANC_ClampFloat(float x, float limit)
{
    if(x > limit)  return limit;
    if(x < -limit) return -limit;
    return x;
}

/* =========================================================
   ANC_ProcessSample
   ---------------------------------------------------------
   Processes one sample of ANC using plain LMS.

   Inputs:
   - d_in = primary input = desired signal + noise
   - x_in = reference input = correlated noise

   Steps:
   1. Shift the reference delay line
   2. Compute filter output y(n)
   3. Compute error/output e(n) = d(n) - y(n)
   4. Update filter coefficients using LMS
   5. Store latest values for debugging/monitoring

   Returns:
   - e(n), the noise-reduced output sample
   ========================================================= */
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

/* =========================================================
   ANC_BufferTriple
   ---------------------------------------------------------
   Stores one (d,x,e) sample set into the ring buffer.

   This is designed to be called from the ISR, so it avoids:
   - sprintf
   - UART transmission
   - any slow/blocking work

   UART decimation is used so not every sample is logged.
   Example: if ANC_UART_DECIM = 16, only every 16th sample
   is stored for MATLAB transmission.
   ========================================================= */

/* ISR-safe-ish buffer write: no sprintf, no UART, adapted to x(n) transmission */
void ANC_BufferTriple(float d, float x, float e)
{
    unsigned int nextIdx;

    g_anc.uart_count++;
    if(g_anc.uart_count < ANC_UART_DECIM)
        return;

    g_anc.uart_count = 0;

    nextIdx = (g_logWriteIdx + 1) % ANC_LOG_BUFFER_SIZE;

    if(nextIdx == g_logReadIdx)
    {
        g_logOverflow++;
        return;
    }

    g_logBuffer[g_logWriteIdx].d = d;
    g_logBuffer[g_logWriteIdx].x = x;
    g_logBuffer[g_logWriteIdx].e = e;
    g_logWriteIdx = nextIdx;
}

/* =========================================================
   ANC_StreamBufferedData
   ---------------------------------------------------------
   Sends buffered (d,x,e) samples over UART to MATLAB.

   IMPORTANT:
   - This should be called from the main loop, NOT the ISR.
   - It sends one character at a time to avoid blocking.

   How it works:
   1. If a CSV line is already being transmitted, keep sending
      the next character whenever UART is ready.
   2. If no line is in progress, pull the next sample from the
      ring buffer, format it as CSV text, and begin sending it.

   Example transmitted line:
   0.123456,0.111111,0.098765
   ========================================================= */

/* Call this in main loop, not ISR */
void ANC_StreamBufferedData(void)
{
    static char tx_msg[96];
    static int tx_len = 0;
    static int tx_pos = 0;

    if(tx_pos < tx_len)
    {
        if(IsTxReady_UART2())
        {
            Write_UART2((Uint8)tx_msg[tx_pos]);
            tx_pos++;
        }
        return;
    }

    if(g_logReadIdx == g_logWriteIdx)
        return;

    tx_len = sprintf(tx_msg, "%.6f,%.6f,%.6f\r\n",
                     g_logBuffer[g_logReadIdx].d,
                     g_logBuffer[g_logReadIdx].x,
                     g_logBuffer[g_logReadIdx].e);
    tx_pos = 0;

    g_logReadIdx = (g_logReadIdx + 1) % ANC_LOG_BUFFER_SIZE;
}