/* 08_sample_system.c

   Create two threads, and one mutex.
   Use arrays for the thread stacks.
   The mutex protects the critical sections.
   Use an application timer to display thread timings.  */

   /****************************************************/
   /*    Declarations, Definitions, and Prototypes     */
   /****************************************************/

#include   "tx_api.h"
#include   <stdio.h>

#define     STACK_SIZE         1024

CHAR stack_speedy[STACK_SIZE];
CHAR stack_slow[STACK_SIZE];


/* Define the ThreadX object control blocks...  */

TX_THREAD               Speedy_Thread;
TX_THREAD               Slow_Thread;

TX_MUTEX                my_mutex;

/* Declare the application timer */
TX_TIMER        stats_timer;

/* Declare the counters and accumulators */
ULONG           Speedy_Thread_counter = 0,
total_speedy_time = 0;
ULONG           Slow_Thread_counter = 0,
total_slow_time = 0;

/* Define prototype for expiration function */
void    print_stats(ULONG);

/* Define thread prototypes.  */

void    Speedy_Thread_entry(ULONG thread_input);
void    Slow_Thread_entry(ULONG thread_input);


/****************************************************/
/*               Main Entry Point                   */
/****************************************************/

/* Define main entry point.  */

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/****************************************************/
/*             Application Definitions              */
/****************************************************/

/* Define what the initial system looks like.  */

void    tx_application_define(void* first_unused_memory)
{


    /* Put system definitions here,
       e.g., thread and mutex creates */

       /* Create the Speedy_Thread.  */
    tx_thread_create(&Speedy_Thread, "Speedy_Thread",
        Speedy_Thread_entry, 0,
        stack_speedy, STACK_SIZE,
        5, 5, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Create the Slow_Thread */
    tx_thread_create(&Slow_Thread, "Slow_Thread",
        Slow_Thread_entry, 1,
        stack_slow, STACK_SIZE,
        15, 15, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Create the mutex used by both threads  */
    tx_mutex_create(&my_mutex, "my_mutex", TX_NO_INHERIT);

    /* Create and activate the timer */
    tx_timer_create(&stats_timer, "stats_timer", print_stats,
        0x1234, 500, 500, TX_AUTO_ACTIVATE);

}


/****************************************************/
/*              Function Definitions                */
/****************************************************/

/* Define the activities for the Speedy_Thread */

void    Speedy_Thread_entry(ULONG thread_input)
{
    UINT    status;
    ULONG   current_time;
    ULONG   start_time, cycle_time;

    while (1)
    {

        /* Get the starting time for this cycle */
        start_time = tx_time_get();

        /* Activity 1:  2 timer-ticks. */
        tx_thread_sleep(2);

        /* Activity 2:  5 timer-ticks   *** critical section ***
        Get the mutex with suspension. */

        status = tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)  break;  /* Check status */

        tx_thread_sleep(5);

        /* Release the mutex. */
        status = tx_mutex_put(&my_mutex);
        if (status != TX_SUCCESS)  break;  /* Check status */

        /* Activity 3:  4 timer-ticks. */
        tx_thread_sleep(4);

        /* Activity 4:  3 timer-ticks   *** critical section ***
        Get the mutex with suspension. */

        status = tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)  break;  /* Check status */

        tx_thread_sleep(3);

        /* Release the mutex. */
        status = tx_mutex_put(&my_mutex);
        if (status != TX_SUCCESS)  break;  /* Check status */

        /* Increment thread counter, compute cycle time & total time */
        Speedy_Thread_counter++;
        current_time = tx_time_get();
        cycle_time = current_time - start_time;
        total_speedy_time = total_speedy_time + cycle_time;

    }
}

/****************************************************/

/* Define the activities for the Slow_Thread */

void    Slow_Thread_entry(ULONG thread_input)
{
    UINT    status;
    ULONG   current_time;
    ULONG   start_time, cycle_time;

    while (1)
    {

        /* Get the starting time for this cycle */
        start_time = tx_time_get();

        /* Activity 5:  12 timer-ticks   *** critical section ***
        Get the mutex with suspension. */

        status = tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)  break;  /* Check status */

        tx_thread_sleep(12);

        /* Release the mutex. */
        status = tx_mutex_put(&my_mutex);
        if (status != TX_SUCCESS)  break;  /* Check status */

        /* Activity 6:  8 timer-ticks. */
        tx_thread_sleep(8);

        /* Activity 7:  11 timer-ticks   *** critical section ***
        Get the mutex with suspension. */

        status = tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)  break;  /* Check status */

        tx_thread_sleep(11);

        /* Release the mutex. */
        status = tx_mutex_put(&my_mutex);
        if (status != TX_SUCCESS)  break;  /* Check status */

        /* Activity 8:  9 timer-ticks. */
        tx_thread_sleep(9);

        /* Increment thread counter, compute cycle time & total time */
        Slow_Thread_counter++;
        current_time = tx_time_get();
        cycle_time = current_time - start_time;
        total_slow_time = total_slow_time + cycle_time;

    }
}

/****************************************************/

/* Display statistics at periodic intervals */
void print_stats(ULONG invalue)
{
    ULONG   current_time, avg_slow_time, avg_speedy_time;

    if ((Speedy_Thread_counter > 0) && (Slow_Thread_counter > 0))
    {
        current_time = tx_time_get();
        avg_slow_time = total_slow_time / Slow_Thread_counter;
        avg_speedy_time = total_speedy_time / Speedy_Thread_counter;

        printf("\n**** Timing Info Summary\n\n");
        printf("Current Time:               %lu\n", current_time);
        printf("  Speedy_Thread counter:  %lu\n", Speedy_Thread_counter);
        printf(" Speedy_Thread avg time:  %lu\n", avg_speedy_time);
        printf("    Slow_Thread counter:  %lu\n", Slow_Thread_counter);
        printf("   Slow_Thread avg time:  %lu\n\n", avg_slow_time);
    }
    else printf("Bypassing print_stats, Time: %lu\n", tx_time_get());
}
