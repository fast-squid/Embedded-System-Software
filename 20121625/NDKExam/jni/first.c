#include "android/log.h"
#include <stdio.h>
#include <stdlib.h>

#define LOG_TAG "MyTag"
#define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

extern int first(int x,int y);
extern int wwww();
extern int cccc();

static int fd;

int first(int x, int y)
{
	fd = open("/dev/dev_driver",1);
    if(fd<0){
        LOGV("dev_open error %d",fd );
    }
    else LOGV("dev_opened with fd : %d",fd);
}

int wwww(int mode){
	int a = write(fd,NULL,mode);
	LOGV("mv_cnt %d, %d",a,fd);
	return a;
}

int cccc(){
	close(fd);
	LOGV("close");
}


