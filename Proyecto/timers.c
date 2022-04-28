/*
 * M�dulo UART2
 * ==================
 * Autores:
 * Jon Mor��igo
 * Iker Fern�ndez
 */

#include "p24HJ256GP610A.h"
#include "commons.h"
#include "timers.h"
#include "utilidades.h"
#include "memoria.h"
#include "LCD.h"

unsigned int flag_T9=0;
unsigned int flag_crono=0;
unsigned int estado_LCD=0;
unsigned int L5=0;
////////////////////
//Inicializaciones
////////////////////
void inic_Timer5 ()
{

    TMR5 = 0 ; 	// Inicializar el registro de cuenta
    PR5 =  40000-1;	// Periodo del temporizador
		// Queremos que cuente 2 ms.
		// Fosc= 40 MHz (vease Inic_oscilator()) de modo que
		// Fcy = 20 MHz (cada instruccion dos ciclos de reloj)
		// Por tanto, Tcy= 50 ns para ejecutar una instruccion
		// Para contar 2 ms se necesitan 40000 ciclos.
    T5CONbits.TCKPS = 0;	// escala del prescaler 1:1
    T5CONbits.TCS = 0;	// reloj interno
    T5CONbits.TGATE = 0;	// Deshabilitar el modo Gate
    IEC1bits.T5IE =1;
    IFS1bits.T5IF =0;
    T5CONbits.TON = 1;	// puesta en marcha del temporizado
}

void inic_Timer7 ()
{

    TMR7 = 0 ; 	// Inicializar el registro de cuenta
    PR7 =  25000-1;	// Periodo del temporizador
		// Queremos que cuente 10 ms.
		// Fosc= 40 MHz (vease Inic_oscilator()) de modo que
		// Fcy = 20 MHz (cada instruccion dos ciclos de reloj)
		// Por tanto, Tcy= 50 ns para ejecutar una instruccion
		// Para contar 10 ms se necesitan 200000 ciclos.
    T7CONbits.TCKPS = 1;	// escala del prescaler XXX
    T7CONbits.TCS = 0;	// reloj interno
    T7CONbits.TGATE = 0;	// Deshabilitar el modo Gate
    IEC3bits.T7IE = 1;  //Activamos las interrupciones
    IFS3bits.T7IF = 0;  //Puesta del flag IF a 0
    T7CONbits.TON = 1;	// puesta en marcha del temporizador
}
void inic_Timer9 (int period, int scale){

    TMR9 = 0 ; 	// Inicializar el registro de cuenta
    PR9 =  period-1;	// Periodo del temporizador
		// Queremos que cuente period timepo.
		// Fosc= 40 MHz (vease Inic_oscilator()) de modo que
		// Fcy = 20 MHz (cada instruccion dos ciclos de reloj)
		// Por tanto, Tcy= 50 ns para ejecutar una instruccion
		// Para contar 1 us se necesitan 20 ciclos.
    T9CONbits.TCKPS = scale;	// escala del prescaler 1:1
    T9CONbits.TCS = 0;	// reloj interno
    T9CONbits.TGATE = 0;	// Deshabilitar el modo Gate
    IEC3bits.T9IE = 1;
    IFS3bits.T9IF = 0;
    T9CONbits.TON = 1;	// puesta en marcha del temporizador
}	

void Delay_us(int time){
    
    int period,scale=0;
    if(time>3276){  //si el tiempo es mayo que 2^16/(1000/50), lo que buscamos con esto es encontrar el limite para que el prescaler sea 0. Siendo 1e3 los us a ns , 50 el Tcy y 2^16 los 16 bits del registro del temporizador
        scale=1;
        time=time/8; //si no se utiliza prescaler 1
    }
    period=time * 20; //establecemos el periodo
    inic_Timer9(period, scale); //inicializamos el timer
    while(!flag_T9){}
    flag_T9=0;
}
void Delay_ms(int time){
    int period,scale;
    if(time<3){ //el funcionamiento es el mismo que delay_us pero esta vez de ms a ns son 1e6
        scale=0;
        period = time * 20000;
    }else if(time<27){
        scale=1;
        period= time * 2500; 
    }else{
        scale=2;
        period = time * 313;
    }
    inic_Timer9(period, scale);
    while(!flag_T9){}
    flag_T9=0;
}

