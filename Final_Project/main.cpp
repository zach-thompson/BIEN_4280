#include "mbed.h"
#include <USBSerial.h>
#include "Audio_File.hpp"
#include <iostream>
#include <cmath>
#include "zthompson_binaryutils.hpp"
#include <Mail.h>
#include <string> 
#include <cstdint>

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

PwmOut led(p6);

//create the mailbox
uint8_t databuf[2];

Mail<int, 50> mail_box;

Thread thread;
Thread thread2;
Thread thread3;
Thread thread4;
Thread thread5;
Thread tempthread;
// resource_mutex1;
//Mutex resource_mutex2;
Mutex resource_mutex3;
Mutex resource_mutex4;


I2C i2c(I2C_SDA1, I2C_SCL1);
uint8_t databuftemp[9];
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
float slope = 0;
int16_t yIntercept = 0;
int16_t Y = 0;
int16_t F = 0;
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
    char handshake[] = {0x92};  // this is the ID register found on page 25 and is correct
    const int read_reg = (uint8_t)0x73;  //correct
    const int write_reg = (uint8_t)0x72; //correct

    int resp=1;
    char readData[] ={10, 10};
    int i=0;

    DigitalOut i2cbuspull(P1_0); // Pull up i2C. resistor.
    i2cbuspull.write(1);
    
    thread_sleep_for(100);
    DigitalOut vcc(P0_22); // Supply power to all of the sensors (VCC)
    vcc.write(1);
    while(true){
        if(i==0){
            //readData[0] = 0x92;
            resp = i2c.write(write_reg, (const char *) handshake, 1, true);
            if(  resp != 0 ){
             //   ser.printf("I failed to talk at the Color sensor. (Returned: %d)\n\r", resp);           
            }
            if( i2c.read(read_reg, readData, 1)  != 0 ){
            //    ser.printf("I failed to listen to the Color sensor.\n\r");       
            }
        
           // ser.printf("Handshake value %x\r\n", readData[0] );
            if( readData[0] != 0xAB ){
             //   ser.printf("Who the heck are you?\n\r");
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
             //   ser.printf("I failed to talk at the CTRL 1. (Returned: %d)\n\r", a);           
            } 
    //TURINGING ON THE ENABLE REGISTER

            a = i2c.write(write_reg, ENABLE, 2, true);
            if(  a != 0 ){
             //   ser.printf("I failed to talk at the ENABLE. (Returned: %d)\n\r", a);           
            }

            a = i2c.write(write_reg, STATUS, 2, true);
            if(  a != 0 ){
             //   ser.printf("I failed to talk at the STATUS. (Returned: %d)\n\r", a);           
            }

    //red
    //red latches to the RDATAH register automatically making the 16 bit number
            a = i2c.write(write_reg, RDATAL, 1, true);
            if(  a != 0 ){
              //  ser.printf("I failed to talk at the Red low sensor. (Returned: %d)\n\r", a);           
            }
            if( i2c.read(read_reg, readData, 1)  != 0 ){
              //  ser.printf("I failed to listen to the Red low sensor.\n\r");       
            }
            RedLowRead=(uint16_t)readData[0];
   
            RedOverall=RedLowRead;
//green
//green latches to the GDATAH register automatically making the 16 bit number
            a = i2c.write(write_reg, GDATAL, 1, true);
            if(  a != 0 ){
              //  ser.printf("I failed to talk at the Green low sensor. (Returned: %d)\n\r", a);           
            }
            if( i2c.read(read_reg, readData, 1)  != 0 ){
               // ser.printf("I failed to listen to the Green low sensor.\n\r");       
            }
            GreenLowRead=(uint16_t)readData[0];
    
            GreenOverall=(GreenLowRead);
    //serial.printf("Green overall value %d\r\n", GreenOverall);

//blue
//blue latches to the BDATAH register automatically making the 16 bit number
            a = i2c.write(write_reg,  BDATAL, 1, true);
            if(  a != 0 ){
             //   ser.printf("I failed to talk at the Blue low sensor. (Returned: %d)\n\r", a);           
            }
            if( i2c.read(read_reg, readData, 1)  != 0 ){
             //   ser.printf("I failed to listen to the Blue low sensor.\n\r");       
            }
            BlueLowRead=(uint16_t)readData[0];
    
            BlueOverall=(BlueLowRead);

            thread_sleep_for(100);         //sleep for 100 ms 
        //}
    }
}


