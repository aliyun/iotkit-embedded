#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    /* test smartconfig, you need call following test func 
     * verify_awss_preprocess()
     * verify_awss_close_monitor();
     * verify_awss_open_monitor();
     * verify_awss_postprocess();
     */
    verify_awss_preprocess(6);
    verify_awss_close_monitor();
    verify_awss_open_monitor();
    
    while(1) {
        sleep(2);
    }
    return 0;
}