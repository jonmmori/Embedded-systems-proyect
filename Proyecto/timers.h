/*
 * M�dulo UART2
 * ==================
 * Autores:
 * Jon Mor��igo
 * Iker Fern�ndez
 */
extern unsigned int scroll;
extern unsigned int fin_programa;
extern unsigned int objetivo;
extern unsigned int ultraSonicFlag;
///////////
//Funciones
///////////
void inic_Timer1();
void inic_Timer2_PWM();
void inic_Timer3();
void inic_Timer5();
void inic_Timer4();
void inic_Timer6();
void inic_Timer7();
void inic_Timer9 ();
void Delay_us(int time);
void Delay_ms(int time);

void cronometro(unsigned int *mili,unsigned int *deci,unsigned int *seg,unsigned int *min);
void inic_crono(unsigned int *mili,unsigned int *deci,
                unsigned int *seg,unsigned int *min);
void reinic_crono(unsigned int *mili,unsigned int *deci,
                unsigned int *seg,unsigned int *min);
