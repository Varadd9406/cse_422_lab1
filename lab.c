#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/kthread.h>


/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Varad and Peter");
MODULE_DESCRIPTION("Lab 1");

/* hr timer */
static unsigned long log_sec;
static unsigned long log_nsec; 

module_param(log_sec, ulong, 0444);
module_param(log_nsec, ulong, 0444);

static struct hrtimer my_hrtimer;
static ktime_t interval;

static enum hrtimer_restart timer_callback(struct hrtimer *timer) {
    hrtimer_forward_now(&my_hrtimer,interval);
    printk("Hey there again");
	return HRTIMER_RESTART;
}

static int __init ModuleInit(void) {
	printk("hrtime Module Loaded!\n");

    interval = ktime_set(log_sec, log_nsec);

    hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_hrtimer.function = &timer_callback;
    hrtimer_start(&my_hrtimer, interval, HRTIMER_MODE_REL);
	return 0;
}

static void __exit ModuleExit(void) {
	hrtimer_cancel(&my_hrtimer);
	printk("Goodbye, Kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);