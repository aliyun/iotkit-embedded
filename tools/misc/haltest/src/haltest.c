#include <stdio.h>
#include <unistd.h>

int switch_channel()
{
    static int index = 0;
    int channels[] = {1,2,3,4,5,6,7,8,9,10,11,12,13};

    /* printf("sizeof(channels): %d\n",sizeof(channels)/sizeof(int)); */
    if (index == sizeof(channels)/sizeof(int)) {
        index = 1;
    }else{
        index++;
    }

    return index;
}

int main(int argc, char *argv[])
{
    /* test smartconfig, you need call following test func 
     * verify_awss_preprocess()
     * verify_awss_close_monitor();
     * verify_awss_open_monitor();
     * verify_awss_switch_channel();
     */
    verify_awss_preprocess();
    verify_awss_close_monitor();
    verify_awss_open_monitor();

    while(1) {
        /* printf("current channel: %d\n",switch_channel()); */
        verify_awss_switch_channel(switch_channel());
        usleep(250*1000);
    }
    return 0;
}