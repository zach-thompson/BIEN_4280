/*
Name: Emma Claire Kinnison
Date: 10/19/21
Comment: This code is the sad sad midterm of Emma Claire Kinnison
Sources: https://cdn.sparkfun.com/assets/learn_tutorials/3/2/1/Avago-APDS-9960-datasheet.pdf
*/

#include "mbed.h"
#include <USBSerial.h>
#include "AudioFile.h"
#include <cstdint>
#include <iostream>
#include <cmath>
#include "CCelano_binaryutils.hpp"
#include <Mail.h>


#define LED_BUILTIN_RGB   ((uint32_t *)0x50000508)  //GPIO 0.###
#define LED_BUILTIN_clr ((uint32_t *)0x5000050C)    //CLEAR
#define LED_BUILTIN_dir ((uint32_t *)0x50000514)    //DIRECTION


#define PWM_COUNTER_TOP 10000 // 1MHz divided by 10000-> 1ms

#define mic_start ((uint32_t *)0x4001D000)    //PDM register base
#define mic_stop ((uint32_t *)0x4001D004)    //PDM register base
#define mic_event_started ((uint32_t *)0x4001D100)    //PDM register base
#define mic_event_stoppedd ((uint32_t *)0x4001D104)    //PDM register base
#define mic_events_end ((uint32_t *)0x4001D108)    //PDM register base
#define mic_enable ((uint32_t *)0x4001D500)    //PDM clock generator
#define mic_PDM_CLK_CTRL ((uint32_t *)0x4001D504)    //PDM mode
#define mic_mode ((uint32_t *)0x4001D508)    //PDM mode
#define GAINL ((uint32_t *)0x4001D518)    //PDM Left Gain
#define GAINR ((uint32_t *)0x4001D51c)    //PDM Right Gain
#define PDM_Ratio ((uint32_t *)0x4001D520)    //PDM ratio
#define mic_clock ((uint32_t *)0x4001D540)    //PDM ratio
#define mic_output ((uint32_t *)0x4001D544)    //PDM register base
#define easy_DMA ((uint32_t *)0x4001D560)    //PDM register base
#define easy_DMA_mem ((uint32_t *)0x4001D564)    //PDM register base

uint32_t max_gain = 0x50;   //gain 20 db

DigitalOut vdd(P0_26);
DigitalOut mic_on(P0_17);

Ticker flipper;
USBSerial ser;

//create the mailbox
Mail<int, 50> mail_box;

Thread thread;
Thread thread2;
Thread thread3;
Thread thread5;
Thread tempthread;
// resource_mutex1;
//Mutex resource_mutex2;
Mutex resource_mutex3;
Mutex resource_mutex4;

I2C i2c(I2C_SDA1, I2C_SCL1);
uint8_t databuf[9];
const int readaddr = 0xBF;
const int writeaddr = 0xBE;
uint8_t whoamiaddr[] = {0x0F};
int resp=4;
char subaddr[10]; // LSB Temperature
char readData[] ={0, 0};
int a = 1;
int b = 1;
int16_t TOUT = 0;
int maskT0 = 0;
int maskT1 = 0;
uint8_t maskedT0MSB = 0;
uint8_t maskedT1MSB = 0;
uint16_t T0_degC_x8 = 0;
uint16_t T0_degC = 0;
uint16_t T1_degC_x8 = 0;
uint16_t T1_degC = 0;
int16_t T0_OUT = 0;
int16_t T1_OUT = 0;
int16_t slope = 0;
int16_t yIntercept = 0;
int16_t Y = 0;

//enum to create the different blinking speeds
enum color4PWM{
    RED,
    GREEN,
    BLUE
};

uint16_t RedLowRead=1;
uint16_t BlueLowRead=1;
uint16_t GreenLowRead=1;
uint64_t ClearLowRead=1;

uint16_t RedOverall=1;
uint16_t GreenOverall=1;
uint16_t BlueOverall=1;
uint16_t ClearOverall=1;