string colorbuf= " ";
//this provides the rough calibration for the colors based on googled RGB values
void calibrate(void){ 
    char printed_color=' ';
    while(true){
           // resource_mutex4.lock(); 
            if((RedOverall>0)&&(GreenOverall<25)&&(BlueOverall<40)){ // home and school lighting
                if(printed_color=='r'){
                }
                else{
                //    ser.printf("The color is RED!\r\n");
                    printed_color='r';
                    colorbuf="Red";
                }
            }
            else if((RedOverall>150)&&(GreenOverall>150)&&(BlueOverall<100)){ //home and school lighting
                if(printed_color=='y'){
                }
                else{
                   // ser.printf("The color is YELLOW!\r\n");
                    printed_color='y';
                    colorbuf="Yellow";

                }            }
            else if((RedOverall>140)&&(GreenOverall>50)&&(BlueOverall<50)){ //home and school
                if(printed_color=='o'){
                }
                else{
                    //ser.printf("The color is ORANGE!\r\n");
                    printed_color='o';
                    colorbuf="Orange";

                }            }
            else if((RedOverall<80)&&(GreenOverall>150)&&(BlueOverall<90)){
                if(printed_color=='g'){
                }
                else{
                    //ser.printf("The color is GREEN!\r\n");
                    printed_color='g';
                    colorbuf="Green";

                }            }
            else if((RedOverall<10)&&(GreenOverall<50)&&(BlueOverall>120)){
                if(printed_color=='b'){
                }
                else{
                    //ser.printf("The color is BLUE!\r\n");
                    printed_color='b';
                    colorbuf="Blue";

                }            }
            else if((RedOverall>100)&&(GreenOverall<60)&&(BlueOverall>140)){
                if(printed_color=='p'){
                }
                else{
                   // ser.printf("The color is PINK!\r\n");
                    printed_color='p';
                    colorbuf="Pink";

                }            }
            else{   //it returns black because it reads nothing :,)
                if(printed_color==' '){
                }
                else{
                    printed_color=' ';
                    colorbuf=" ";

                }            
            }
            //resource_mutex4.unlock();
       
        thread_sleep_for(100);

    }

}
 