//////////////////
//Funciones
//////////////////
void inic_crono(unsigned int *mili,unsigned int *deci,
                unsigned int *seg,unsigned int *min)	
// inicializacion de las variables del cronometro
{
    (*mili)=0;
    (*deci)=0;
    (*seg)=0;
    (*min)=0;
}

void reinic_crono(unsigned int *mili,unsigned int *deci,
                unsigned int *seg,unsigned int *min)	
// Reiniciar las variables del cronometro
{
    (*mili)=0;
    (*deci)=0;
    (*seg)=0;
    (*min)=0;
    
    LATAbits.LATA1=0;
    LATAbits.LATA3=0;
    LATAbits.LATA5=0;
    
}

void cronometro(unsigned int *mili,unsigned int *deci,unsigned int *seg,unsigned int *min)	// control del tiempo mediante el temporizador 7
{   
    unsigned char tiempo[2];
    //LCD
    
    if(flag_crono==1 && T7CONbits.TON){// espera a que pasen 10 milisegundos 10ms
        flag_crono=0;                       //Volvemos lvemos a bajar el flag
        (*mili)+=10;    //sumamos 10 segundos 
        if((*mili)==100) {                      //Si milisegundos llega a 100
            LATAbits.LATA1=!LATAbits.LATA1;     //Conmutamos el led D4
            (*mili)=0;                          //Reseteamos los milisegundos
            (*deci)++;  
            if((*deci)==10) {                       //si las decimas llegan a a 100
                LATAbits.LATA3=!LATAbits.LATA3;     //Conmutamos el led D6
                (*deci)=0;                          //Reseteamos los decimales
                (*seg)++;                           //Incrementamos los sergundos
                conversion_tiempo(tiempo, (*deci));
                LCD_Pantalla[1][14]=tiempo[1]; 
                if((*seg)==60 ) {                       //Si segundos llega a 60
                    LATAbits.LATA5=!LATAbits.LATA5;     //Conmutamos el led D8
                    (*seg)=0;                           //Reseteamos los segundos
                    (*min)++; 
                    conversion_tiempo(tiempo, (*seg));
                    LCD_Pantalla[1][11]=tiempo[0]; 
                    LCD_Pantalla[1][12]=tiempo[1];
                    conversion_tiempo(tiempo, (*min));
                    LCD_Pantalla[1][8]=tiempo[0]; 
                    LCD_Pantalla[1][9]=tiempo[1];//Incrementamos los minutos
                }else{
                    conversion_tiempo(tiempo, (*seg));
                    LCD_Pantalla[1][11]=tiempo[0]; 
                    LCD_Pantalla[1][12]=tiempo[1];
                }//Incrementamos los segundos
            }else{
                conversion_tiempo(tiempo, (*deci));    
                LCD_Pantalla[1][14]=tiempo[1];
            }    
        } 
        
    }  
    
} 




//////////////////
//Interrupciones
//////////////////
void _ISR_NO_PSV _T5Interrupt(){
    
    switch (estado_LCD) {       //segun en que estado estemos haremos una cosa u otras
        case 0 :            //en el estado 0 nos colocamos en la linea 1
            lcd_cmd(0x80);
            estado_LCD++;
            break;
        case 1:             //en el estado 1 vamos recorriendo todos los caracteres de la linea 0 por cada interrupcion 
            lcd_data(LCD_Pantalla[0][L5]);
            L5++;
            if (L5==16){
                L5=0;
                estado_LCD++;
            }
            break;
        case 2:             //en el estado 2 nos colocamos en la linea 2
            lcd_cmd(0xC0);
            estado_LCD++;
            break;
        case 3:             //en el estado 3 vamos recorriendo todos los caracteres de la linea 1 por cada interrupcion
            lcd_data(LCD_Pantalla[1][L5]);
            L5++;
            if (L5==16){
                L5=0;
                estado_LCD=0;
            }
            break;       
    }
    IFS1bits.T5IF=0;    //bajamos el flag de la interrupcion
}

void _ISR_NO_PSV _T7Interrupt(){
    flag_crono=1;       //Subimos el flag del contador cronometro
    IFS3bits.T7IF=0;    //Ponemos el flag T7IF a 0
}

void _ISR_NO_PSV _T9Interrupt(){
    flag_T9=1;
    T9CONbits.TON = 0; //apagamos el timer
    IFS3bits.T9IF=0;
}