//part 1
void colorimetry(void){
    
    char slave[] = {0x39};
    char handshake[] = {0x92};  //this is the ID register found on page 25 and is correct
    const int read_reg = (uint8_t)0x73;  //correct
    const int write_reg = (uint8_t)0x72; //correct

    int resp=1;
    char readData[] ={10, 10};
    int i=0;

    DigitalOut i2cbuspull(P1_0); // Pull up i2C. resistor.
    i2cbuspull.write(1);
    
    thread_sleep_for(100);
    //no power needed https://www.etechnophiles.com/arduino-nano-33-ble-sense-pinout-introduction-specifications/
    //this is incorrect
    DigitalOut vcc(P0_22); // Supply power to all of the sensors (VCC)
    vcc.write(1);
    while(true){
        if(i==0){
            //readData[0] = 0x92;
            resp = i2c.write(write_reg, (const char *) handshake, 1, true);
            if(  resp != 0 ){
                ser.printf("I failed to talk at the Color sensor. (Returned: %d)\n\r", resp);           
            }
            if( i2c.read(read_reg, readData, 1)  != 0 ){
                ser.printf("I failed to listen to the Color sensor.\n\r");       
            }
        
            ser.printf("Handshake value %x\r\n", readData[0] );
            if( readData[0] != 0xAB ){
                ser.printf("Who the heck are you?\n\r");
            }
        }
        thread_sleep_for(1000);
        if(readData[0]==171){
            i=1;
        }
    }
}

//part 2
/*
This funciont obtains the RGBC values from the sensor and transforms them into uint16_t values
ALSO CURRENTLY DOES NOT WORK AND ONLY RETURNS 0S FOR EACH VALUE10/19/21 6:54PM
*/
void obtain(void){
    while(true){

            char readData[] ={0, 0};


            const char CDATAL[] = {0x94};
           // const char CDATAH[] = {0x95};
            const char RDATAL[] = {0x96};
            //const char RDATAH[] = {0x97};
            const char GDATAL[] = {0x98};
           // const char GDATAH[] = {0x99};
            const char BDATAL[] = {0x9A};
            //const char BDATAH[] = {0x9B};

            const int read_reg = (uint8_t)0x73;  //correct
            const int write_reg = (uint8_t)0x72; //correct

            char CTRL1[]= {0x8F, 0x43};
            char ENABLE[]= {0x80, 0x13};
            char STATUS[]= {0x93, 0x11};

            int a=1;


    //TURINGING ON THE CTRL1 REGISTER

            a = i2c.write(write_reg, CTRL1, 2, true);
            if(  a != 0 ){
                ser.printf("I failed to talk at the CTRL 1. (Returned: %d)\n\r", a);           
            } 
    //TURINGING ON THE ENABLE REGISTER

            a = i2c.write(write_reg, ENABLE, 2, true);
            if(  a != 0 ){
                ser.printf("I failed to talk at the ENABLE. (Returned: %d)\n\r", a);           
            }

            a = i2c.write(write_reg, STATUS, 2, true);
            if(  a != 0 ){
                ser.printf("I failed to talk at the STATUS. (Returned: %d)\n\r", a);           
            }

    //red
    //red latches to the RDATAH register automatically making the 16 bit number
            a = i2c.write(write_reg, RDATAL, 1, true);
            if(  a != 0 ){
                ser.printf("I failed to talk at the Red low sensor. (Returned: %d)\n\r", a);           
            }
            if( i2c.read(read_reg, readData, 1)  != 0 ){
                ser.printf("I failed to listen to the Red low sensor.\n\r");       
            }
            RedLowRead=(uint16_t)readData[0];
   
            RedOverall=RedLowRead;
//green
//green latches to the GDATAH register automatically making the 16 bit number
            a = i2c.write(write_reg, GDATAL, 1, true);
            if(  a != 0 ){
                ser.printf("I failed to talk at the Green low sensor. (Returned: %d)\n\r", a);           
            }
            if( i2c.read(read_reg, readData, 1)  != 0 ){
                ser.printf("I failed to listen to the Green low sensor.\n\r");       
            }
            GreenLowRead=(uint16_t)readData[0];
    
            GreenOverall=(GreenLowRead);
    //serial.printf("Green overall value %d\r\n", GreenOverall);

//blue
//blue latches to the BDATAH register automatically making the 16 bit number
            a = i2c.write(write_reg,  BDATAL, 1, true);
            if(  a != 0 ){
                ser.printf("I failed to talk at the Blue low sensor. (Returned: %d)\n\r", a);           
            }
            if( i2c.read(read_reg, readData, 1)  != 0 ){
                ser.printf("I failed to listen to the Blue low sensor.\n\r");       
            }
            BlueLowRead=(uint16_t)readData[0];
    
            BlueOverall=(BlueLowRead);

            thread_sleep_for(100);         //sleep for 100 ms 
        //}
    }
}



