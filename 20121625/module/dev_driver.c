#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/slab.h>

#define DEV_MAJOR 242
/* FND */
#define FND_ADDRESS 0x08000004
#define DOT_ADDRESS 0x08000210  

static unsigned char *dot_addr;
static char dot_font[10][10] = {
    {0x3e,0x7f,0x63,0x73,0x73,0x6f,0x67,0x63,0x7f,0x3e}, // 0
    {0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}, // 1
	{0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f}, // 2
	{0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e}, // 3
	{0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06}, // 4
	{0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e}, // 5
	{0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e}, // 6
	{0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03}, // 7
	{0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e}, // 8
    {0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03}
};
static unsigned char *fnd_addr;
static unsigned short fnd;

/* TIMER */
static struct timer_list timer;
static void timer_function(unsigned long);


static int inter_major=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;


static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

/* MODE SETTINGS */
static int mode;
static int time;
static int time_set;
static int mv_cnt;

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler3(int irq, void* dev_id, struct pt_regs* reg);

int interruptCount=0;

wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

static struct file_operations inter_fops =
{
	.open = inter_open,
	.write = inter_write,
	.release = inter_release,
};
void dev_display(){
    int i;
    /* counting mode */
    if(mode==0){
        fnd = (mv_cnt/1000)<<12;
        fnd += (mv_cnt/100)<<8;
        fnd += (mv_cnt/10)<<4;
        fnd += (mv_cnt%10);
        outw(fnd,(unsigned int)fnd_addr);
        for(i=0;i<10;i++){
            outw(dot_font[mv_cnt%10][i]&0x7f,(unsigned int)dot_addr+i*2);
        }
 
    }
    /* timer mode */
    else{
        fnd = ((time/10)<<4) + time%10;
        outw(fnd,(unsigned int)fnd_addr);
        if(time<10 && time_set==0){
            /* DOT MATRIX */ 
            for(i=0;i<10;i++){
                outw(dot_font[time][i]&0x7f,(unsigned int)dot_addr+i*2);
            }
        }
    }
}

static void timer_function(unsigned long timeout){
    int i;
    timer.expires = get_jiffies_64() + 100;
    timer.data = (unsigned long)&timer;
    timer.function = timer_function;

    if(time<0){
/*        outw(0,(unsigned int)fnd_addr);
        for(i=0;i<10;i++){
            outw(0&0x7f,(unsigned int)dot_addr+i*2);
        }*/
        return;
    }
    dev_display();
    time--;
    add_timer(&timer);
}

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg) {
    int i;
    /* COUNTER TO TIMER*/
    if(mode==0){
        mode=1;
        time = 0;
        time_set = 1;
        for(i=0;i<10;i++){
            outw(0&0x7f,(unsigned int)dot_addr+i*2);
        }

    }
    /* TIMER TO COUNTER */
    else{
        mode = 0;
        del_timer(&timer);
    }
    printk("mode : %d",mode);
    dev_display();
	return IRQ_HANDLED;
}


irqreturn_t inter_handler2(int irq, void* dev_id,struct pt_regs* reg) {
    if(mode!=0 && time_set!=0){
        if(time<60) time++;
    
        printk("%d",time);
        dev_display();
    }
    return IRQ_HANDLED;
}


irqreturn_t inter_handler3(int irq, void* dev_id, struct pt_regs* reg) {
    /* START COUNTING DOWN */
    if(mode!=0 && time_set!=0){
        time_set=0;
        del_timer(&timer);
        timer.expires = get_jiffies_64() + 90;
        timer.data = (unsigned long)&timer;
        timer.function = timer_function;
        add_timer(&timer);
    }
    return IRQ_HANDLED;
}


static int inter_open(struct inode *minode, struct file *mfile){
	/* INTERRUPT SETTINGS */
    int irq, ret;
    gpio_direction_input(IMX_GPIO_NR(1,11));
	irq=gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, (irq_handler_t)inter_handler1, IRQF_TRIGGER_FALLING, "home", 0);

	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq=gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, (irq_handler_t)inter_handler2, IRQF_TRIGGER_FALLING, "volup", 0);

	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq=gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, (irq_handler_t)inter_handler3, IRQF_TRIGGER_FALLING, "voldown", 0);

    /* DEVICE MAPPING */
    fnd_addr = ioremap(FND_ADDRESS,0x4);
    dot_addr = ioremap(DOT_ADDRESS,0x10);

    /* INIT VARIABLES */
    mode = 0;
    time = 0;
    time_set = 1;
    mv_cnt = 0;
    dev_display();
	return 0;
}

static int inter_release(struct inode *minode, struct file *mfile){
    int i;
    free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	
    outw(0,(unsigned int)fnd_addr);
    for(i=0;i<10;i++){
        outw(0&0x7f,(unsigned int)dot_addr+i*2);
    }
    iounmap(dot_addr);
    iounmap(fnd_addr);
 
	printk(KERN_ALERT "Release Module\n");
	return 0;
}

static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
    if(count==1){
        printk("timer mode");
        dev_display();
        return time;
    }
    else if(!mode){
        mv_cnt++;
        printk("mv_cnt : %d",mv_cnt);
    }
    dev_display();
	return mv_cnt;
}

static int inter_register_cdev(void)
{
    int error;
    inter_major = DEV_MAJOR;
	if(inter_major) {
		inter_dev = MKDEV(inter_major, 0);
		error = register_chrdev_region(inter_dev,1,"dev_driver");
	}
    else{
		error = alloc_chrdev_region(&inter_dev,0,1,"dev_driver");
		inter_major = MAJOR(inter_dev);
	}
	if(error<0) {
		printk(KERN_WARNING "dev_driver: can't get major %d\n", inter_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", inter_major);
	cdev_init(&inter_cdev, &inter_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	error = cdev_add(&inter_cdev, inter_dev, 1);
	if(error)
	{
		printk(KERN_NOTICE "dev_driver Register Error %d\n", error);
	}
	return 0;
}

static int __init inter_init(void) {
	int result;

	if((result = inter_register_cdev()) < 0 )
		return result;
	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/dev_driver, Major Num : 242 \n");
    
    init_timer(&timer);
	return 0;
}

static void __exit inter_exit(void) {
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
	MODULE_LICENSE("GPL");
