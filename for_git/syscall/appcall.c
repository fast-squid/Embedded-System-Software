#include <linux/kernel.h>
#include <asm/uaccess.h>

asmlinkage int sys_appcall(int interval, int iter_num, char *option, char* param) {
    char buf[4];
    int i;
    int res;
    copy_from_user(buf,option,sizeof(char)*4);

    for(i=0;i<4;i++){
        if(buf[i]!='0') break;
    }
    
    put_user(interval, &param[0]);
    __put_user(iter_num, &param[1]);
    __put_user(i, &param[2]);
    __put_user(buf[i]-'0', &param[3]);

    res = buf[i]-'0'; res<<=8;
    res += i; res<<=8;
    res += iter_num; res <<=8;
    res += interval;

	return res;
}
