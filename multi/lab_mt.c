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

/* timer callback*/
static enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
	int i = 0;
	hrtimer_forward_now(&my_hrtimer,interval);
	printk("Log - timer callback function");

	while(i<4)
	{
		wake_up_process(threads[i]);
		i+=1;
	}
	return HRTIMER_RESTART;
}


/* function which each thread executes*/
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


	while(i<4)
	{
		threads[i] = kthread_create(thread_function, NULL, "thread");
		kthread_bind(threads[i],i);
		i+=1;
	}

	my_hrtimer.function = &timer_callback;
	hrtimer_start(&my_hrtimer, interval, HRTIMER_MODE_REL);

	while(i<4)
	{
		wake_up_process(threads[i]);
	}

	return 0;
}

static void __exit ModuleExit(void)
{
    int ret;
    
    ret = hrtimer_cancel(&my_hrtimer);
    if (ret) {
        printk(KERN_INFO "The timer was still in use...\n");
    }

	while(i<4)
	{
		kthread_stop(threads[i]);
	}
	printk("Goodbye, Kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);