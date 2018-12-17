#include <stdio.h>

void HAL_Printf(const char *fmt, ...);

int main(int argc, char *argv[])
{
    HAL_Printf("dynreg example\n");
    return 0;
}