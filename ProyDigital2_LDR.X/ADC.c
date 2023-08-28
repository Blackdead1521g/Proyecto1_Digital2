/* Archivo: ADC.c
 * Dispositivo: PIC16F887
 * Autor: Kevin Alarc�n
 * Compilador: XC8(v2.40), MPLABX V6.05
 * 
 * 
 * Programa: Librer�a de configuracion del ADC
 * Hardware: NA
 * 
 * Creado: 17 de julio, 2023
 * �ltima modificaci�n: 21 de julio, 2023
 */

#include "ADC.h"

//Funci�n para elegir en que canal iniciar el ADC
void adc_init(int channel){
    if(channel <= 13){ 
        ADCON0bits.CHS = channel;
    }
    else {
        ADCON0bits.CHS = 0b0000;
    }
    
    //Configuraci�n ADC
    ADCON1bits.VCFG0 = 0; //Voltaje referenciado de 0V
    ADCON1bits.VCFG1 = 0; //Voltaje referenciado de 5V
    ADCON0bits.ADCS = 0b001; // 8*Tosc
     
    ADCON1bits.ADFM = 1; //Justificado a la derecha
    ADCON0bits.ADON = 1;//Encendemos el m�dulo del ADC
     
    //Banderas e interrupciones
    /*PIR1bits.ADIF = 0; //Apagamos la bandera del ADC
    INTCONbits.PEIE = 1; //Habilitar interrupciones perif�ricas
    PIE1bits.ADIE = 1; //Habilitar interrupciones del ADC
    INTCONbits.GIE = 1; //Habilitar interrupciones globales
    ADCON0bits.GO = 1; //Activamos la lectura del ADC*/
    ADCON0bits.CHS = 0b0000;
    return;
}

int adc_read(){
    return ADRESH;
}

void adc_change_channel(int channel){
    if(channel <= 13){ 
        ADCON0bits.CHS = channel;
    }
    else {
        ADCON0bits.CHS = 0b0000;
    }
    return;
    
}

int adc_get_channel(){
    char canal = ADCON0bits.CHS; //Le asignamos a la variable canal el n�mero de canal del ADC
    return canal;
}

int map (int val, int minx, int maxx, int miny, int maxy){ //Funci�n para mapear el valor del potenciometro a cualquier otro valor.
    int vo;
    vo = (long)(val-minx)*(maxy-miny)/(maxx-minx)+ miny; 
    return vo;
}

