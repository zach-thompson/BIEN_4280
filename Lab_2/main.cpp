#include "mbed.h"
#include "zthompson_binaryutils.hpp"
#include "USBSerial.h"

/*
* Author: Zach Thompson
* Date (due): 10/15/21
* Description: Sense and calibrate the humidity and temperature
*/

// Define registers
#define OUTCLR (uint32_t*)0x5000050C // Set
#define OUTSET (uint32_t*)0x50000508 // Clear
#define DIRSET (uint32_t*)0x50000514 // Direction

// Define flags
#define HUMIDITY (1UL << 0)
#define TEMPERATURE (1UL << 9)

// Define threads
Thread humidity;
Thread temperature;

// Initializations
USBSerial serial;
Ticker tick;
EventFlags event;
I2C i2c(I2C_SDA1, I2C_SCL1);


int change = 0;
void ticks() {
    if (change == 1) {
        event.set(TEMPERATURE);
        change = 0;
    }
    else {
        event.set(HUMIDITY);
        change = 1;
    }
}

void read_humidity() {
    while (true){
        if (event.wait_all(HUMIDITY)){
            event.clear(HUMIDITY);
            serial.printf("Reading humidity\r\n\n");
            setbit(OUTCLR, 6); // Blue on
            thread_sleep_for(1);
            setbit(OUTSET, 16); // Green off
            thread_sleep_for(1000);
        }
    }
}

void read_temperature() {
    while (true) {
        if(event.wait_all(TEMPERATURE)){
            event.clear(TEMPERATURE);
            serial.printf("Reading temperature\r\n\n");
            setbit(OUTCLR, 16); //pin 16 is the Green LED
            thread_sleep_for(1);
            setbit(OUTSET, 6); //turn off blue
            thread_sleep_for(1000);
        }
        else{
        }
        thread_sleep_for(100);
    }
}

const int read_addr = 0xBF;
const int write_addr = 0xBE;
uint8_t whoami_addr[] = {0x0F};
int info;
char read_data[] ={0, 0};
char sub_addr[10];
uint8_t data_buffer[2];
uint16_t temp_out;
void measure_temp() {
    // Confirm i2c connection with sensor   
    info = i2c.write(write_addr, (const char *) whoami_addr, 1, true);
    if (info != 0 ) {
       serial.printf("Write: failed\n\r");
       serial.printf("Returned: %i\r\n", info);
    }
    if (i2c.read(read_addr, read_data, 1)  != 0 ) {
       serial.printf("Read: failed\n\r"); 
       serial.printf("Returned: %i\r\n", i2c.read(read_addr, read_data, 1));     
    }
    if (read_data[0] != 0xBC) {
        serial.printf("Who am I: Failed\n\r");
        serial.printf("Returned: %x\r\n", read_data[0]);
    } 
    
    // Measure temperature
    read_data[0] = 0x20; // Control Reg 1
    read_data[1] = 0x84; // Turn on temp sensor
    info = i2c.write(read_addr, read_data, 2);    
    while (true) {        
        read_data[0] = 0x21; // Control Reg 2
        read_data[1] = 0x1; // One shot reading
        info = i2c.write(read_addr, read_data, 2);

        sub_addr[0] = 0x2A; // LSB Temperature
        i2c.write(write_addr, (const char *) sub_addr, 1, true);
        i2c.read(read_addr, read_data, 1);
        data_buffer[0] = ((uint8_t)read_data[0]);

        sub_addr[0] = 0x2B; // MSB Temperature
        i2c.write(write_addr, (const char *) sub_addr, 1, true);
        i2c.read(read_addr, read_data, 1);
        data_buffer[1] = read_data[0];

        temp_out = data_buffer[0] | (data_buffer[1] << 8);

        serial.printf("Temperature: %d\n\r", temp_out);
        thread_sleep_for(1000);
    }
}

// main() runs in its own thread in the OS
int main()
{
    DigitalOut pullup(P1_0);
    pullup.write(1);
    DigitalOut vcc(P0_22);
    vcc.write(1);
    thread_sleep_for(100);
    //humidity.start(callback(measure_humidity));
    temperature.start(callback(measure_temp));
    //tick.attach(&ticks, 1000ms); // Only used for Part 1
    while (true) {
    }
}
