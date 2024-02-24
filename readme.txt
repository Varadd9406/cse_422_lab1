Varad Deouskar - varad@wustl.edu
Peter Rong - l.rong@wustl.edu

# Module Design

Inside the single/lab.c file, we declared two global static unsigned long variables (log_sec, log_nsec). 
We then used these two lines: 
```module_param(log_sec, ulong, 0)```
```module_param(log_nsec, ulong, 0)```
To define the two ulong module parameters.
We used the 0 permission bit in module_param to hide the module parameters from sysfs.

# Timer Design and Evaluation

We first created a static method call "timer_callback" that returns in type enum hrtimer_restart for the timer's expiration. 
Inside the init functon, we called hrtimer_init, and we used "my_hrtimer.function = &timer_callback;" to have the module's timer variable point to the timer expiration function. Lastly, we used "hrtimer_start(&my_hrtimer, interval, HRTIMER_MODE_REL);" to call hrtimer_start
Inside the exit function, we called "hrtimer_cancel(&my_hrtimer);" to cancel the module's timer.

Part of the system log: 
```
Feb 23 15:53:13 peterrpi kernel: [102015.718385] Log - timer callback function
Feb 23 15:53:13 peterrpi kernel: [102015.818385] Log - timer callback function
Feb 23 15:53:13 peterrpi kernel: [102015.918387] Log - timer callback function
Feb 23 15:53:13 peterrpi kernel: [102016.018389] Log - timer callback function
```
As we can see, there's log of thread waking up approximately every 0.1 seconds, which is really close to what we expect it to perform. (off by at most 0.0001 escond)

# Thread Design and Evaluation

Inside the thread function, we have a printk call that logs the iteration, voluntary and involuntary cs. Right after that, we have ```set_current_state(TASK_INTERRUPTIBLE)``` and calls ```schedule()```. We have a while loop wrapped around these to check if there's ```kthread_should_stop``` flag being true. 
The race condition can be avoided by canceling the timer before stopping the thread, as hrtimer_cancel() will ensure that the timer callback function is not running if it returns 1, indicating that the timer was active. If it returns 0, the timer was not active, and there's no risk of entering the callback function thereafter.

Part of system log for 1 second interval:
```
Feb 23 16:25:06 peterrpi kernel: [103929.096321] Iteration 2: Thread Woken Up, Voluntary CS: 2, Involuntary CS: 0
Feb 23 16:25:07 peterrpi kernel: [103930.096392] Iteration 3: Thread Woken Up, Voluntary CS: 3, Involuntary CS: 0
Feb 23 16:25:08 peterrpi kernel: [103931.096365] Iteration 4: Thread Woken Up, Voluntary CS: 4, Involuntary CS: 0
```

Part of system log for 1,000,000 nanosecond (1 millisecond) interval:
```
Feb 23 16:33:48 peterrpi kernel: [  154.804145] Iteration 8568: Thread Woken Up, Voluntary CS: 8568, Involuntary CS: 4
Feb 23 16:33:48 peterrpi kernel: [  154.805145] Iteration 8569: Thread Woken Up, Voluntary CS: 8569, Involuntary CS: 4
Feb 23 16:33:48 peterrpi kernel: [  154.806146] Iteration 8570: Thread Woken Up, Voluntary CS: 8570, Involuntary CS: 4
Feb 23 16:33:48 peterrpi kernel: [  154.807146] Iteration 8571: Thread Woken Up, Voluntary CS: 8571, Involuntary CS: 4
Feb 23 16:33:48 peterrpi kernel: [  154.808146] Iteration 8572: Thread Woken Up, Voluntary CS: 8572, Involuntary CS: 4
```

1) The time interval varies approximately up to 0.001 seconds using 1 second intervals, and the time intervals varies approximately up to 0.000001 seconds using 1 millisecond intervals. Proportionately, the one with 1 second intervals varies more drastically. 
2) If a higher priority thread becomes runnable, the kernel may preempt the current thread to allow the higher priority one to run first, as we decrease the interval to 1ms, it becomes much more likely a thread might confict with another thread and the scheduler preempted involuntarily.

# Multi-threading Design and Evaluation

The set up is similar to that of a single thread monitor, except that inside ```timer_callback``` and ```ModuleInit``` function we wake up/call all four threads

