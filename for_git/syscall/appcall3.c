#include <linux/kernel.h>
#include <asm/uaccess.h>

asmlinkage int sys_appcall(int interval, int iter_num, char *option, char* param) {
    char buf[4];
    int i;
    unsigned long err;
    err=copy_from_user(buf,option,sizeof(char)*4);

    for(i=0;i<4;i++){
        if(buf[i]!='0') break;
    }
    char a,b,c,d;
    a=interval;
    b=iter_num;
    c=i;
    d=buf[i]-'0';
    printk("a : %d\n",a);
    printk("b : %d\n",b);
    printk("c : %d\n",c);
    printk("d : %d\n",d);
    
    err=put_user(a, &param[0]);
    err=put_user(b, &param[1]);
    err=put_user(c, &param[2]);
    err=put_user(d, &param[3]);

	return 1;
}
