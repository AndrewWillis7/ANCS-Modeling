MEMORY
{
    L2RAM (RWX) : origin = 0x11800000 length = 0x00040000
}

SECTIONS
{
    .text   > L2RAM
    .cinit  > L2RAM
    .bss    > L2RAM
    .stack  > L2RAM
    .sysmem > L2RAM
    .const  > L2RAM
}
