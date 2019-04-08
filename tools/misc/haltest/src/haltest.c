#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    verify_awss_rx_frame();
    while(1) {
        sleep(2);
    }
    return 0;
}