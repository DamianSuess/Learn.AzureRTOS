/* 02_sample_system.c

   Create two threads, one byte pool, and one mutex.
   The threads cooperate with each other via the mutex.  */


   /****************************************************/
   /*    Declarations, Definitions, and Prototypes     */
   /****************************************************/

#include "tx_api.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         1024
#define     DEMO_BYTE_POOL_SIZE     9120


/* Define the ThreadX object control blocks...  */

TX_THREAD               Speedy_Thread;
TX_THREAD               Slow_Thread;

TX_MUTEX                my_mutex;

TX_BYTE_POOL            my_byte_pool;


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

    CHAR* pool_pointer;


    /* Create a byte memory pool from which to allocate
       the thread stacks.  */
    tx_byte_pool_create(&my_byte_pool, "my_byte_pool",
        first_unused_memory,
        DEMO_BYTE_POOL_SIZE);

    /* Put system definition stuff in here, e.g., thread
       creates and other assorted create information.  */

       /* Allocate the stack for the Speedy_Thread.  */
    tx_byte_allocate(&my_byte_pool, (VOID**)&pool_pointer,
        DEMO_STACK_SIZE, TX_NO_WAIT);

    /* Create the Speedy_Thread.  */
    tx_thread_create(&Speedy_Thread, "Speedy_Thread",
        Speedy_Thread_entry, 0,
        pool_pointer, DEMO_STACK_SIZE, 5, 5,
        TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the stack for the Slow_Thread.  */
    tx_byte_allocate(&my_byte_pool, (VOID**)&pool_pointer,
        DEMO_STACK_SIZE, TX_NO_WAIT);

    /* Create the Slow_Thread.  */
    tx_thread_create(&Slow_Thread, "Slow_Thread",
        Slow_Thread_entry, 1, pool_pointer,
        DEMO_STACK_SIZE, 15, 15,
        TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Create the mutex used by both threads  */
    tx_mutex_create(&my_mutex, "my_mutex", TX_NO_INHERIT);


}


/****************************************************/
/*              Function Definitions                */
/****************************************************/


/* Entry function definition of the "Speedy_Thread"
   it has a higher priority than the "Slow_Thread" */

void  Speedy_Thread_entry(ULONG thread_input)
{

    ULONG current_time;

    while (1)
    {
        /* Activity 1:  2 timer-ticks */
        tx_thread_sleep(2);

        /* Get the mutex with suspension */
        tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);

        /* Activity 2:  5 timer-ticks  *** critical section *** */
        tx_thread_sleep(5);

        /* Release the mutex */
        tx_mutex_put(&my_mutex);

        /* Activity 3:  4 timer-ticks */
        tx_thread_sleep(4);

        /* Get the mutex with suspension */
        tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);

        /* Activity 4:  3 timer-ticks  *** critical section *** */
        tx_thread_sleep(3);

        /* Release the mutex */
        tx_mutex_put(&my_mutex);

        current_time = tx_time_get();
        printf("Current Time: %5lu  Speedy_Thread finished a cycle...\n",
            current_time);

    }
}

/****************************************************/

/* Entry function definition of the "Slow_Thread"
   it has a lower priority than the "Speedy_Thread" */

void  Slow_Thread_entry(ULONG thread_input)
{


    ULONG current_time;

    while (1)
    {
        /* Activity 5 - 12 timer-ticks  *** critical section ***  */

        /* Get the mutex with suspension */
        tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);

        tx_thread_sleep(12);

        /* Release the mutex  */
        tx_mutex_put(&my_mutex);

        /* Activity 6 - 8 timer-ticks  */
        tx_thread_sleep(8);

        /* Activity 7 - 11 timer-ticks  *** critical section ***  */

        /* Get the mutex with suspension  */
        tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);

        tx_thread_sleep(11);

        /* Release the mutex  */
        tx_mutex_put(&my_mutex);

        /* Activity 8 - 9 timer-ticks  */
        tx_thread_sleep(9);

        current_time = tx_time_get();
        printf("Current Time: %5lu  Slow_Thread finished a cycle...\n",
            current_time);

    }
}
