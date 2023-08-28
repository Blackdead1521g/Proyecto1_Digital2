/* Archivo: Ultrasonico.c
 * Dispositivo: PIC16F887
 * Autor: Kevin Alarcón
 * Compilador: XC8(v2.40), MPLABX V6.05
 * 
 * 
 * Programa: Realizar un contador con ADC y enviarlo al master por comunicación I2C
 * Hardware: Potenciometro.
 * 
 * Creado: 08 de agosto, 2023
 * Última modificación: 11 de agosto, 2023
 */
//*****************************************************************************
// Palabra de configuración
//*****************************************************************************
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (RCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
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

//*****************************************************************************
// Definición e importación de librerías
//*****************************************************************************
#include <stdint.h>
#include <string.h>
#include <pic16f887.h>
#include "LCD8bits.h"
#include "I2C.h"
#include <xc.h>
#include <stdio.h>
//*****************************************************************************
// Definición de variables
char LCD_Buffer[15];
uint8_t z;
uint8_t Bandera;
uint16_t Distancia;
//*****************************************************************************
#define _XTAL_FREQ 8000000
/*#define RS RE0
#define EN RE1
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7*/
#define Pin_Trig PORTAbits.RA0 //Asignamos "Pin_Trig" al RA0 (salida)
#define Pin_Echo PORTAbits.RA1 //Asignamos "Pin_Trig" al RA1 (entrada)

//*****************************************************************************
// Definición de funciones para que se puedan colocar después del main de lo 
// contrario hay que colocarlos todas las funciones antes del main
//*****************************************************************************

void setup(void); //Configuración de puertos
void Timer1_Init(void); //Inicializar el TIMER1
uint16_t Obtener_Distancia (void); //Obtener la distancia del sensor

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
            Bandera = SSPBUF;             // Guardar en el PORTD el valor del buffer de recepción
            PORTD = Bandera;
            __delay_us(250);
            
        }else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){ //Condición para escibirle al maestro
            z = SSPBUF;
            BF = 0;
            SSPBUF = Distancia;
            SSPCONbits.CKP = 1;
            __delay_us(250);
            while(SSPSTATbits.BF);
        }
       
        PIR1bits.SSPIF = 0;    
    }
}

//*****************************************************************************
// Main
//*****************************************************************************
void main(void) {
    uint8_t DC = 1;
    
    setup();
    Timer1_Init();
    //Lcd_Init8(); //Inicializamos la LCD
    //Lcd_Clear8(); //Limpiamos la LCD
    
    while(1){
        //PORTD = Bandera;
        Distancia = Obtener_Distancia(); //Cargamos el valor de la distancia del sensor en nuestra variable
        
        if(Distancia <= 10){
            if(DC == 1){
                DC = 0;
                PORTBbits.RB0 = 1;
                PORTBbits.RB1 = 0;
                __delay_ms(125); // 
            }
            else{
                PORTB = 0;
            }
        }
        
        if (DC == 0){
            if(PORTD == 255){
                PORTBbits.RB0 = 0;
                PORTBbits.RB1 = 1;
                PORTD++;
                __delay_ms(150); // 
            }
            PORTBbits.RB0 = 0;
            PORTBbits.RB1 = 0;
        }
        
        /*Lcd_Clear8();
        sprintf(LCD_Buffer, "%d", Distancia);
        Lcd_Set_Cursor8(1,1); //Definimos en donde queremos ver ADC en la LCD
        Lcd_Write_String8("LDR ?");

        Lcd_Set_Cursor8(2,1); //Definimos en donde queremos ver el ADC en la LCD
        Lcd_Write_String8(LCD_Buffer);
        __delay_ms(100);*/
        __delay_ms(500);
    }
}

void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    
    //Configuramos los puertos como entradas/salidas
    TRISA = 0b00000010;
    TRISB = 0;
    TRISD = 0;
    TRISE = 0;
    
    //Limpiamos los puertos
    PORTA = 0;
    PORTB = 0;
    PORTD = 0;
    PORTE = 0;
    
    OSCCONbits.IRCF = 0b111; //8 MHz
    OSCCONbits.SCS = 1;
    I2C_Slave_Init(0x50); // Inicializar Comunicación I2C   
    return;
}

void Timer1_Init(void){
    T1CONbits.T1CKPS = 0b00; //Prescaler 1
    T1CONbits.TMR1CS = 0; //Reloj interno (Fosc/4)
    TMR1 = 0;
    T1CONbits.TMR1ON = 0; //Temporizador de TIMER1 detenido
    return;
}

uint16_t Obtener_Distancia(void){
    
    uint16_t Duracion;
    uint16_t Distancia;
    uint16_t Timer_1;
    
    Pin_Trig = 1; //Ponemos en 1 el RA0
    __delay_us(10); //Retardo de 10 us
    Pin_Trig = 0; //Ponemos en 0 el RA0
    while(Pin_Echo == 0); //Esperamos mientras RA1 sea 0
    T1CONbits.TMR1ON = 1; //Activamos el temporizador del TIMER1
    while(Pin_Echo == 1); //Esperamos mientras RA1 sea 1
    T1CONbits.TMR1ON = 0; //Se detiene el temporizador del TIMER1
    Timer_1 = TMR1;
    Duracion = (Timer_1)/2;
    
    if (Duracion <= 23200){ //23529.4 us equivale a 400 cm(Máxima distancia)
        Distancia = Duracion/58; //Valor de distancia en cm (Fórmula aplicada para us)
    }
    else if (Duracion < 116){ //117.6 us equivale a 2 cm (Mínima distancia)
        Distancia = 0;
    }
    else {
        Distancia = 0;
    }
    Duracion = 0;  //Reiniciamos el valor de la variable duracion
    TMR1 = 0; //Reiniciamos el valor del TMR1
    
    return Distancia;
}