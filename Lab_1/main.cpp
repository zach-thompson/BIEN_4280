#include "mbed.h"
#include "zthompson_binaryutils.hpp"
#include "USBSerial.h"
#include <MemoryPool.h>

/*
* Author: Zach Thompson
* Date (due): 10/5/21
* Description: PWM three ways
*/

#define DIRSET (uint32_t*)0x50000514
#define OUTSET (uint32_t*)0x50000508
#define OUTCLR (uint32_t*)0x5000050C

USBSerial serial;

typedef struct {
    int vanilla_dc;
    int chocolate_dc;
} message_t;

MemoryPool<message_t, 18> V_pool;
Queue<message_t, 9> V_queue;
Queue<message_t, 9> C_queue;

Ticker ticks;
Semaphore qMessage(1);

Thread p_thread;
Thread v_thread;
Thread c_thread;

int VDC = 333;
int i = 0;
void tick() {

    if (i <= VDC) {
        setbit(OUTCLR, 16);
    }
    else {
        setbit(OUTSET, 16);
    }

    if (i == 1000) {
        i = -1;
    }
    i++;
}

void Producer() {
    int i = 0;
    int c_flag = 0;
    int c_state = 0;

    while(true)
    {
        if (c_state == 0) {
            c_flag = 0;
        }
        if (c_state >= 10) {
            c_flag = 1;
        }
        switch (c_flag) {
            case 0:
                    c_state++;
                    break;
            case 1:
                    c_state--;
                    break;
            default:
                    c_state++;
                    break;
        }
        if (i >= 10) {
            i = 0;
        }

        message_t *message = V_pool.alloc();
        message -> chocolate_dc = c_state;
        message -> vanilla_dc = c_state;
        V_queue.put(message);
        C_queue.put(message);
        thread_sleep_for(100);
    }
}

void Vanilla() {
    setbit(DIRSET, 16);
    ticks.attach(&tick, 30us);
    while(true){
        osEvent event = V_queue.get(0);
        if (event.status == osEventMessage) {
            message_t* v_state = (message_t*) event.value.p;
            if (v_state -> vanilla_dc == 1) {
                VDC = 1000;
            }
            else if (v_state -> vanilla_dc == 2) {
                VDC = 900;
            }
            else if (v_state -> vanilla_dc == 3) {
                VDC = 800;
            }
            else if (v_state -> vanilla_dc == 4) {
                VDC = 700;
            }
            else if (v_state -> vanilla_dc == 5) {
                VDC = 600;
            }
            else if (v_state -> vanilla_dc == 6) {
                VDC = 500;
            }
            else if (v_state -> vanilla_dc == 7) {
                VDC = 400;
            }
            else if (v_state -> vanilla_dc == 8) {
                VDC = 300;
            }
            else if (v_state -> vanilla_dc == 9) {
                VDC = 200;
            }
            else if (v_state -> vanilla_dc == 10) {
                VDC = 0;
            }
            V_pool.free(v_state);
        }
        thread_sleep_for(100);
    }
}

void Chocolate() {
    PwmOut blue(LED4);
    blue.period_ms(1);

    while (true) {
        qMessage.acquire();
        osEvent event = C_queue.get(0);
        if (event.status == osEventMessage) {
            message_t* dutycycle = (message_t*) event.value.p;
            if(dutycycle -> chocolate_dc == 1){
                blue.write(0.8);
            }
            else if(dutycycle -> chocolate_dc == 2){
                blue.write(0.6);
            }
            else if(dutycycle -> chocolate_dc == 3){
                blue.write(0.4);
            }
            else if(dutycycle -> chocolate_dc == 4){
                blue.write(0.2);
            }
            else if(dutycycle -> chocolate_dc == 5){
               blue.write(0.0);
            }
            else if(dutycycle -> chocolate_dc == 6){
                blue.write(0.2);
            }
            else if(dutycycle -> chocolate_dc == 7){
                blue.write(0.4);
            }
            else if(dutycycle -> chocolate_dc == 8){
                blue.write(0.6);
            }
            else if(dutycycle -> chocolate_dc == 9){
                blue.write(0.8);
            }
            else if(dutycycle -> chocolate_dc == 10){
                blue.write(1.0);
            }
            V_pool.free(dutycycle);
        }
        thread_sleep_for(100);
        qMessage.release();
    }
}

void Strawberry() {
    serial.printf("Strawberry is a terrible flavor of ice cream\r\n");
}

// main() runs in its own thread in the OS
int main()
{
    //serial.printf("Start\r\n");
    p_thread.start(callback(Producer));
    v_thread.start(callback(Vanilla));
    c_thread.start(callback(Chocolate));

    while (true) {}
}