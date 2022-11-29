#include "mbed.h"
#include "USBSerial.h"

/*
* Author: Zach Thompson
* Date: 12/17/21
* Description: Midterm (Flavor 1) - Using proximity sensor
*/

// Initialize serial connection
USBSerial serial;

// Establish i2c communications
I2C i2c(I2C_SDA1, I2C_SCL1);

// Initialize threads
Thread prox_thread; // Handshake, measuring proximity
Thread led_thread; // LED

// Set up mailbox to store power levels
typedef struct {
    uint32_t State;
} mail_t;
Mail<mail_t, 5> mail_box;

// Create levels of power to be added to mailbox
enum power {off, one, two, three, four, five};

// Make initializations to perform a "handshake"
const int read_addr = 0x73; // read address
const int write_addr = 0x72; // write addrss
uint8_t id_addr[] = {0x92}; // Device ID address
char read_data[] = {0, 0}; // Empty array
int connect_w; // Store i2c.write return
int connect_r; // Store i2c.read return

// Make initialzations for proximity readings
const char proximity_enable[] = {0x80, 5}; // Proximity interrupt enable address
const char proximity_data[] = {0x9C}; // Proximity data address
const char gain_enable[] = {0x9F,32}; // Proximity gain enable address
const char gain[] = {0x8F,12}; // Proximity gain control address
char data[] = {0,0}; // Empty array to write into
uint8_t proximity; // Store piece of data[]
uint8_t calibrated; // Calibrated proximity value converted into cm

// Perform handshake with the sensor. Measure proximity and calibrate using ruler provided
void handshake() {
    // Initiate a handshake
    if (true) {
        connect_w = i2c.write(write_addr, (const char *) id_addr, 1, true); // Reach out
        connect_r = i2c.read(read_addr, read_data, 1); // Insert ET touching fingertips .gif
        serial.printf("Handshake attempted: ");
        if ((connect_w == 0) & (connect_r == 0)) {
            serial.printf("Successful\r\n");
            serial.printf("Sensor ID: %X\r\n", read_data[0]);
        }
        else {
            serial.printf("Diplomacy failed\r\n");
            serial.printf("connect_w: %i\r\n", connect_w);
            serial.printf("connect_r: %i\r\n", connect_r);
        }
    }
    
    // Enable necessary registers
    i2c.write(write_addr, proximity_enable, 2, false);
    i2c.write(write_addr, gain_enable, 2, false);
    i2c.write(write_addr, gain, 2, false);

    // Read proximity from sensor
    while (true) {
        mail_t *mail = mail_box.alloc();
        i2c.write(write_addr, proximity_data, 1, true);
        i2c.read(read_addr, data, 1);
        proximity = data[0];

        serial.printf("Proximity: %ucm\r\n", proximity);

       /* if (proximity > 4) {
            calibrated = 8.35 * exp((-0.0065) * proximity);
            serial.printf("Calibrated: %ucm\r\n", calibrated);
       */ }
           
        // Send appropriate power level to mailbox based on proximity value
        if ((proximity > 4) && (proximity <= 51)) { // Sensor doesn't consistently return < 3
            mail -> State = one;
        }
        else if ((proximity > 51) && (proximity <= 102)) {
            mail -> State = two;
        }
        else if ((proximity > 102) && (proximity <= 153)) {
            mail -> State = three;
        }
        else if ((proximity > 153) && (proximity <= 204)) {
            mail-> State = four;
        }
        else if (proximity > 204) {
            mail -> State = five;
        }
        else {
            mail -> State = off;
        }
        mail_box.put(mail);
        thread_sleep_for(100);
   }
}

// Adjust the brightness of the indicator LED based on power setting sent from mail
void setLED(void) {
    PwmOut green(LED3);
    PwmOut red(LED2);
    green.period_ms(1);
    red.period_ms(1);

    while (true) {
        osEvent event = mail_box.get(0);
        if (event.status == osEventMail) {
            mail_t* mail = (mail_t*)event.value.v;
            if (mail -> State == off) {
                green.write(1.0);
                red.write(0.0);
            } 
            if (mail -> State == one) {
                green.write(0.8);
                red.write(1.0);
            }
            if (mail -> State == two) {
                green.write(0.6);
                red.write(1.0);
            }
            if (mail -> State == three) {
                green.write(0.4);
                red.write(1.0);
            }
            if (mail -> State == four) {
                green.write(0.2);
                red.write(1.0);
            }
            if (mail -> State == five) {
                green.write(0.0);
                red.write(1.0);
            }
            mail_box.free(mail);
        }
    }
}

// main() runs in its own thread in the OS
int main() {
    DigitalOut pullup(P1_0); 
    pullup.write(1);      
    DigitalOut vcc(P0_22);
    vcc.write(1);
    thread_sleep_for(100);
    prox_thread.start(callback(handshake));
    thread_sleep_for(100);
    led_thread.start(callback(setLED));

    while (true) {}
}