//this provides the rough calibration for the colors based on googled RGB values
void calibrate(void){ 
    char printed_color=' ';
    while(true){
           // resource_mutex4.lock(); 
            if((RedOverall>0)&&(GreenOverall<25)&&(BlueOverall<40)){ // home and school lighting
                if(printed_color=='r'){
                }
                else{
                    ser.printf("The color is RED!\r\n");
                    printed_color='r';
                }
            }
            else if((RedOverall>150)&&(GreenOverall>150)&&(BlueOverall<100)){ //home and school lighting
                if(printed_color=='y'){
                }
                else{
                    ser.printf("The color is YELLOW!\r\n");
                    printed_color='y';
                }            }
            else if((RedOverall>140)&&(GreenOverall>50)&&(BlueOverall<50)){ //home and school
                if(printed_color=='o'){
                }
                else{
                    ser.printf("The color is ORANGE!\r\n");
                    printed_color='o';
                }            }
            else if((RedOverall<80)&&(GreenOverall>150)&&(BlueOverall<90)){
                if(printed_color=='g'){
                }
                else{
                    ser.printf("The color is GREEN!\r\n");
                    printed_color='g';
                }            }
            else if((RedOverall<10)&&(GreenOverall<50)&&(BlueOverall>120)){
                if(printed_color=='b'){
                }
                else{
                    ser.printf("The color is BLUE!\r\n");
                    printed_color='b';
                }            }
            else if((RedOverall>100)&&(GreenOverall<60)&&(BlueOverall>140)){
                if(printed_color=='p'){
                }
                else{
                    ser.printf("The color is PINK!\r\n");
                    printed_color='p';
                }            }
            else{   //it returns black because it reads nothing :,)
                if(printed_color==' '){
                }
                else{
                    printed_color=' ';
                }            
            }
            //resource_mutex4.unlock();
       
        thread_sleep_for(100);

    }

}

//this thread measures the uncalibrated temp
void measure_temp(){
    ser.printf("start read temp\r\n");

    resp = i2c.write(writeaddr, (const char *) whoamiaddr, 1, true);
    if( resp != 0 ) { ser.printf("I failed to talk at the temp sensor. (Returned: %d)\n\r", resp); }
    if (i2c.read(readaddr, readData, 1) != 0 ) { ser.printf("I failed to listen to the temp sensor.\n\r"); }
    thread_sleep_for(1); 

    ser.printf("Who Am I? %d\n", readData[0]);
    if( readData[0] != 0xBC ){ ser.printf("Who are are you?\n\r"); }

    readData[0] = 0x20; // Control Reg 1
    readData[1] = 0x84; // Turn on our temp sensor, and ensure that we read low to high on our values.
    resp = i2c.write(readaddr, readData, 2);   
    while(true) {
        readData[0] = 0x21; // Control Reg 2
        readData[1] = 0x1; // Signal a one shot temp reading.
        resp = i2c.write(readaddr, readData, 2);

        
        subaddr[0] = 0x2A; // LSB Temperature
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuf[0] = ((uint8_t)readData[0]);
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 1. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 1.\r\n"); }
        else { ser.printf("temp sensor 1: %d\r\n", databuf[0]); }
        readData[0] = 0;
        thread_sleep_for(1); 

        subaddr[0] = 0x2B; // MSB Temperature
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuf[1] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 2. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 2.\r\n"); }
        else { ser.printf("temp sensor 2: %d\r\n", databuf[1]); }
        readData[0] = 0;
        thread_sleep_for(1); 
        
        TOUT = databuf[0] | (databuf[1]<<8);
           
        ser.printf("Uncalibrated temperature: %d\r\n",TOUT);

        maskT0 = 00000011;
        maskT1 = 00001100;

        subaddr[0] = 0x35; // T1/T0 MSB Temperature
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuf[2] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 3. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 3.\r\n"); }
        else { ser.printf("temp sensor 3: %d\r\n", databuf[2]); }
        readData[0] = 0;
        thread_sleep_for(1); 

        maskedT0MSB = (databuf[2]) & (maskT0);
        maskedT1MSB = (databuf[2]) & (maskT1);

        subaddr[0] = 0x32; // T0_degC_x8
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuf[3] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 4. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 4.\r\n"); }
        else { ser.printf("temp sensor 4: %d\r\n", databuf[3]); }
        readData[0] = 0;
        thread_sleep_for(1); 

        T0_degC_x8 = (databuf[3]) | (maskedT0MSB << 8);
        T0_degC = T0_degC_x8/8;

        subaddr[0] = 0x33; // T1_degC_x8
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuf[4] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 5. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 5.\r\n"); }
        else { ser.printf("temp sensor 5: %d\r\n", databuf[4]); }
        readData[0] = 0;
        thread_sleep_for(1); 

        T1_degC_x8 = (databuf[4]) | (maskedT1MSB << 8);
        T1_degC = T1_degC_x8/8;

        subaddr[0] = 0x3C; // T0_OUT_3C
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuf[5] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 6. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 6.\r\n"); }
        else { ser.printf("temp sensor 6: %d\r\n", databuf[5]); }
        readData[0] = 0;
        thread_sleep_for(1); 

        subaddr[0] = 0x3D; // T0_OUT_3D
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuf[6] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 7. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 7.\r\n"); }
        else { ser.printf("temp sensor 7: %d\r\n", databuf[6]); }
        readData[0] = 0;
        thread_sleep_for(1); 

        T0_OUT = (databuf[5]) | (databuf[6] << 8);

        subaddr[0] = 0x3E; // T1_OUT_3E
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuf[7] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 8. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 8.\r\n"); }
        else { ser.printf("temp sensor 8: %d\r\n", databuf[7]); }
        readData[0] = 0;
        thread_sleep_for(1); 
        

        subaddr[0] = 0x3F; // T1_OUT_3F
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuf[8] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 9. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 9.\r\n"); }
        else { ser.printf("temp sensor 9: %d\r\n", databuf[8]); }
        readData[0] = 0;
        thread_sleep_for(1); 

        T1_OUT = (databuf[7]) | (databuf[8] << 8);

        slope = ((T1_degC) - (T0_degC))/((T1_OUT) - (T0_OUT));

        yIntercept = (-1*slope*T0_OUT) - T0_degC;

        Y = (slope*TOUT) + yIntercept;

        ser.printf("Calibrated temperature: %d\n\r", Y);
        thread_sleep_for(1500);
    }
}



