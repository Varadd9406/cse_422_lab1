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
static unsigned long log_sec = 1;
static unsigned long log_nsec = 0; 

module_param(log_sec, ulong, 0);
module_param(log_nsec, ulong, 0);

static struct hrtimer my_hrtimer;
static ktime_t interval;
static struct task_struct* threads[4];
static int iteration = 0;

static enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
    int i = 0;
    hrtimer_forward_now(&my_hrtimer,interval);
    printk("Log - timer callback");

    while(i<4)
    {
        wake_up_process(threads[i]);
        i+=1;
    }
	return HRTIMER_RESTART;
}


static int thread_function(void *data) {
     while (!kthread_should_stop())
     {
        iteration++;

        printk(KERN_INFO "Iteration %d: Thread Woken Up, Voluntary CS: %lu, Involuntary CS: %lu\n CPU - %d",
               iteration, current->nvcsw, current->nivcsw, smp_processor_id());

        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
    }

    return 0;
}

static int __init ModuleInit(void) {
    int i = 0;

	printk("hrtime Module Loaded!\n");

    interval = ktime_set(log_sec, log_nsec);

    hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_hrtimer.function = &timer_callback;
    hrtimer_start(&my_hrtimer, interval, HRTIMER_MODE_REL);

    while(i<4)
    {
        threads[i] = kthread_create(thread_function, NULL, "thread");
        kthread_bind(threads[i],i);
        i+=1;
    }

	return 0;
}

static void __exit ModuleExit(void)
{
	hrtimer_cancel(&my_hrtimer);
	printk("Goodbye, Kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);