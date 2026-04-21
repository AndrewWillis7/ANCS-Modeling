#include "ANC.h"

void _StartUp_(void)
{
    ANC_Init();
}

int main(void)
{
    DSP_Init();
    _StartUp_();

    while(1)
    {
        /* main idles here; ISR drives the application, stream to MATLAB */
        ANC_StreamBufferedData();
    }
}