//this thread measures the uncalibrated temp
//this thread measures the uncalibrated temp
void measure_temp(){
    
    //ser.printf("start read temp\r\n");

    resp = i2c.write(writeaddr, (const char *) whoamiaddr, 1, true);
    if( resp != 0 ) { ser.printf("I failed to talk at the temp sensor. (Returned: %d)\n\r", resp); }
    if (i2c.read(readaddr, readData, 1) != 0 ) { ser.printf("I failed to listen to the temp sensor.\n\r"); }
    thread_sleep_for(1); 

  //  ser.printf("Who Am I? %d\n", readData[0]);
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
        databuftemp[0] = ((uint8_t)readData[0]);
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 1. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 1.\r\n"); }
        else { 
            //ser.printf("temp sensor 1: %d\r\n", databuftemp[0]);
             }
        readData[0] = 0;
        thread_sleep_for(1); 

        subaddr[0] = 0x2B; // MSB Temperature
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuftemp[1] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 2. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 2.\r\n"); }
        else { 
            //ser.printf("temp sensor 2: %d\r\n", databuftemp[1]);
             }
        readData[0] = 0;
        thread_sleep_for(1); 
        
        TOUT = databuftemp[0] | (databuftemp[1]<<8);
           
        //ser.printf("Uncalibrated temperature: %d\r\n",TOUT);

        maskT0 = 00000011;
        maskT1 = 00001100;

        subaddr[0] = 0x35; // T1/T0 MSB Temperature
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuftemp[2] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 3. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 3.\r\n"); }
        else { 
            //ser.printf("temp sensor 3: %d\r\n", databuftemp[2]);
             }
        readData[0] = 0;
        thread_sleep_for(1); 

        maskedT0MSB = (databuftemp[2]) & (maskT0);
        maskedT1MSB = (databuftemp[2]) & (maskT1);

        subaddr[0] = 0x32; // T0_degC_x8
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuftemp[3] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 4. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 4.\r\n"); }
        else { 
            //ser.printf("temp sensor 4: %d\r\n", databuftemp[3]);
             }
        readData[0] = 0;
        thread_sleep_for(1); 

        T0_degC_x8 = (databuftemp[3]) | (maskedT0MSB << 8);
        T0_degC = T0_degC_x8/8;

        subaddr[0] = 0x33; // T1_degC_x8
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuftemp[4] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 5. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 5.\r\n"); }
        else { 
            //ser.printf("temp sensor 5: %d\r\n", databuftemp[4]);
             }
        readData[0] = 0;
        thread_sleep_for(1); 

        T1_degC_x8 = (databuftemp[4]) | (maskedT1MSB << 6);
        T1_degC = T1_degC_x8/8;

        subaddr[0] = 0x3C; // T0_OUT_3C
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuftemp[5] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 6. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 6.\r\n"); }
        else { 
            //ser.printf("temp sensor 6: %d\r\n", databuftemp[5]); 
            }
        readData[0] = 0;
        thread_sleep_for(1); 

        subaddr[0] = 0x3D; // T0_OUT_3D
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuftemp[6] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 7. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 7.\r\n"); }
        else {
            // ser.printf("temp sensor 7: %d\r\n", databuftemp[6]); 
            }
        readData[0] = 0;
        thread_sleep_for(1); 

        T0_OUT = (databuftemp[5]) | (databuftemp[6] << 8);

        subaddr[0] = 0x3E; // T1_OUT_3E
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuftemp[7] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 8. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 8.\r\n"); }
        else {
            // ser.printf("temp sensor 8: %d\r\n", databuftemp[7]); 
             }
        readData[0] = 0;
        thread_sleep_for(1); 
        

        subaddr[0] = 0x3F; // T1_OUT_3F
        a = i2c.write(writeaddr, (const char *) subaddr, 1, true);
        b = i2c.read(readaddr, readData, 1);
        databuftemp[8] = readData[0];
        if ( a != 0 ) { ser.printf("I failed to talk at the temp sensor 9. (Returned: %d)\r\n", a); }
        if (b != 0 ) { ser.printf("I failed to listen to the temp sensor 9.\r\n"); }
        else { 
           // ser.printf("temp sensor 9: %d\r\n", databuftemp[8]); 
            }
        readData[0] = 0;
        thread_sleep_for(1); 

        T1_OUT = (databuftemp[7]) | (databuftemp[8] << 8);

    //***********************************************************************************************************************************
    //ATTENTION
    //Inorder for this to work you must configure mbed os->platform->mbed lib json to "minimal-printf-enable-floating-point": set to true
    //ATTENTION
    //***********************************************************************************************************************************

        slope = (((float)T1_degC) - ((float)T0_degC))/(((float)T1_OUT) - ((float)T0_OUT));
        yIntercept= T0_degC-(slope*T0_OUT);
        Y = (slope*(float)TOUT) + yIntercept;
        Y= Y/8.0;
        F= (float)Y*(9/5)+32;

       // ser.printf("Calibrated temperature in C: %d\n\r", Y);
        //ser.printf("Calibrated temperature in F: %d\n\r", F);

        thread_sleep_for(1500);
    }
}
char *audiobuf = new char[1];
 //  pc.printf("Hello There",11);
   
   
void audio_reaction(){
  
   led = 1;
   ser.read(audiobuf, sizeof(audiobuf));
    ser.write(audiobuf, sizeof(audiobuf));
 
    ser.printf("I am recieving something!");;
   
    while(true){
    //Uncal temp
        if(*audiobuf=='1'){
            ser.printf("Uncalibrated temp: %d\r\n", TOUT);
            //*audiobuf=" ";
        }
    //color
        else if(*audiobuf=='2'){
            ser.printf("Color: %s\r\n", colorbuf.c_str());
            //*audiobuf=" ";
             led = 0;

        }
    //calibrated Tem Degree C
        else if(*audiobuf=='3'){
            ser.printf("Calibrated temp (C): %d\r\n", Y);
            //*audiobuf=" ";

        }
    //calibrated Tem Degree F
        else if(*audiobuf=='4'){
            ser.printf("Calibrated temp (F): %d\r\n", F);
            //*audiobuf=" ";
        }
    //calibrated RGB
        else if(*audiobuf=='5'){
            ser.printf("Red: %d\r\n", RedOverall);
            ser.printf("Green: %d\r\n", GreenOverall);
            ser.printf("Blue: %d\r\n", BlueOverall);
            //*audiobuf=" ";
        }
        else{
        }
    thread_sleep_for(10);

    }

}