1 second interval:
```
Feb 23 17:30:20 peterrpi kernel: [ 3546.910838]  CPU - 0
Feb 23 17:30:20 peterrpi kernel: [ 3546.910847] Iteration 90: Thread Woken Up, Voluntary CS: 23, Involuntary CS: 0
Feb 23 17:30:20 peterrpi kernel: [ 3546.910847]  CPU - 1
Feb 23 17:30:20 peterrpi kernel: [ 3546.910856] Iteration 91: Thread Woken Up, Voluntary CS: 23, Involuntary CS: 0
Feb 23 17:30:20 peterrpi kernel: [ 3546.910856]  CPU - 3
Feb 23 17:30:21 peterrpi kernel: [ 3546.910869] Iteration 92: Thread Woken Up, Voluntary CS: 23, Involuntary CS: 0
Feb 23 17:30:21 peterrpi kernel: [ 3546.910869]  CPU - 2
Feb 23 17:30:21 peterrpi kernel: [ 3547.910787] Hey there again
Feb 23 17:30:21 peterrpi kernel: [ 3547.910851] Iteration 93: Thread Woken Up, Voluntary CS: 24, Involuntary CS: 0
Feb 23 17:30:21 peterrpi kernel: [ 3547.910851]  CPU - 3
Feb 23 17:30:21 peterrpi kernel: [ 3547.910860] Iteration 94: Thread Woken Up, Voluntary CS: 24, Involuntary CS: 0
Feb 23 17:30:21 peterrpi kernel: [ 3547.910860]  CPU - 1
Feb 23 17:30:21 peterrpi kernel: [ 3547.910869] Iteration 95: Thread Woken Up, Voluntary CS: 24, Involuntary CS: 0
Feb 23 17:30:21 peterrpi kernel: [ 3547.910869]  CPU - 0
```

100 ms interval: 
```
Feb 23 17:38:16 peterrpi kernel: [ 4022.825856]  CPU - 1
Feb 23 17:38:16 peterrpi kernel: [ 4022.925794] Logging
Feb 23 17:38:16 peterrpi kernel: [ 4022.925841] Iteration 195: Thread Woken Up, Voluntary CS: 50, Involuntary CS: 0
Feb 23 17:38:16 peterrpi kernel: [ 4022.925841]  CPU - 2
Feb 23 17:38:16 peterrpi kernel: [ 4022.925847] Iteration 196: Thread Woken Up, Voluntary CS: 50, Involuntary CS: 6
Feb 23 17:38:16 peterrpi kernel: [ 4022.925847]  CPU - 3
Feb 23 17:38:16 peterrpi kernel: [ 4022.925854] Iteration 197: Thread Woken Up, Voluntary CS: 50, Involuntary CS: 0
Feb 23 17:38:16 peterrpi kernel: [ 4022.925854]  CPU - 0
Feb 23 17:38:16 peterrpi kernel: [ 4022.925860] Iteration 198: Thread Woken Up, Voluntary CS: 50, Involuntary CS: 0
Feb 23 17:38:16 peterrpi kernel: [ 4022.925860]  CPU - 1
Feb 23 17:38:16 peterrpi kernel: [ 4023.025847] Iteration 199: Thread Woken Up, Voluntary CS: 51, Involuntary CS: 0
Feb 23 17:38:16 peterrpi kernel: [ 4023.025847]  CPU - 2
Feb 23 17:38:16 peterrpi kernel: [ 4023.025855] Iteration 199: Thread Woken Up, Voluntary CS: 51, Involuntary CS: 6
Feb 23 17:38:16 peterrpi kernel: [ 4023.025855]  CPU - 3
Feb 23 17:38:16 peterrpi kernel: [ 4023.025862] Iteration 200: Thread Woken Up, Voluntary CS: 51, Involuntary CS: 0
Feb 23 17:38:16 peterrpi kernel: [ 4023.025862]  CPU - 0
```

