/* Archivo: Fotoresistecia.c
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
#include <pic16f887.h>
#include "LCD8bits.h"
#include "ADC.h"
#include "I2C.h"
#include <xc.h>
//*****************************************************************************
// Definición de variables
unsigned int RADC = 0; //Para resolución del ADC
float LDR = 0; //Valor del LDR con decimales
unsigned long resistencia = 0; //Valor del LDR entero
uint16_t Resistor;
unsigned char LCD_Buffer[15]; //Para la LCD
uint8_t z;
uint8_t Alarma;
//*****************************************************************************
#define _XTAL_FREQ 8000000
#define RE0 PORTEbits.RE0
#define RE1 PORTEbits.RE1
/*#define RS RE0
#define EN RE1
#define D4 RD4
#define D5 RB5
#define D6 RD6
#define D7 RD7*/

//*****************************************************************************
// Definición de funciones para que se puedan colocar después del main de lo 
// contrario hay que colocarlos todas las funciones antes del main
//*****************************************************************************
void setup(void); //Configuración de puertos
int map1 (uint16_t val, uint16_t minx, uint16_t maxx, int miny, int maxy);

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
            PORTD = SSPBUF;             // Guardar en el PORTD el valor del buffer de recepción
            __delay_us(250);
            
        }else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){ //Condición para escibirle al maestro
            z = SSPBUF;
            BF = 0;
            SSPBUF = Resistor;
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
    setup();
    //Lcd_Init8(); //Inicializamos la LCD
    //Lcd_Clear8(); //Limpiamos la LCD
    
    while(1){
        Resistor = map1(resistencia, 503, 19999, 0, 255);
        if (Resistor >= 255){
            Resistor = 255;
        }
        else {
            Resistor = Resistor;
        }
        
        if (Resistor <= 15){
            Alarma = 1;
        }
        
        if (Alarma == 1){
            RE0 = ~RE0;
            RE1 = ~RE1;
        }
        
        if (ADCON0bits.GO == 0) { // Si la lectura del ADC se desactiva

            __delay_us(1000); //Este es el tiempo que se dará cada vez que se desactiva la lectura
            ADCON0bits.GO = 1; //Activamos la lectura del ADC
        }
        
       __delay_ms(500);
        
        ADCON0bits.GO = 1; //Iniciamos la lectura del ADC
        while(ADCON0bits.GO == 1); //Esperamos a que termine la lectura del ADC
        RADC = ADRESH; //Le cargamos al RADC los primeros 8 bits del ADC
        RADC = RADC<<8; //Corremos 8 bits hacia la izquierda los primeros bits
        RADC |= ADRESL; //Hacemos un or entre los 8 bits más significativos del ADC y el RADC 
                        //para así obtener los 16 bits totales del ADC
        LDR = (48.8758*RADC)/(5.0-(0.00488758*RADC)); //Valor flotante del LDR
        resistencia = LDR; //Valor entero del LDR
        
        /*Lcd_Clear8();
        sprintf(LCD_Buffer, "%d", Resistor);
        Lcd_Set_Cursor8(1,1); //Definimos en donde queremos ver ADC en la LCD
        Lcd_Write_String8("LDR ?");

        Lcd_Set_Cursor8(2,1); //Definimos en donde queremos ver el ADC en la LCD
        Lcd_Write_String8(LCD_Buffer);
        __delay_ms(100);*/

    }
    return;
}

void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    
    adc_init(0);
    //definir analógico
    ANSELbits.ANS0 = 1; //Habilitamos para el canal 0
    
    //Configuramos los puertos como entradas/salidas
    TRISA = 0b00000001;
    TRISD = 0;
    TRISE = 0;
    
    //Limpiamos los puertos
    PORTA = 0;
    PORTD = 0;
    PORTE = 0b0001;
    
    OSCCONbits.IRCF = 0b111; //8 MHz
    OSCCONbits.SCS = 1;
    I2C_Slave_Init(0x60); // Inicializar Comunicación I2C   
}

int map1 (uint16_t val, uint16_t minx, uint16_t maxx, int miny, int maxy){ //Función para mapear el valor del potenciometro a cualquier otro valor.
    int vo;
    vo = (long)(val-minx)*(maxy-miny)/(maxx-minx)+ miny; 
    return vo;
}

