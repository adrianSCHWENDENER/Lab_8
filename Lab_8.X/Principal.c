/*
 * File:   Principal.c
 * Author: schwe
 *
 * Created on 17 de abril de 2023, 12:25 PM
 */

//CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT                        //Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF                                   //Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF                                  //Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF                                  //RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF                                     //Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF                                    //Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF                                  //Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF                                   //Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF                                  //Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF                                    //Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V                               //Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF                                    //Flash Program Memory Self Write Enable bits (Write protection off)

//Librerías
#include <xc.h>
#include <stdio.h>

//Definición de variables
#define _XTAL_FREQ 1000000
uint8_t indicador = 0B00110000;
const char data = 13;                                       //Valor ASCII de ENTER
char adc[2];
int esperar = 0;

//Prototipos
void setup(void);
void main(void);
void cadena(char txt[]);

//Interrupcion
void __interrupt() isr(void) {
    if (PIR1bits.ADIF){                                     //Si se inicio ADC
        sprintf(adc, "%d", ADRESH);                         //Convertir ADRESH en tipo char
    }
    if (PIR1bits.RCIF){                                     //Si se puede recibir valor de UART
        indicador = RCREG;                                  //Guardar valor en indicador
        if (indicador == 0B00110010){                       //Si indicador es 2
            RCREG = 0;
            while (esperar == 0){                           //Esperar a que se presione una tecla
                PORTB = RCREG;                              //Mandar valor a PORTB
                esperar = PORTB;                            //Si no se presiono tecla, esperar = 0
            }
            indicador = 0B00110000;                         //Regresar al menú
            esperar = 0;                                    //Resear variable
        }
    }
    PIR1bits.RCIF = 0;
    PIR1bits.ADIF = 0;
    return;
}

//Setup General
void setup(void){
    //Oscilador
    OSCCON = 0B01000000;                                    //Oscilador a 1Mhz
        
    //Interrupciones
    INTCON = 0B11000000;                                    //Int globales, PEIE activadas
    PIE1 = 0B01100000;                                      //Int ADC, UART match activadas
    PIR1 = 0B00000000;
    
    //UART
    TXSTA = 0B00100100;
    BAUDCTL = 0B00001000;
    RCSTA = 0B10010000;
    SPBRG = 25;
    SPBRGH = 0;
    
    //ADC
    ADCON0 = 0B01000001;                                    //Fosc/8 (2us), AnCH0, ADC encendido 
    ADCON1 = 0B00000000;                                    //Voltajes de referencia y formato a la izquierda
    ANSELH = 0;
    ANSEL = 0B00000001;                                     //Bits 0 y 1 como analogicos

    //Entradas y salidas
    TRISA = 0B00000001;
    TRISB = 0B00000000;                                     //Salidas
    
    //Valores iniciales de variables y puertos
    PORTB = 0;
    return;
}

//Loop
void main(void) {
    setup();
    while(1){
        ADCON0bits.GO = 1;                                  //Iniciar ADC
        if (indicador == 0B00110000){                       //Si esta activado el menú
            cadena("1. Leer potenciometro ");               //Mostrar
            cadena("2. Enviar ASCII ");
            indicador = 0;                                  //Evitar repetir menú y esperar indicador
        }
        else if (indicador == 0B00110001){                  //Si indicador es 1
            cadena(adc);                                    //Mostrar valor del potenciometro
            indicador = 0B00110000;                         //Regresar al menú
        }
        __delay_ms(500);
    }
}

//Funciones
void cadena(char txt[]){
    for (uint8_t i = 0; txt[i] != '\0'; i++){               //Recorrer el string y mostrarlo en
        while(!PIR1bits.TXIF);                              //terminal hasta llegar al final (\0)
        TXREG = txt[i]; 
    }
    __delay_ms(100);
    if (PIR1bits.TXIF){                                     //Si se puede mandar valor
        TXREG = data;                                       //Mandar ENTER en ASCII
    }
}