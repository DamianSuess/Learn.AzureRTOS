/* 13_case_study.c

   Implement a simplified version of a real-time, video/audio/motion (VAM)
   recording system.

   Create three threads named: initializer, data_capture, event_recorder
   Create one byte pool for thread stacks and message queue: my_byte_pool
   Create one mutex to guard protected memory: memory_mutex
   Create one message queue to store event notices: event_notice
   Create nine application timers named: crash_interrupt, unsafe_interrupt,
    warning_interrupt, manual_interrupt, crash_copy_scheduler,
    unsafe_copy_scheduler, manual_copy_scheduler, stats_timer

   For this system, assume that each timer-tick represents one second  */

   /****************************************************/
   /*    Declarations, Definitions, and Prototypes     */
   /****************************************************/

#include  "tx_api.h"
#include  <stdio.h>

#define   STACK_SIZE         1024
#define   BYTE_POOL_SIZE     9120
#define   MAX_EVENTS           16
#define   MAX_TEMP_MEMORY     200


/* Define the ThreadX object control blocks */

TX_THREAD     initializer;
TX_THREAD     data_capture;
TX_THREAD     event_recorder;

TX_QUEUE      event_notice;

TX_MUTEX      memory_mutex;
TX_BYTE_POOL  my_byte_pool;

TX_TIMER      crash_interrupt;
TX_TIMER      unsafe_interrupt;
TX_TIMER      warning_interrupt;
TX_TIMER      manual_interrupt;

TX_TIMER      crash_copy_scheduler;
TX_TIMER      unsafe_copy_scheduler;
TX_TIMER      warning_copy_scheduler;
TX_TIMER      manual_copy_scheduler;
TX_TIMER      stats_timer;


/* Define the counters and variables used in the VAM system  */

ULONG  num_crashes = 0, num_unsafe = 0, num_warning = 0, num_manual = 0;
ULONG  frame_index, event_count, frame_data[2];

/* Define the arrays used to represent temporary memory       */
/* and protected memory. temp_memory contains pair of data    */
/* in the form time-data and protected_memory contains rows   */
/* of 26 elements in the form time-priority-data-data-data... */
/* The working index to temp_memory is frame_index and the    */
/* working index to protected_memory is event_count.          */

ULONG  temp_memory[MAX_TEMP_MEMORY][2],
protected_memory[MAX_EVENTS][26];

/* Define thread and function prototypes.  */

void  initializer_process(ULONG);
void  data_capture_process(ULONG);
void  event_recorder_process(ULONG);
void  crash_ISR(ULONG);
void  unsafe_ISR(ULONG);
void  warning_ISR(ULONG);
void  manual_ISR(ULONG);
void  crash_copy_activate(ULONG);
void  unsafe_copy_activate(ULONG);
void  warning_copy_activate(ULONG);
void  manual_copy_activate(ULONG);
void  print_stats(ULONG);


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