uint32_t buffer[50000]={0}; 
int16_t buffer2[2]={0}; 

uint32_t mic_mode_mask =4;
uint32_t mic_PDM_CLK_CTRL_mask =0x8400000;
uint32_t Max_Gain_set= 0x51;
uint32_t Min_Gain_set= 0x01;

uint32_t Max_Gain_clr= 0xFFFFFFFF;
uint32_t mic_clk_mask_clear= 0x8000003F;
uint32_t mic_clk_mask_set= 0x1B;
uint32_t mic_output_mask_clear= 0x8000003F;
uint32_t mic_output_mask_set= 0x1A;
//uint32_t easy_DMA_mask_clear= 0xFFFFFFFF;
uint32_t easy_DMA_mask_set= 0x20010001;
uint32_t easy_DMA_mem_mask= 0xAC45;
uint32_t first15= 0xFFFF;
uint32_t second15= 0xFFFF0000;

//uint32_t Ram8Power_mask= 0x800100;
void AudioRec(){


       // ser.printf("mic mord pre: %d\r\n", *mic_mode);
       // setbits(mic_mode, mic_mode_mask); 
        *mic_mode=1;
       // ser.printf("mic enable post: %d\r\n", *mic_mode);

     
        *easy_DMA= (uint32_t) buffer;
        *easy_DMA_mem= 50000;

        *mic_clock=26;
        *mic_output=25;
        *mic_enable=1;
       // ser.printf("easy_DMA post: %d\r\n", *easy_DMA);
       // ser.printf("easy_DMA_mem post: %d\r\n", *easy_DMA_mem);

        /*ser.printf("mic enable pre: %d\r\n", *mic_enable);
        setbit(mic_enable, 0); 
        ser.printf("mic enable post: %d\r\n", *mic_enable);*/
        //ser.printf("Speak in 3\r\n");
        thread_sleep_for(1000);
        //ser.printf("2\r\n");
        thread_sleep_for(1000);
        //ser.printf("1...\r\n");
        thread_sleep_for(1000);
       // ser.printf("Start!\r\n");

        *mic_start=1;
        thread_sleep_for(1500);
        *mic_stop=1; 
        //ser.printf("Stop!\r\n");
    
        //ser.printf("**********************\r\n");
       // const uint64_t DMABus[6] = {0x9400020008};
       // while(true){
                    for(int j=0; j<(*easy_DMA_mem/2);j++){

                        buffer2[0]= (int16_t) (buffer[j] & first15);
                        buffer2[1]= (int16_t)((buffer[j] & second15)>>16);
                        ser.printf("%d\r\n", buffer2[0]);
                        ser.printf("%d\r\n", buffer2[1]);

                    }
            

} 



/*void readUSBserial(){
    uint64_t buf[128];
    while(true){
        //ser.printf("I am Alive");
        ser.scanf("%s", buf);
        ser.printf("%s", buf);
        thread_sleep_for(1);
    }
}   */ 
    
    // main() runs in its own thread in the OS
int main()
{

    DigitalOut i2cbuspull(P1_0);
    i2cbuspull.write(1);
    DigitalOut vcc(P0_22);
    vcc.write(1);
    thread_sleep_for(100);  
    thread2.start(callback(colorimetry));
    thread5.start(callback(obtain));
    thread4.start(callback(calibrate));
   tempthread.start(callback(measure_temp)); 

    mic_on.write(1);
    vdd.write(1);
    thread3.start(callback(AudioRec));
    //thread_sleep_for(10000);

    //tempthread.start(callback(readUSBserial)); 

   thread.start(callback(audio_reaction));
  

    while (true) {
        thread_sleep_for(10000);
    }
}