10 ms interval:
```
Feb 23 17:39:47 peterrpi kernel: [ 4114.108206]  CPU - 0
Feb 23 17:39:47 peterrpi kernel: [ 4114.108211] Iteration 1488: Thread Woken Up, Voluntary CS: 387, Involuntary CS: 0
Feb 23 17:39:47 peterrpi kernel: [ 4114.108211]  CPU - 3
Feb 23 17:39:47 peterrpi kernel: [ 4114.108217] Iteration 1489: Thread Woken Up, Voluntary CS: 387, Involuntary CS: 2
Feb 23 17:39:47 peterrpi kernel: [ 4114.108217]  CPU - 1
Feb 23 17:39:47 peterrpi kernel: [ 4114.108222] Iteration 1490: Thread Woken Up, Voluntary CS: 387, Involuntary CS: 17
Feb 23 17:39:47 peterrpi kernel: [ 4114.108222]  CPU - 2
Feb 23 17:39:47 peterrpi kernel: [ 4114.118162] Logging
Feb 23 17:39:47 peterrpi kernel: [ 4114.118194] Iteration 1491: Thread Woken Up, Voluntary CS: 388, Involuntary CS: 0
Feb 23 17:39:47 peterrpi kernel: [ 4114.118194]  CPU - 0
Feb 23 17:39:47 peterrpi kernel: [ 4114.118199] Iteration 1492: Thread Woken Up, Voluntary CS: 388, Involuntary CS: 17
Feb 23 17:39:47 peterrpi kernel: [ 4114.118199]  CPU - 2
Feb 23 17:39:47 peterrpi kernel: [ 4114.118205] Iteration 1493: Thread Woken Up, Voluntary CS: 388, Involuntary CS: 2
Feb 23 17:39:47 peterrpi kernel: [ 4114.118205]  CPU - 1
```

1 ms interval:
```
Feb 23 17:41:00 peterrpi kernel: [ 4187.040498]  CPU - 0
Feb 23 17:41:00 peterrpi kernel: [ 4187.040499] Iteration 23141: Thread Woken Up, Voluntary CS: 5968, Involuntary CS: 34
Feb 23 17:41:00 peterrpi kernel: [ 4187.040499]  CPU - 2
Feb 23 17:41:00 peterrpi kernel: [ 4187.040501] Iteration 23142: Thread Woken Up, Voluntary CS: 5952, Involuntary CS: 44
Feb 23 17:41:00 peterrpi kernel: [ 4187.040501]  CPU - 3
Feb 23 17:41:00 peterrpi kernel: [ 4187.040504] Iteration 23143: Thread Woken Up, Voluntary CS: 5950, Involuntary CS: 0
Feb 23 17:41:00 peterrpi kernel: [ 4187.040504]  CPU - 1
Feb 23 17:41:00 peterrpi kernel: [ 4187.041465] Logging
Feb 23 17:41:00 peterrpi kernel: [ 4187.041481] Iteration 23144: Thread Woken Up, Voluntary CS: 5951, Involuntary CS: 0
Feb 23 17:41:00 peterrpi kernel: [ 4187.041481]  CPU - 1
Feb 23 17:41:00 peterrpi kernel: [ 4187.041483] Iteration 23145: Thread Woken Up, Voluntary CS: 5969, Involuntary CS: 34
Feb 23 17:41:00 peterrpi kernel: [ 4187.041483]  CPU - 2
Feb 23 17:41:00 peterrpi kernel: [ 4187.041486] Iteration 23146: Thread Woken Up, Voluntary CS: 5953, Involuntary CS: 44
Feb 23 17:41:00 peterrpi kernel: [ 4187.041486]  CPU - 3
Feb 23 17:41:00 peterrpi kernel: [ 4187.041488] Iteration 23147: Thread Woken Up, Voluntary CS: 5956, Involuntary CS: 0
Feb 23 17:41:00 peterrpi kernel: [ 4187.041488]  CPU - 0
```

1) The difference in the actual timestamp is more close to the actual intended interval in the multithread case than the single thread case.
2) If a higher priority thread becomes runnable, the kernel may preempt the current thread to allow the higher priority one to run first, as we decrease the interval to 1ms, it becomes much more likely a thread might confict with another thread and the scheduler preempted involuntarily. Since we are binding a the threads to specific core, as compared to a single thread, it is now more likely to get preempted involuntarily.
3) The number of voluntary context switches increases with the decrease in the sampling interval. This is expected as the system is likely to perform more operations.
    Involuntary Context Switches were absent in the longer intervals but became more prominent in shorter intervals (10 ms and 1 ms), suggesting that as the system's operational granularity increases, so does the scheduler's intervention to forcibly switch contexts, to maintain responsiveness.
    In longer intervals, involuntary context switches are almost non-existent, indicating that threads voluntarily yield the CPU most of the time. However, as we move to finer intervals, involuntary context switches become more common, indicating an increase in scheduler activity to preemptively switch threads, possibly due to higher competition for CPU time.
    In summary, the variation between voluntary and involuntary context switches becomes more pronounced as the observation interval decreases, indicating a competitive CPU scheduling environment.


# System Performance
1) total_exec min: 0.000015 max: 0.000019 mean: 0.000017
2) jitter min: 0.000001 max: 0.000002 mean: 0.0000017
3) single_thread_statistics min: 0.000014 max: 0.000018 mean: 0.0000165 

# Development Effort
This project took the team roughly 2 days work. 