/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iot_export.h"
#include "ota_service.h"

static void usage(void){
	printf("usage:[OPTIONS] \n\n"
		"  $1, Product Key.\n"
		"  $2, Device Name.\n"
                "  $3, Device Secret.\n"
		"\n");
}


int main(int argc, char *argv[]){
    if(argc <= 2) {
        usage();
        return 1;
    }
    ota_service_manager ctx = {0};
    strcpy(ctx.pk,argv[1]);
    strcpy(ctx.dn,argv[2]);
    strcpy(ctx.ds,argv[3]);
    ctx.trans_protcol = 0;
    ctx.dl_protcol = 3;

    printf("Hello OTA.\n");
    IOT_OpenLog("uOTA");
    IOT_SetLogLevel(LOG_DEBUG_LEVEL);

    ota_service_init(&ctx);
    while(1){
    };

    IOT_CloseLog();
    return 0;  
}
