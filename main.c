#include "ANC.h"

void StartUp(void)
{
    ANC_Init();
}

int main(void)
{
    DSP_Init();
    StartUp();

    while(1)
    {
        /* main idles here; ISR drives the application */
    }
}