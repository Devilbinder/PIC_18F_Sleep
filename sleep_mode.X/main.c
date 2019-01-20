#include <xc.h>
#include <p18f4520.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "conbits.h"
#include "uart_layer.h"
#include "global_var.h"

#define TIME_PER_OVERFLOW   (65535u) // us 65535u x 8 = 524,280 ms
#define TIME_PER_TICK       (1u) // us

void interrupt high_isr(void);
void interrupt low_priority low_isr(void);

uint8_t print_buffer[256] = {0}; // buffer to print stuff to serial


void running_leds(void){
    uint8_t run = 1;
    
    for(uint8_t i = 0; i < 8;i++){
        LATB = run;
        if(run >= 0x80){
            run = 1;
        }else{
            run *= 2;
        }
        __delay_ms(200);
    }
    
}

void main(void){

    OSCCONbits.IDLEN = 0; // 0 = deep sleep 1 = idle run
    OSCCONbits.IRCF = 0x07;
    OSCCONbits.SCS = 0x03;
    while(OSCCONbits.IOFS!=1); // 8Mhz
    
    TRISB=0;    // b port as output
    LATB=0x00; // b port low
    
    TRISCbits.RC1 = 1;
  
    uart_init(51,0,1,0);//baud 9600
    
    CCP2CONbits.CCP2M = 0x05;
    
    T3CONbits.RD16 = 0;
    T3CONbits.T3CCP2 = 0;
    T3CONbits.T3CCP1 = 1;
    T3CONbits.T3CKPS = 3;
    T3CONbits.TMR3CS = 0;
    
    IPR2bits.TMR3IP = 0;
    IPR2bits.CCP2IP = 1;
    
    PIE2bits.TMR3IE = 1;
    PIE2bits.CCP2IE = 1;

    RCONbits.IPEN = 1; 
    INTCONbits.GIEH = 1; 
    INTCONbits.GIEL = 1;// base interrupt setup
    
    
    __delay_ms(2000);
    sprintf(print_buffer,"Program start\n\r");
    uart_send_string(print_buffer); // everything works in setup
    
    uint8_t sleep_time = 0;
    T3CONbits.TMR3ON = 1;
    for(;;){
        running_leds();
        sleep_time++;
        sprintf(print_buffer,"enter sleep mode %d\n\r",sleep_time);
        uart_send_string(print_buffer); // everything works in setup
        if(sleep_time == 2){
            LATB=0x00; // b port low
            sleep_time=0;
            Sleep();
        }
    } 
}



void interrupt high_isr(void){
    INTCONbits.GIEH = 0;
    if(PIR2bits.CCP2IF == 1){
        
        PIR2bits.CCP2IF = 0; 
    }
    INTCONbits.GIEH = 1;
}

void interrupt low_priority low_isr(void){
    INTCONbits.GIEH = 0;
    if(PIR2bits.TMR3IF == 1){
        
        PIR2bits.TMR3IF = 0;
    }  
    INTCONbits.GIEH = 1;
}