void  tx_application_define(void* first_unused_memory)
{

    CHAR* byte_pointer;

    /* Put system definition stuff in here, e.g., thread creates
       and other assorted create information.  */

       /* Create a memory byte pool from which to allocate
          the thread stacks.  */
    tx_byte_pool_create(&my_byte_pool, "my_byte_pool",
        first_unused_memory, BYTE_POOL_SIZE);

    /* Allocate the stack for the initializer thread.  */
    tx_byte_allocate(&my_byte_pool, (VOID**)&byte_pointer,
        STACK_SIZE, TX_NO_WAIT);

    /* Create the initializer thread.  */
    tx_thread_create(&initializer, "initializer",
        initializer_process, 0,
        byte_pointer, STACK_SIZE, 11, 11,
        TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the stack for the data_capture thread.  */
    tx_byte_allocate(&my_byte_pool, (VOID**)&byte_pointer,
        STACK_SIZE, TX_NO_WAIT);

    /* Create the data_capture thread.  */
    tx_thread_create(&data_capture, "data_capture",
        data_capture_process, 0,
        byte_pointer, STACK_SIZE, 15, 15,
        TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the stack for the event_recorder thread.  */
    tx_byte_allocate(&my_byte_pool, (VOID**)&byte_pointer,
        STACK_SIZE, TX_NO_WAIT);

    /* Create the event_recorder thread.  */
    tx_thread_create(&event_recorder, "event_recorder",
        event_recorder_process, 0,
        byte_pointer, STACK_SIZE, 12, 12,
        TX_NO_TIME_SLICE, TX_DONT_START);

    /* Create and activate the 4 timers to simulate interrupts */
    tx_timer_create(&crash_interrupt, "crash_interrupt", crash_ISR,
        0x1234, 1444, 1444, TX_AUTO_ACTIVATE);
    tx_timer_create(&unsafe_interrupt, "unsafe_interrupt", unsafe_ISR,
        0x1234, 760, 760, TX_AUTO_ACTIVATE);
    tx_timer_create(&warning_interrupt, "warning_interrupt", warning_ISR,
        0x1234, 410, 410, TX_AUTO_ACTIVATE);
    tx_timer_create(&manual_interrupt, "manual_interrupt", manual_ISR,
        0x1234, 888, 888, TX_AUTO_ACTIVATE);

    /* Create and activate the 4 timers to initiate data copying */
    tx_timer_create(&crash_copy_scheduler, "crash_copy_scheduler",
        crash_copy_activate, 0x1234, 12, 12, TX_NO_ACTIVATE);
    tx_timer_create(&unsafe_copy_scheduler, "unsafe_copy_scheduler",
        unsafe_copy_activate, 0x1234, 12, 12, TX_NO_ACTIVATE);
    tx_timer_create(&warning_copy_scheduler, "warning_copy_scheduler",
        warning_copy_activate, 0x1234, 12, 12, TX_NO_ACTIVATE);
    tx_timer_create(&manual_copy_scheduler, "manual_copy_scheduler",
        manual_copy_activate, 0x1234, 12, 12, TX_NO_ACTIVATE);

    /* Create and activate the timer to print statistics periodically */
    tx_timer_create(&stats_timer, "stats_timer", print_stats,
        0x1234, 1000, 1000, TX_AUTO_ACTIVATE);

    /* Create the message queue that holds the frame_indexes for all events.  */
    /* The frame_index is a position marker for the temp_memory array.        */
    /* Whenever an event occurs, the event ISR sends the current frame_index  */
    /* and event priority to the queue for storing crash event information.   */
    /* First, allocate memory space for the queue, then create the queue.     */
    tx_byte_allocate(&my_byte_pool, (VOID**)&byte_pointer,
        MAX_EVENTS * 2 * sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&event_notice, "event_notice", TX_2_ULONG,
        byte_pointer, MAX_EVENTS * 2 * sizeof(ULONG));

}


/**********************************************************/
/*                 Function Definitions                   */
/**********************************************************/


/* Entry function definition of the initializer thread */

void  initializer_process(ULONG thread_input)
{
    /* Perform initialization tasks                              */
    /* Because we are using arrays to represent files, there is  */
    /* very little initialization to perform. We initialize two  */
    /* global variables that represent starting array indexes.   */
    printf("VAM System - Trace of Event Activities Begins...\n\n");
    frame_index = 0;
    event_count = 0;
}

/************************************************************/
/* Entry function definition of the data_capture thread */

void    data_capture_process(ULONG thread_input)
{
    /* Perform data capture from the VAM system to temporary memory  */
    /* This function simulates the data capture operation by writing */
    /* to an array, which represents a temporary memory file. For    */
    /* simplicity, we will write to the array once every timer-tick   */
    while (1) {
        temp_memory[frame_index][0] = tx_time_get();
        temp_memory[frame_index][1] = 0x1234;
        frame_index = (frame_index + 1) % MAX_TEMP_MEMORY;
        tx_thread_sleep(1);
    }
}


/************************************************************/
/********** crash event detection and processing ************/
/************************************************************/

/* Timer function definition for simulated crash interrupt       */
/* This is a simulated ISR -- an actual ISR would probably begin */
/* with a context save and would end with a context restore.     */

void    crash_ISR(ULONG timer_input)
{
    ULONG frame_data[2];
    frame_data[0] = frame_index;
    frame_data[1] = 1;
    num_crashes++;
    /* Activates timer to expire in 12 seconds - end of event */
    /* Put frame_index and priority on queue for crash events */
    tx_queue_send(&event_notice, frame_data, TX_NO_WAIT);
    tx_timer_activate(&crash_copy_scheduler);
}
/************************************************************/
/* Timer function definition for timer crash_copy_scheduler,   */
/* which resumes thread that performs recording of crash data  */

void    crash_copy_activate(ULONG timer_input)
{
    /* resume recorder thread to initiate data recording */
    tx_thread_resume(&event_recorder);
    tx_timer_deactivate(&crash_copy_scheduler);
}

/************************************************************/
/**** Entry function definition of thread event_recorder ****/
/************************************************************/

void    event_recorder_process(ULONG thread_input)
{
    ULONG frame, event_priority, event_time, index, frame_data[2];
    while (1)
    {
        /* Copy an event from temporary memory to protected memory.    */
        /* Get frame_index from event message queue and copy 24 frames */
        /* from temp_memory to protected_memory.                       */
        tx_queue_receive(&event_notice, frame_data, TX_NO_WAIT);
        /* Store event time and event priority in protected memory */
        frame = frame_data[0];
        event_priority = frame_data[1];
        event_time = temp_memory[frame][0];
        printf("**Event**   Time: %5lu   Count: %2lu   Pri: %lu",
            event_time, event_count, event_priority);
        if (event_count < MAX_EVENTS)
        {
            tx_mutex_get(&memory_mutex, TX_WAIT_FOREVER);
            protected_memory[event_count][0] = event_time;
            protected_memory[event_count][1] = event_priority;
            if (frame < 11)
                frame = (MAX_TEMP_MEMORY - 1) - (frame_index + 1);
            for (index = 0; index < 24; index++)
            {
                protected_memory[event_count][index + 2] = temp_memory[frame][1];
                frame = (frame + 1) % MAX_TEMP_MEMORY;
            }
            tx_mutex_put(&memory_mutex);
            event_count++;
        }
        else printf(" **not processed**");
        printf("\n");
        tx_thread_suspend(&event_recorder);
    }
}

/************************************************************/
/********** unsafe event detection and processing ***********/
/************************************************************/

/* Timer function definition for simulated unsafe interrupt      */
/* This is a simulated ISR -- an actual ISR would probably begin */
/* with a context save and would end with a context restore.     */

void    unsafe_ISR(ULONG timer_input)
{
    ULONG frame_data[2];
    frame_data[0] = frame_index;
    frame_data[1] = 2;
    num_unsafe++;
    /* Activates timer to expire in 12 seconds - end of event  */
    /* Put frame_index and priority on queue for unsafe events */
    tx_queue_send(&event_notice, frame_data, TX_NO_WAIT);
    tx_timer_activate(&unsafe_copy_scheduler);
}
/************************************************************/
/* Timer function definition for timer unsafe_copy_scheduler,   */
/* which resumes thread that performs recording of unsafe data  */

void    unsafe_copy_activate(ULONG timer_input)
{
    /* resume event_recorder thread to initiate data recording */
    tx_thread_resume(&event_recorder);
    tx_timer_deactivate(&unsafe_copy_scheduler);
}


/************************************************************/
/********* warning event detection and processing ***********/
/************************************************************/

/* Timer function definition for simulated warning interrupt     */
/* This is a simulated ISR -- an actual ISR would probably begin */
/* with a context save and would end with a context restore.     */

void    warning_ISR(ULONG timer_input)
{
    ULONG frame_data[2];
    frame_data[0] = frame_index;
    frame_data[1] = 3;
    num_warning++;
    /* Activates timer to expire in 12 seconds - end of event   */
    /* Put frame_index and priority on queue for warning events */
    tx_queue_send(&event_notice, frame_data, TX_NO_WAIT);
    tx_timer_activate(&warning_copy_scheduler);
}
/************************************************************/
/* Timer function definition for timer warning_copy_scheduler,   */
/* which resumes thread that performs recording of warning data  */

void    warning_copy_activate(ULONG timer_input)
{
    /* resume event_recorder thread to initiate data recording */
    tx_thread_resume(&event_recorder);
    tx_timer_deactivate(&warning_copy_scheduler);
}


/************************************************************/
/********** manual event detection and processing ***********/
/************************************************************/

/* Timer function definition for simulated manual interrupt      */
/* This is a simulated ISR -- an actual ISR would probably begin */
/* with a context save and would end with a context restore.     */

void    manual_ISR(ULONG timer_input)
{
    ULONG frame_data[2];
    frame_data[0] = frame_index;
    frame_data[1] = 4;
    num_manual++;
    /* Activates timer to expire in 12 seconds - end of event  */
    /* Put frame_index and priority on queue for manual events */
    tx_queue_send(&event_notice, frame_data, TX_NO_WAIT);
    tx_timer_activate(&manual_copy_scheduler);
}
/************************************************************/
/* Timer function definition for timer manual_copy_scheduler,   */
/* which resumes thread that performs recording of manual data  */

void    manual_copy_activate(ULONG timer_input)
{
    /* resume event_recorder thread to initiate data recording */
    tx_thread_resume(&event_recorder);
    tx_timer_deactivate(&manual_copy_scheduler);
}


/************************************************************/
/*********** print statistics at specified times ************/
/************************************************************/

void print_stats(ULONG invalue)
{
    UINT row, col;
    printf("\n\n**** VAM System Periodic Event Summary\n\n");
    printf("     Current Time:               %lu\n", tx_time_get());
    printf("       Number of Crashes:        %lu\n", num_crashes);
    printf("       Number of Unsafe Events:  %lu\n", num_unsafe);
    printf("       Number of Warnings:       %lu\n", num_warning);
    printf("       Number of Manual Events:  %lu\n", num_manual);

    if (event_count > 0)
    {
        printf("\n\n**** Portion of Protected Memory Contents\n\n");
        printf("%6s%6s%6s\n", "Time", "Pri", "Data");
        for (row = 0; row < event_count; row++)
        {
            for (col = 0; col < 8; col++)
                printf("%6lu", protected_memory[row][col]);
            printf("    (etc.)\n");
        }
    }
    if (event_count >= MAX_EVENTS)
        printf("   Warning: Protected Memory is full...\n\n");
}
