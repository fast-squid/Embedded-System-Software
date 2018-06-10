#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
int main(void){
	int fd;
	int retn;

	fd = open("/dev/stopwatch", O_RDWR);
	if(fd < 0) {
		perror("/dev/stopwatch error");
		exit(-1);
	}
    else { printf("< inter Device has been detected > \n"); }
	
	retn = write(fd, NULL, 0);
	close(fd);

	return 0;
}
