/* 12_sample_system.c

   Create two threads, one byte pool, two message queues, three timers, and
   one counting semaphore. This is an example of multiple object suspension
   using event-chaining, i.e., speedy_thread and slow_thread wait for a
   message to appear on either of two queues  */


   /****************************************************/
   /*    Declarations, Definitions, and Prototypes     */
   /****************************************************/

#include   "tx_api.h"
#include   <stdio.h>

#define     STACK_SIZE         1024
#define     BYTE_POOL_SIZE     9120
#define     NUMBER_OF_MESSAGES 100
#define     MESSAGE_SIZE       TX_1_ULONG
#define     QUEUE_SIZE         MESSAGE_SIZE*sizeof(ULONG)*NUMBER_OF_MESSAGES


/* Define the ThreadX object control blocks...  */

TX_THREAD      speedy_thread;  /* higher priority thread */
TX_THREAD      slow_thread;    /* lower priority thread */

TX_BYTE_POOL   my_byte_pool;   /* byte pool for stacks and queues */
TX_SEMAPHORE   gatekeeper;     /* indicate how many objects available */

TX_QUEUE       queue_1;        /* queue for multiple object suspension */
TX_QUEUE       queue_2;        /* queue for multiple object suspension */

TX_TIMER       stats_timer;    /* generate statistics at intervals */
TX_TIMER       queue_timer_1;  /* send message to queue_1 at intervals */
TX_TIMER       queue_timer_2;  /* send message to queue_2 at intervals */

/* Variables needed to get info about the message queue */
CHAR* info_queue_name;
TX_THREAD* first_suspended;
TX_QUEUE* next_queue;
ULONG     enqueued_1 = 0, enqueued_2 = 0, suspended_count = 0, available_storage = 0;

/* Define the variables used in the sample application...  */
ULONG  speedy_thread_counter = 0, total_speedy_time = 0;
ULONG  slow_thread_counter = 0, total_slow_time = 0;
ULONG  send_message_1[TX_1_ULONG] = { 0X0 }, send_message_2[TX_1_ULONG] = { 0X0 };
ULONG  receive_message_1[TX_1_ULONG], receive_message_2[TX_1_ULONG];

/* speedy_thread and slow_thread entry function prototypes */
void    speedy_thread_entry(ULONG thread_input);
void    slow_thread_entry(ULONG thread_input);

/* timer entry function prototypes */
void    queue_timer_1_entry(ULONG thread_input);
void    queue_timer_2_entry(ULONG thread_input);
void    print_stats(ULONG);

/* event notification function prototypes */
void    queue_1_send_notify(TX_QUEUE* queue_1_ptr);
void    queue_2_send_notify(TX_QUEUE* queue_2_ptr);


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

    CHAR* speedy_stack_ptr;
    CHAR* slow_stack_ptr;
    CHAR* queue_1_ptr;
    CHAR* queue_2_ptr;

    /* Create a byte memory pool from which to allocate the thread stacks. */
    tx_byte_pool_create(&my_byte_pool, "my_byte_pool",
        first_unused_memory, BYTE_POOL_SIZE);

    /* Create threads, queues, the semaphore, timers, and register functions
       for event-chaining */

       /* Allocate the stack for speedy_thread.  */
    tx_byte_allocate(&my_byte_pool, (VOID**)&speedy_stack_ptr, STACK_SIZE,
        TX_NO_WAIT);

    /* Create speedy_thread.  */
    tx_thread_create(&speedy_thread, "speedy_thread", speedy_thread_entry, 0,
        speedy_stack_ptr, STACK_SIZE, 5, 5, TX_NO_TIME_SLICE,
        TX_AUTO_START);

    /* Allocate the stack for slow_thread.  */
    tx_byte_allocate(&my_byte_pool, (VOID**)&slow_stack_ptr, STACK_SIZE,
        TX_NO_WAIT);

    /* Create slow_thread */
    tx_thread_create(&slow_thread, "slow_thread", slow_thread_entry, 1,
        slow_stack_ptr, STACK_SIZE, 15, 15, TX_NO_TIME_SLICE,
        TX_AUTO_START);

    /* Create the message queues used by both threads.  */
    tx_byte_allocate(&my_byte_pool, (VOID**)&queue_1_ptr,
        QUEUE_SIZE, TX_NO_WAIT);

    tx_queue_create(&queue_1, "queue_1", MESSAGE_SIZE,
        queue_1_ptr, QUEUE_SIZE);

    tx_byte_allocate(&my_byte_pool, (VOID**)&queue_2_ptr,
        QUEUE_SIZE, TX_NO_WAIT);

    tx_queue_create(&queue_2, "queue_2", MESSAGE_SIZE,
        queue_2_ptr, QUEUE_SIZE);

    /* Create the gatekeeper semaphore that counts the available objects */
    tx_semaphore_create(&gatekeeper, "gatekeeper", 0);

    /* Create and activate the stats timer */
    tx_timer_create(&stats_timer, "stats_timer", print_stats,
        0x1234, 500, 500, TX_AUTO_ACTIVATE);

    /* Create and activate the timer to send messages to queue_1 */
    tx_timer_create(&queue_timer_1, "queue_timer", queue_timer_1_entry,
        0x1234, 12, 12, TX_AUTO_ACTIVATE);

    /* Create and activate the timer to send messages to queue_2 */
    tx_timer_create(&queue_timer_2, "queue_timer", queue_timer_2_entry,
        0x1234, 9, 9, TX_AUTO_ACTIVATE);

    /* Register the function to increment the gatekeeper semaphore when a
       message is sent to queue_1 */
    tx_queue_send_notify(&queue_1, queue_1_send_notify);

    /* Register the function to increment the gatekeeper semaphore when a
       message is sent to queue_2 */
    tx_queue_send_notify(&queue_2, queue_1_send_notify);
}


