#include "mbed.h"
#include "zthompson_binaryutils.hpp"
#include "USBSerial.h"

/*
* Author: Zach Thompson
* Date (due): 9/21/21
* Description: Lab_0
*/

// Define the necessary GPIO register addresses
#define PIN0 (uint32_t*) 0x50000000 // General purpose input/output port
#define OUTSET (uint32_t*) 0x50000508 // Set individual bits in GPIO port
#define OUTCLR (uint32_t*) 0x5000050C // Clear individual bits in GPIO port
#define DIRSET (uint32_t*) 0x50000514 // DIR register

// Define diagnostic states
typedef enum {
    NO_ERROR,
    ATTN_REQ,
    FATAL_ERROR,
} State;

Mail<State, 9> mail_box; // Initialize Mail class

// Declare threads
Thread thread1; // led_diag_handler thread
Thread thread2; // diag_tester thread

USBSerial serial; // Initialize serial connection

void led_diag_handler() {
    setbit(DIRSET, 13);
    uint32_t t;
    osEvent event = mail_box.get(0);
    State* priority = (State*) event.value.p;

    while (true) {
        if (*priority == NO_ERROR) {
            serial.printf("NO_ERROR\r\n\n");
            t = 1000;
            mail_box.free(priority);
        }
        if (*priority == ATTN_REQ) {
            serial.printf("ATTN_REQ\r\n\n");
            t = 500;
            mail_box.free(priority);
        }
        if (*priority == FATAL_ERROR) {
            serial.printf("FATAL_ERROR\r\n\n");
             t = 100;
             mail_box.free(priority);
        }

        setbit(OUTSET, 13);
        thread_sleep_for(t);
        setbit(OUTCLR, 13);
        thread_sleep_for(t);
    }
}

void diag_tester()
{
    int count = 0;
    while(true)
    {
        State* priority = mail_box.try_alloc();
        if(count == 0){
            *priority = NO_ERROR;
        }
        if(count == 1){
            *priority = ATTN_REQ;
        }
        if(count == 2){
            *priority = FATAL_ERROR;
            count = -1;
        }
    count++;
    
    mail_box.put(priority);
    thread1.start(callback(led_diag_handler));
    thread_sleep_for(5000);
    }
}

// main() runs in its own thread in the OS
int main()
{
    serial.printf("Start:\r\n\n");
    thread2.start(callback(diag_tester));
}
