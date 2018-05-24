#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/ioctl.h>

#define DEV_NAME "/dev/dev_driver"
#define MK_CMD _IOW('Q',1,int)

int main(int argc, char* argv[]){
    if(argc!=4){
        printf("input format : [interval] [iter_num] [option]");
        return -1;
    }
    int interval=atoi(argv[1]);
    int iter_num=atoi(argv[2]);
    char option[5];
    strcpy(option,argv[3]);
    
    char param[4];
    char param_ret[4];
	int sysret = syscall(379, interval, iter_num, option, param);

    memcpy(param_ret,&sysret,4);
    printf("user : %d %d %d %d\n",param_ret[0], param[1], param[2], param[3]);

    int fd;
    fd = open(DEV_NAME,O_WRONLY);
    if(fd<0){
        printf("device open failuer\n");
        return -1;
    }
   // write(fd, param,sizeof(char)*4);
    ioctl(fd,MK_CMD,param);
    close(fd);
    
	return 0;
}
