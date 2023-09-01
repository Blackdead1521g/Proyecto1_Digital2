/*
 * File:   Infraredsensor.c
 * Author: Ronal
 *
 * Created on 22 de agosto de 2023, 11:54 AM
 */

#include <stdint.h>
#include <string.h>
#include <pic16f887.h>
#include <xc.h>
#include <stdio.h>
#include "PWM_SERVO1.h"
#include "I2C.h"

// Configuración del PIC16F887
// CONFIG1
#pragma config FOSC = INTRC_CLKOUT// Oscillator Selection bits (INTOSC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define _XTAL_FREQ 4000000 //definimos la frecuencia del oscilador
uint8_t z;
uint8_t Bandera;
uint8_t Sensor;
uint8_t Servo = 1;
// Definiciones de pines

// Prototipos
void servoRotate90Degrees();
void servoRotate0Degrees();

//*****************************************************************************
// Código de Interrupción 
//*****************************************************************************
void __interrupt() isr(void){
   if(PIR1bits.SSPIF == 1){ //Revisamos si ya hay datos que recibir

        SSPCONbits.CKP = 0;
       
        if ((SSPCONbits.SSPOV) || (SSPCONbits.WCOL)){
            z = SSPBUF;                 // Read the previous value to clear the buffer
            SSPCONbits.SSPOV = 0;       // Clear the overflow flag
            SSPCONbits.WCOL = 0;        // Clear the collision bit
            SSPCONbits.CKP = 1;         // Enables SCL (Clock)
        }

        if(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) { //Condición para recibir datos del maestro 
            //__delay_us(7);
            z = SSPBUF;                 // Lectura del SSBUF para limpiar el buffer y la bandera BF
            //__delay_us(2);
            PIR1bits.SSPIF = 0;         // Limpia bandera de interrupción recepción/transmisión SSP
            SSPCONbits.CKP = 1;         // Habilita entrada de pulsos de reloj SCL
            while(!SSPSTATbits.BF);     // Esperar a que la recepción se complete
            Bandera = SSPBUF;  
            PORTD = Bandera; // Guardar en el PORTD el valor del buffer de recepción
            __delay_us(250);
            
        }else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){ //Condición para escibirle al maestro
            z = SSPBUF;
            BF = 0;
            SSPBUF = Sensor;
            SSPCONbits.CKP = 1;
            __delay_us(250);
            while(SSPSTATbits.BF);
        }
       
        PIR1bits.SSPIF = 0;    
    }
}

void main() {
    ANSEL = 0;
    ANSELH = 0;
    // Configuración de pines
    TRISA = 0;
    TRISB = 0b11111111; 
    TRISC = 0;
    TRISD = 0;

    
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    
    //////Configuración PWM
    TRISCbits.TRISC1 = 1; //RC1 / CCP2 como entrada
    TRISCbits.TRISC2 = 1; //RC2 / CCP1 como entrada
     
    PR2 = 255; //Valor del PWM periodo
    
    CCP1CONbits.P1M = 0; //PWM mode una salida
    CCP1CONbits.CCP1M = 0b1100; //Modo PWM    

    CCPR1L = 0x0f; //Inicio del ciclo de trabajo
    
    CCP1CONbits.DC1B = 0; 
    CCP2CONbits.DC2B0 = 0;
    CCP2CONbits.DC2B1 = 0;
    
    PIR1bits.TMR2IF = 0; //Limpiamos la bandera del TMR2
    T2CONbits.T2CKPS = 0b111; //prescaler 1:16
    T2CONbits.TMR2ON = 1; //Encendemos el TMR2
    while(PIR1bits.TMR2IF == 0); //Esperamos a que se complete un ciclo del TMR2 (hasta que la bandera del TMR2 se encienda)
    PIR1bits.TMR2IF = 0; //Bajamos la bandera del TMR2
    TRISCbits.TRISC1 = 0; //RC1 / CCP2 como salida
    TRISCbits.TRISC2 = 0; //RC2 / CCP1 como salida
    
    // Configuración del oscilador a 8 MHz
    OSCCONbits.IRCF = 0b111;
    OSCCONbits.SCS = 1;
    
    I2C_Slave_Init(0x70); // Inicializar Comunicación I2C   
    
    // Ciclo principal
    while (1) {
        if (PORTBbits.RB0 == 0) {
            PORTA = 0xFF;
            Sensor = 0xFF;
            if(Servo == 1){
                // Si el sensor IR detecta un objeto, gira el servo 90 grados
                Servo = 0;
                //PORTCbits.RC2 = 1; 
                CCPR1L = ((255 >> 1) + 124); //Asignar a la variable potenciometro el valor del potenciometro del PORTA5
    
                __delay_ms(500); //
                //CCPR1L = 0x5E; //  90 grados
          //  __delay_ms(10); // Espera para evitar lecturas múltiples del sensor
            }
        }
        else {
            PORTA = 0;
            Sensor = 0;
            
        }
        
        if (Servo == 0){
            if (Bandera == 255){
                CCPR1L = (124);
                Bandera++;
                __delay_ms(500); //
            }
        }
        __delay_ms(500);
    }
}



void servoRotate90Degrees() {
    // Configuración del módulo CCP1 para generar PWM
 
    CCPR1L = ((255 >> 1) + 124); //Asignar a la variable potenciometro el valor del potenciometro del PORTA5
    
   __delay_ms(500); // 
    
    //CCPR1L = (124); //Asignar a la variable potenciometro el valor del potenciometro del PORTA5*/
   return;
}