/****************************************************/
/*              Function Definitions                */
/****************************************************/


/* Entry function definition of speedy_thread
   it has a higher priority than slow_thread */

void    speedy_thread_entry(ULONG thread_input)
{

    ULONG   start_time, cycle_time = 0, current_time = 0;
    UINT    status;

    /* This is the higher priority speedy_thread */

    while (1)
    {
        /* Get the starting time for this cycle */
        start_time = tx_time_get();

        /* Activity 1:  2 ticks.  */
        tx_thread_sleep(2);

        /* Activity 2:  5 ticks.  */
        /* wait for a message to appear on either one of the two queues */
        tx_semaphore_get(&gatekeeper, TX_WAIT_FOREVER);

        /* Determine whether a message queue_1 or queue_2 is available */
        status = tx_queue_receive(&queue_1, receive_message_1, TX_NO_WAIT);

        if (status == TX_SUCCESS)
            ; /* A message on queue_1 has been found - process */
        else
            /* Receive a message from queue_2 */
            tx_queue_receive(&queue_2, receive_message_2, TX_WAIT_FOREVER);

        tx_thread_sleep(5);

        /* Increment the thread counter and get timing info  */
        speedy_thread_counter++;
        current_time = tx_time_get();
        cycle_time = current_time - start_time;
        total_speedy_time = total_speedy_time + cycle_time;
    }
}

/************************************************************/

/* Entry function definition of slow_thread
   it has a lower priority than speedy_thread */

void    slow_thread_entry(ULONG thread_input)
{

    ULONG   start_time, current_time = 0, cycle_time = 0;
    UINT    status;


    while (1)
    {
        /* Get the starting time for this cycle */
        start_time = tx_time_get();

        /* Activity 3 - sleep 12 ticks.  */
        /* wait for a message to appear on either one of the two queues */
        tx_semaphore_get(&gatekeeper, TX_WAIT_FOREVER);

        /* Determine whether a message queue_1 or queue_2 is available */
        status = tx_queue_receive(&queue_1, receive_message_1, TX_NO_WAIT);

        if (status == TX_SUCCESS)
            ; /* A message on queue_1 has been found - process */
        else
            /* Receive a message from queue_2 */
            tx_queue_receive(&queue_2, receive_message_2, TX_WAIT_FOREVER);

        tx_thread_sleep(12);


        /* Activity 4:  8 ticks.  */
        tx_thread_sleep(8);

        /* Increment the thread counter and get timing info  */
        slow_thread_counter++;

        current_time = tx_time_get();
        cycle_time = current_time - start_time;
        total_slow_time = total_slow_time + cycle_time;
    }
}

/*****************************************************/
/* print statistics at specified times */
void print_stats(ULONG invalue)
{
    ULONG   current_time, avg_slow_time, avg_speedy_time;

    if ((speedy_thread_counter > 0) && (slow_thread_counter > 0))
    {
        current_time = tx_time_get();
        avg_slow_time = total_slow_time / slow_thread_counter;
        avg_speedy_time = total_speedy_time / speedy_thread_counter;
        tx_queue_info_get(&queue_1, &info_queue_name, &enqueued_1,
            &available_storage, &first_suspended,
            &suspended_count, &next_queue);
        tx_queue_info_get(&queue_2, &info_queue_name, &enqueued_2,
            &available_storage, &first_suspended,
            &suspended_count, &next_queue);
        printf("\nEvent-Chaining: 2 threads waiting for 2 queues\n\n");
        printf("     Current Time:                    %lu\n", current_time);
        printf("         speedy_thread counter:       %lu\n", speedy_thread_counter);
        printf("        speedy_thread avg time:       %lu\n", avg_speedy_time);
        printf("           slow_thread counter:       %lu\n", slow_thread_counter);
        printf("          slow_thread avg time:       %lu\n", avg_slow_time);
        printf(" total # queue_1 messages sent:       %lu\n", send_message_1[TX_1_ULONG - 1]);
        printf(" total # queue_2 messages sent:       %lu\n", send_message_2[TX_1_ULONG - 1]);
        printf(" current # messages in queue_1:       %lu\n", enqueued_1);
        printf(" current # messages in queue_2:       %lu\n\n", enqueued_2);

    }
    else printf("Bypassing print_stats function, Current Time: %lu\n", tx_time_get());
}



/*****************************************************/
/* Send a message to queue_1 at specified times */
void queue_timer_1_entry(ULONG invalue)
{

    /* Send a message to queue_1 using the multiple object suspension approach  */
    /* The gatekeeper semaphore keeps track of how many objects are available
       via the notification function */
    send_message_1[TX_1_ULONG - 1]++;
    tx_queue_send(&queue_1, send_message_1, TX_NO_WAIT);

}

/*****************************************************/
/* Send a message to the queue at specified times */
void queue_timer_2_entry(ULONG invalue)
{

    /* Send a message to queue_2 using the multiple object suspension approach  */
    /* The gatekeeper semaphore keeps track of how many objects are available
       via the notification function */
    send_message_2[TX_1_ULONG - 1]++;
    tx_queue_send(&queue_2, send_message_2, TX_NO_WAIT);

}

/*****************************************************/
/* Notification function to increment gatekeeper semaphore
   whenever a message has been sent to queue_1 */
void queue_1_send_notify(TX_QUEUE* queue_ptr_1)
{
    tx_semaphore_put(&gatekeeper);
}

/*****************************************************/
/* Notification function to increment gatekeeper semaphore
   whenever a message has been sent to queue_2 */
void queue_2_send_notify(TX_QUEUE* queue_ptr_2)
{
    tx_semaphore_put(&gatekeeper);
}
