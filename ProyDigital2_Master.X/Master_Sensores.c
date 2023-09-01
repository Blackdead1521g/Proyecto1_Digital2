/* Archivo: Master.c
 * Dispositivo: PIC16F887
 * Autor: Kevin Alarcón
 * Compilador: XC8(v2.40), MPLABX V6.05
 * 
 * 
 * Programa: Realizar un reloj que permita modificar su hora y fecha exacta
 * Hardware: Botones, reloj RTC y una LCD
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
#include "I2C.h"
#include "LCD8bits.h"
#include "USART.h"
#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//*****************************************************************************
// Definición de variables
uint8_t datos = 0;
uint8_t data = 0;
uint8_t BANDERA = 0;
uint8_t Bandera = 0;
uint16_t Ultrasonico;
uint16_t Fotoresistencia; //Valor del LDR entero
uint16_t Infrarrojo;
uint8_t contador = 0;
/*static char Ultra[] = "000";
static char Foto[] = "000";
static char Infra[] = "000";*/
char Total[10] = "";
char Ultra[4];
char Foto[4];
char Infra[4];
char ultra[4];
char foto[4];
char infra[4];
//*****************************************************************************
#define _XTAL_FREQ 8000000
#define RS RE0
#define EN RE1
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7


//*****************************************************************************
// Definición de funciones para que se puedan colocar después del main de lo 
// contrario hay que colocarlos todas las funciones antes del main
//*****************************************************************************
void setup(void);


