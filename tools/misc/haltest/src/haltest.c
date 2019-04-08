#include <stdio.h>
#include <unistd.h>
#include "smartconfig_ieee80211.h"

int main(int argc, char *argv[])
{
    printf("hello haltest\n");
    verify_smartconfig_raw_frame();
    while(1) {
        sleep(2);
    }
    return 0;
}