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
#define GET_FND_0(x) ((x>>12)&0xF)
#define GET_FND_1(x) ((x>>8)&0xF)
#define GET_FND_2(x) ((x>>4)&0xF)
#define GET_FND_3(x) ((x)&0xF)

static unsigned char *fnd_addr;
static unsigned short fnd;

/* TIMER */
static struct timer_list timer;
static struct timer_list killer;
static void timer_function(unsigned long);
static void killer_function(unsigned long);

static int pause;
static int kill;
static u32 exit_down;

static int inter_major=0, inter_minor=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;
static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);


irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler3(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg);

int interruptCount=0;

wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

static struct file_operations inter_fops =
{
	.open = inter_open,
	.write = inter_write,
	.release = inter_release,
};

static void timer_function(unsigned long timeout){
    if(kill){ 
        outw(0,(unsigned int)fnd_addr);
        return;
    }

    timer.expires = get_jiffies_64() + (100);
    timer.data = (unsigned long)&timer;
    timer.function = timer_function;
    printk("in timer funct\n");
    if(!pause){
        fnd++;
        if(GET_FND_3(fnd)>=10){
            fnd += 1<<4;
            fnd &= 0xFFF0;
        }
        if(GET_FND_2(fnd)>=6){
            fnd += 1<<8;
            fnd &=0xFF00;
        }
        if(GET_FND_1(fnd)>=10){
            fnd += 1<<12;
            fnd &=0xF000;
        }
        if(GET_FND_0(fnd)>=6){
            fnd=0;
        }
    }
    outw(fnd,(unsigned int)fnd_addr);
    add_timer(&timer);
}

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg) {
    if(pause){
        del_timer(&timer);
        timer.expires = get_jiffies_64()+(100);
        timer.function = timer_function;
        add_timer(&timer);
    }
    pause = 0;
    printk("home\n");
	return IRQ_HANDLED;
}

irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg) {
    pause =1;
    del_timer(&timer);
    printk("back\n");
    return IRQ_HANDLED;
}

irqreturn_t inter_handler3(int irq, void* dev_id,struct pt_regs* reg) {
    fnd = 0;
    pause = 1;
    del_timer(&timer);
    printk("reset\n");
    outw(fnd,(unsigned int)fnd_addr);

    return IRQ_HANDLED;
}

static void killer_function(unsigned long timeout){
    kill = 1;
    exit_down = 0;
    del_timer(&timer);
    timer.expires = get_jiffies_64()+10;
    timer.function = timer_function;
    add_timer(&timer);

    __wake_up(&wq_write, 1, 1, NULL);

    
    printk("wake up\n");
}


irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg) {
    printk("exit\n");
    u32 now = get_jiffies_64();
    if(!exit_down){
        exit_down=get_jiffies_64();
        del_timer(&killer);
        killer.expires = get_jiffies_64()+300;
        killer.function = killer_function;
        add_timer(&killer);
    }
    else if(now<killer.expires){
        printk("exit cancle\n");
        exit_down = 0;
        del_timer(&killer);
    }

    return IRQ_HANDLED;
}


static int inter_open(struct inode *minode, struct file *mfile){
	int ret;
	int irq;

	printk(KERN_ALERT "Open Module\n");

	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, (irq_handler_t)inter_handler1, IRQF_TRIGGER_FALLING, "home", 0);

	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, (irq_handler_t)inter_handler2, IRQF_TRIGGER_FALLING, "back", 0);

	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, (irq_handler_t)inter_handler3, IRQF_TRIGGER_FALLING, "volup", 0);

	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, (irq_handler_t)inter_handler4, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "voldown", 0);

	return 0;
}

static int inter_release(struct inode *minode, struct file *mfile){
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	
	printk(KERN_ALERT "Release Module\n");
	return 0;
}

static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
    /* INIT FND */
    fnd_addr = ioremap(FND_ADDRESS,0x4);
    fnd = 0; 
    outw(0,(unsigned int)fnd_addr);
    pause = 1;
    kill = 0;
    exit_down = 0;
    if(interruptCount==0){
        printk("sleep on\n");
        interruptible_sleep_on(&wq_write);
    }
	return 0;
}

static int inter_register_cdev(void)
{
    int error;
    inter_major = DEV_MAJOR;
	if(inter_major) {
		inter_dev = MKDEV(inter_major, inter_minor);
		error = register_chrdev_region(inter_dev,1,"stopwatch");
        printk("got major 242\n");
	}
    else{
		error = alloc_chrdev_region(&inter_dev,inter_minor,1,"stopwatch");
		inter_major = MAJOR(inter_dev);
	}
	if(error<0) {
		printk(KERN_WARNING "stopwatch: can't get major %d\n", inter_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", inter_major);
	cdev_init(&inter_cdev, &inter_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	error = cdev_add(&inter_cdev, inter_dev, 1);
	if(error)
	{
		printk(KERN_NOTICE "stopwatch Register Error %d\n", error);
	}
	return 0;
}

static int __init inter_init(void) {
	int result;

	if((result = inter_register_cdev()) < 0 )
		return result;
	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/stopwatch, Major Num : 242 \n");
    
    init_timer(&timer);
    init_timer(&killer);
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