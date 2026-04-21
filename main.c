#include "ANC.h"

void _StartUp_(void)
{
    ANC_Init();
}

int main(void)
{
    /* Generic startup message for the DSP board*/
    DSP_Init();

    /* Custon Startup for ANC Control System */
    _StartUp_();

    while(1)
    {
        /* main idles here; ISR drives the application, stream to MATLAB */
        ANC_StreamBufferedData();
    }
}