float buffer[44100][1]; 
int i=0;
uint32_t mic_mode_mask =0x03;
uint32_t mic_PDM_CLK_CTRL_mask =0x8400000;
uint32_t Max_Gain= 0x50;
uint32_t mic_clk_mask= 0x1A;
uint32_t mic_output_mask= 0x7FFFFFD9;
uint32_t easy_DMA_mask= 0x8004000;
uint32_t easy_DMA_mem_mask= 0xAC44;
//uint32_t Ram8Power_mask= 0x800100;
void AudioRec(){

        setbit(mic_enable, 1); 
        setbits(mic_mode, mic_mode_mask); 
        setbits(mic_PDM_CLK_CTRL, mic_PDM_CLK_CTRL_mask);
        setbits(GAINL, Max_Gain);
        setbits(GAINR, Max_Gain);
        clearbit(PDM_Ratio, 0); 
        setbits(mic_clock, mic_clk_mask);
        setbits(mic_output, mic_output_mask);
        setbits(easy_DMA, easy_DMA_mask);
        setbits(easy_DMA_mem, easy_DMA_mem_mask);
        setbit(mic_start, 0);
        
        ser.printf("**********************\r\n");
       // const uint64_t DMABus[6] = {0x9400020008};
        char readData[] ={0, 0};
        while(true){
            if(i<16){
                buffer[i][0]= *easy_DMA;
                
                /*int a = i2c.write(write_reg, DMABus, 1, true);
                if(  a != 0 ){
                    ser.printf("I failed to talk at the EasyDMA. (Returned: %d)\n\r", a);           
                }
                if( i2c.read(read_reg, readData, 1)  != 0 ){
                    ser.printf("I failed to listen to the EasyDMA.\n\r");       
                }
                */
                ser.printf("Buffer: %d\r\n", &buffer[i][0]);

                i++;
                thread_sleep_for(1);
                
            }
            else{
                //clearbit(mic_enable, 1); 
                i=0;
                clearbit(mic_start, 0);
                setbit(mic_stop, 0);

            }
        }
} 

// main() runs in its own thread in the OS
int main()
{
    //mic_on.write(1);
    //vdd.write(1);
    //thread3.start(callback(AudioRec));
    /*thread.start(callback(colorimetry));
    thread_sleep_for(10000);
    thread2.start(callback(obtain));
    thread5.start(calibrate);*/
    DigitalOut i2cbuspull(P1_0);
    i2cbuspull.write(1);
    DigitalOut vcc(P0_22);
    vcc.write(1);
    thread_sleep_for(100);
    tempthread.start(callback(measure_temp));    
    
    //****************************************
    //I AM AWARE THAT TO GET THIS IS HOW YOU GET IT TO RUN EVERY 100MS IT JUST DOESNT WORK
   // flipper.attach(&obtain, 1);    //this should allow aobtain to run every 100ms but right now its not working nicely
    //****************************************

    /*DigitalOut vcc(P0_17); // Supply power to all of the sensors (VCC)
    vcc.write(1);
    DigitalOut vcc2(P0_25); // Supply power to all of the sensors (VCC)
    vcc2.write(1);
    DigitalOut vcc3(P0_26); // Supply power to all of the sensors (VCC)
    vcc3.write(1);
*/

    while (true) {
        thread_sleep_for(10000);
    }
}