//*****************************************************************************
// Código de Interrupción 
//*****************************************************************************
void __interrupt() isr(void){
    if(PIR1bits.RCIF){
        data = RCREG;
        //string("HOLA");
    }
    if(INTCONbits.RBIF){
        if(PORTBbits.RB0 == 0){//Incrementar minutos
            while(PORTBbits.RB0 == 0);//Incrementar minutos
            BANDERA = 255;
            Bandera = 255;
        }
        INTCONbits.RBIF = 0;
    }
    return;
}
//*****************************************************************************
// Main
//*****************************************************************************
void main(void) {
    setup();
    Lcd_Init8(); //Inicializamos la LCD
    Lcd_Clear8(); //Limpiamos la LCD
    unsigned int a;
    while(1){
        //if(PORTBbits.RB0 == 0)
        //BANDERA = 0;
        
        if(datos == 0){
            //strcat(foto, "f");
            UART_write_char(strcat(foto, "f"));
            //__delay_ms(100);
            datos = 1;
        }
        else if (datos == 1){
            //strcat(infra, "i");
            UART_write_char(strcat(infra, "i"));
            //__delay_ms(100);
            datos = 2;
        }
        else if (datos == 2){
            //strcat(ultra, "u");
            UART_write_char(strcat(Ultra, "u"));
            //__delay_ms(100);
            datos = 0;
        }
        
        
        
        I2C_Master_Start(); //Le escibimos el contador al Esclavo1
        I2C_Master_Write(0x60);
        I2C_Master_Write(BANDERA);
        I2C_Master_Stop();
        __delay_ms(300);
       
        I2C_Master_Start(); //Leemos el ADC del Esclavo1
        I2C_Master_Write(0x61);
        Fotoresistencia = I2C_Master_Read(0);
        /*UART_write_char(foto);
        __delay_ms(100);*/
        I2C_Master_Stop();
        __delay_ms(300);
        
        I2C_Master_Start(); //Le escibimos el contador al Esclavo1
        I2C_Master_Write(0x70);
        I2C_Master_Write(Bandera);
        I2C_Master_Stop();
        __delay_ms(300);
       
        I2C_Master_Start(); //Leemos el ADC del Esclavo1
        I2C_Master_Write(0x71);
        Infrarrojo = I2C_Master_Read(0);
        //UART_write_char(infra);
        //__delay_ms(100);
        I2C_Master_Stop();
        __delay_ms(300);
        
        I2C_Master_Start(); //Le escibimos el contador al Esclavo1
        I2C_Master_Write(0x50);
        I2C_Master_Write(BANDERA);
        I2C_Master_Stop();
        __delay_ms(300);
       
        I2C_Master_Start(); //Leemos el ADC del Esclavo1
        I2C_Master_Write(0x51);
        Ultrasonico = I2C_Master_Read(0);
        //UART_write_char(ultra);
        //__delay_ms(100);
        I2C_Master_Stop();
        __delay_ms(300);
        
        
        
        //Pasamos cada valor leido a un string para presentarlo en la LCD
        sprintf(ultra, "%d\r", Ultrasonico);
        sprintf(foto, "%d\r", Fotoresistencia);
        sprintf(infra, "%d\r", Infrarrojo);
        
        //Total = foto + infra + ultra;
        /*Total[3] = infra;
        Total[6] = ultra;*/
        
        sprintf(Ultra, "%d", Ultrasonico);
        sprintf(Foto, "%d", Fotoresistencia);
        sprintf(Infra, "%d", Infrarrojo);
        
        Lcd_Clear8();
        Lcd_Set_Cursor8(1,1); //Definimos en donde queremos ver ADC en la LCD
        Lcd_Write_String8("Ultra");
        
        Lcd_Set_Cursor8(1,8); //Definimos en donde queremos ver ADC en la LCD
        Lcd_Write_String8("Foto");
        
        Lcd_Set_Cursor8(1,14); //Definimos en donde queremos ver ADC en la LCD
        Lcd_Write_String8("Infra");

        Lcd_Set_Cursor8(2,1); //Definimos en donde queremos ver el ADC en la LCD
        /*Ultra[0]= Ultrasonico / 100 + '0';
        Ultra[1]= ((Ultrasonico % 100)/10) + '0';
        Ultra[2]= Ultrasonico % 10 + '0';  */       // Convertir el valor a cadena
        Lcd_Write_String8(Ultra);
        
        Lcd_Set_Cursor8(2,8); //Definimos en donde queremos ver el ADC en la LCD
        /*Foto[0]= Fotoresistencia / 100 + '0';
        Foto[1]= ((Fotoresistencia % 100)/10) + '0';
        Foto[2]= Fotoresistencia % 10 + '0';  */       // Convertir el valor a cadena
        Lcd_Write_String8(Foto);
        
        Lcd_Set_Cursor8(2,14); //Definimos en donde queremos ver el ADC en la LCD
        /*Infra[0]= Infrarrojo / 100 + '0';
        Infra[1]= ((Infrarrojo % 100)/10) + '0';
        Infra[2]= Infrarrojo % 10 + '0';  */       // Convertir el valor a cadena
        Lcd_Write_String8(Infra);
        
        __delay_ms(100);
        
        
        BANDERA = 0;
        contador++;   
        PORTA = Ultrasonico;
        
    }
    return;
}
//*****************************************************************************
// Función de Inicialización
//*****************************************************************************
void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    
    //Configuramos los puertos como entradas/salidas
    TRISA = 0;
    TRISB = 0b11111111;
    TRISC = 0b11000000;
    TRISD = 0;
    TRISE = 0;
    
    //Limpiamos los puertos
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTE = 0;
    
    OPTION_REGbits.nRBPU = 0; //Habilitamos los PULLUPS
    IOCB = 0b11111111; //Habilitamos las interrupciones al cambiar de estaso el puerto B
    
    OSCCONbits.IRCF = 0b111; //8 MHz
    OSCCONbits.SCS = 1;
    
    INTCONbits.RBIF = 0; //Apagamos la bandera del puerto B
    INTCONbits.RBIE = 1; //Habilitamos la interrupción en el puerto B
    INTCONbits.GIE = 1;         // Habilitamos interrupciones
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones PEIE
    UART_RX_config(9600); //Configuración de los registros para la comunicación serial
    UART_TX_config(9600);
    
    I2C_Master_Init(100000);        // Inicializar Comunicación I2C
    return;
}

/*void initUART(void){
    //paso 1
    SPBRG = 25; //SPBRGH:SPBRG  = [(4Mhz/9600)/64]-1 = 12 ? real 9615.38
    SPBRGH = 0; //%error = (9615.38-9600)/9600 * 100 = 0.16%
    BRGH = 1;   
    BRG16 = 0;
    
    //paso 2
    CREN = 1;
    SYNC = 0;   // TXSTAbits ? habilitar transmision & TXIF = 1
    SPEN = 1;   //RCSTAbits ? apagar bandera TX
    TXSTAbits.TXEN = 1; //permite transmitir
    
    //paso 3: habilitar los 9 bits
    RCSTAbits.RX9 = 0;
    
    //paso 4
    TXEN = 1; //TXSTAbits -> habilitar transmision & TXIF = 1
    TXIF = 0; //PIRbits -> apagar bandera TX
    //C
    
    //paso 5: interrupciones
    PIE1bits.RCIE = 1; //Habilitamos interrupción de RCIF
    PIR1bits.RCIF = 0; //Limpiamos bandera del RCIF

    
    
    //paso 6: cargar 9no bit
    //paso 7: cargar 
    //return;
}*/