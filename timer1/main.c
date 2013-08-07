/***Использование динамической индикации***/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*#define BIT(x) (1 << (x)) 
#define SETBITS(x,y) ((x) |= (y)) 
#define CLEARBITS(x,y) ((x) &= (~(y))) 
#define SETBIT(x,y) SETBITS((x), (BIT((y)))) 
#define CLEARBIT(x,y) CLEARBITS((x), (BIT((y)))) 
#define BITSET(x,y) ((x) & (BIT(y))) 
#define BITCLEAR(x,y) !BITSET((x), (y)) 
#define BITSSET(x,y) (((x) & (y)) == (y)) 
#define BITSCLEAR(x,y) (((x) & (y)) == 0) 
#define BITVAL(x,y) (((x)>>(y)) & 1) */

#define SB(x,y) (x|=(1<<y))		//setbit
#define CB(x,y)	(x&= ~(1<<y))	//clearbit
#define TB(x,y) (x^=(1<<y))		//togglebit
#define CH(x,y) (x&(1<<y))		//checkbit

// тоже самое для порта C

#define SBC(x) (PORTC|=(1<<x))		//setbit
#define CBC(x)	(PORTC&= ~(1<<x))	//clearbit
#define TBC(x) (PORTC^=(1<<x))		//togglebit
#define CHC(x) (PORTC&(1<<x))		//checkbit

#define truba_btn (!(PINC&(1<<PC2)))&&(PINC&(1<<PC1))&&(PINC&(1<<PC0)) //100
#define truba_off CBC(5)//(PORTC&= ~(1<<PC5))
#define truba_on SBC(5)//(PORTC|=(1<<PC5))
#define fire_btn ((PINC&(1<<PC2)))&&(!(PINC&(1<<PC1)))&&(PINC&(1<<PC0))// 010
#define podacha_btn ((PINC&(1<<PC2)))&&((PINC&(1<<PC1)))&&(!(PINC&(1<<PC0)))// 001
#define avtomat_btn (!(PINC&(1<<PC2)))&&((PINC&(1<<PC1)))&&(!(PINC&(1<<PC0))) //101
#define count_btn ((PINC&(1<<PC2)))&&(!(PINC&(1<<PC1)))&&(!(PINC&(1<<PC0))) //011

char SEGMENT[ ] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
volatile int adc_but,BUTTON;
volatile int ch_st=0; // 0 - 1 channel, 1 - 2 channel
volatile int count=0,count1=0,count2=0,i=0;
volatile unsigned int adc6,adc7;//данные с каналов ацп
int adc6_a=0,adc7_a=0; //переменные для отображения регулирования таймера
//volatile unsigned int r,x;
volatile unsigned char segcounter = 0;
volatile int display1 = 0;
volatile int display2 = 0;
void InitDisplay();
void InitTimer();
void InitADC();
void InitControl();
void delay(int x);
int trigger(int btn,int port);
//int sec2int(int sec);
int count_st=0;

/*
int sec2int(int sec){
	int time; // как бы это ... массив в строку, а лучше в число
	
	return time;
}

****//////// Обработчик прерывания по переполнению таймера2
ISR (TIMER2_OVF_vect)
{
PORTD = 0xFF; //гасим все разряды
PORTB = (1 << segcounter); //выбираем следующий разряд
switch (segcounter){ 
	case 0:PORTD = ~(SEGMENT[display1  % 6000/600]);break; // здесь раскладываем число на разряды
	case 1:PORTD = ~(SEGMENT[display1  % 600 /60 ]);CB(PORTD,7);break;
	case 2:PORTD = ~(SEGMENT[display1 % 60 /10]);break;
	case 3:PORTD = ~(SEGMENT[display1 % 10]);break;
}
	
switch (segcounter){ 
	case 4:PORTD = ~(SEGMENT[display2 % 6000 / 600]);break; // здесь раскладываем число на разряды
	case 5:PORTD = ~(SEGMENT[display2 % 600 / 60]);CB(PORTD,7);break;
	case 6:PORTD = ~(SEGMENT[display2 % 60 / 10]);break;
	case 7:PORTD = ~(SEGMENT[display2 % 10]);break;
}
if ((segcounter++) > 6) segcounter = 0;
	i--;
}

//Прерывание ADC
ISR (ADC_vect){
	//ADMUX  |= (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);adc6=ADCW;
	//ADMUX  |= (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0);adc7=ADCW;
	if (ADMUX==7){ 
		ADMUX=6; // Перекючить вход ADC6
		adc6=ADCW; // снять показания в adc6
		ADCSRA|=0x40;// // Начать следующее преобразоание
		//ADMUX=0;adc_but=ADCW;ADCSRA|=0x40;ADMUX=6;ADCSRA|=0x40;
		} 

	else { 
		ADMUX=7; // Перекючить вход ADC7 
		adc7=ADCW*6; // Отображать ADCW число на PORTD
		ADCSRA|=0x40; // Начать следующее преобразоание
		//ADMUX=0;adc_but=ADCW;ADCSRA|=0x40;ADMUX=7;ADCSRA|=0x40;
	}
	
	
}
//*/

// Счетчик времени , минуты и секунды
ISR( TIMER1_OVF_vect )
{
  TCNT1 = 64456; //выставляем начальное значение TCNT1 (65536 — 1080) =  для 11МГц 
	//(92.593 мкс 65536*92.593*10^-6 = 6.068 секунды,  8-ми битный —  0.0237 секунды)
	//тактовая частота / предделитель (1024), 16bit=65536 тиков 8бит = 256 тиков 
	// 4000000/1024=3906 ~ 1/3906=256 msec * 65536 = 16.7 sec
 /* if( PIND & ( 1 << PD7 ) ) {
    PORTD &= ~( 1 << PD7 );
  }
  else {
    PORTD |= ( 1 << PD7 );
  }
}*/
/*msec++;
values[0] = msec%10;
values[1] = msec/10;
if (msec == 100)
{
msec = 0;
values[0] = 0;
values[1] = 0;
sec++;
values[2] = sec%10;
values[3] = sec/10;
if (sec==60)
{
sec = 0;
values[2] = 0;
values[3] = 0;
min++;
values[4] = min%10;
values[5] = min/10;
if (min==60)
min = 0;
}
*/
//display1++;
//count--;
//count2++;
//display1--;


//*	 переключатель по таймеру

if (count_st==1){
if (count!=0){
		count--;
		//if (count1>0)count1--;
		//if (count2>0)count2--;
	}
	else{
		if (ch_st==0){
			count=adc6;
			count1=count+1;
			ch_st=1;
			//count1--;
		}//else count1--;
		//if (ch_st==1)
			else{
			count=adc7;
			count2=count+1;
			ch_st=0;
			//count2--;
		}//else count1--;
		
	}

if (ch_st==0) {count2--;}
	else {count1--;}
}
//*/
//case (count==0){
}	
void delay (int x){
	//int h;
	for ( i=x; i>0; ){}
}

int trigger(int btn,int port){
			//*
			if (btn&&(!(CH(PORTC,port)))) {
			delay(250);			
			if (btn&&(!(CH(PORTC,port)))) SBC(port);
				return 0;
				//goto m; //для выхода из функции , тк переменная бтн в функции не меняется
			}				
			// выключить
			if ((CH(PORTC,port))&&btn) {
			delay(250);
			if ((CH(PORTC,port))&&btn) (CB(PORTC,port));
				//goto m;
			return 0;
			}		
		//m:return 0;
			return 0;
		//*/
	/*switch (CHC(port)){
	case 0 :if(btn)SBC(port);break;
	case 1 :if(btn)CBC(port);break;
	}*/
}

/***Главная функция***/
int main (void) 
{
	InitDisplay();
	InitTimer();
	InitADC();
	InitControl();
	//count=500;
	//display1=300;
	//count2=100;
	//count1=100;

	

while(1){
	//count1=adc6;
	//count2=adc7;
	display1=count1; if (display1==0) {display1=adc6_a;delay(200);}
	display2=count2; if (display2==0) {display2=adc7_a;delay(200);}
	
	//adc6_a=adc6;
	//adc7_a=adc7;
	if (adc6_a!=adc6) {display1=adc6;delay(500);adc6_a=adc6;}
	if (adc7_a!=adc7) {display2=adc7;delay(500);adc7_a=adc7;}
	
	/////////***********Счетчик старт стоп , автомат ручное (одно и тоже короче)**********//////////////
	
	if (count_btn&&(count_st==0)){delay(250);if (count_btn&&(count_st==0));count_st=1;} 
	if (count_btn&&(count_st==1)){delay(250);if (count_btn&&(count_st==1));count_st=0;CBC(3);count=0;} //остановка таймера
	if (podacha_btn&&(count_st==0)) {trigger(podacha_btn,3);}
	
	
	//*
	if ((ch_st==1)&&(count_st==1)){PORTC|=(1<<PC3);}
	//else {PORTC&= ~(1<<PC3);}
	if ((ch_st==0)&&(count_st==1)){PORTC&= ~(1<<PC3);}
	//*/
	//////***********************Труба переключатель О_о ************///////////////	
	
	if (truba_btn){trigger(truba_btn,5);}
	//if ((!(PINC&(1<<PC2)))&&(PINC&(1<<PC1))&&(PINC&(1<<PC0))&&(PORTC&= ~(1<<PC5))) {
	/*if (truba_btn&&truba_off) {
			delay(250);			
			if (truba_btn&&truba_off) {SBC(5);}				
		}	// выключить
	if (truba_btn&&truba_on) {
			delay(250);
			if (truba_btn&&truba_on) (truba_off);
		}
		*/
	

			
	//////////*********горелка************/////////////
	if (fire_btn){trigger(fire_btn,4);} //1 
	
	
	//подача
	
	//_delay_ms(100); // задержка
	

	}
	
	return 0;
}

void InitControl(){
	DDRC=0b00111000;
	//DDRC |= (1 << PC1)|(1 << PC2)|(1 << PC3)|(1 << PC4)|(1 << PC5)|(1 << PC6);
	PORTC=0b00000111;
}

void InitDisplay(){
	
	cli();
		// Инициализация таймера и портов индикатора
	DDRD |= (1 << PD0)|(1 << PD1)|(1 << PD2)|(1 << PD3)|(1 << PD4)|(1 << PD5)|(1 << PD6)|(1 << PD7);
	DDRB |= (1 << PB0)|(1 << PB1)|(1 << PB2)|(1 << PB3)|(1 << PB4)|(1 << PB5)|(1 << PB6)|(1 << PB7);
	PORTD = 0x00;
	PORTB = 0x00;
	TIMSK |= (1 << TOIE2); // разрешение прерывания по таймеру2
	TCCR2 |= (1 << CS21); //предделитель на 8 

	sei(); //глобально разрешаем прерывания
}
		// Инициализация счетчика
void InitTimer(){
	cli();
	//DDRD = ( 1 << PD7 );  // настраиваем PC3 на выход
	TCCR1B = (1<<CS12)|(0<<CS11)|(0<<CS10); // настраиваем делитель 100=256 101=1024
	TIMSK |= (1<<TOIE1); // разрешаем прерывание по переполнению таймера 1
	TCNT1 = 64456;        // выставляем начальное значение TCNT1
	sei();                // выставляем бит общего разрешения прерываний
}

void InitADC(){
	cli();
	ADCSRA |=  (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescalar 
	ADMUX |= (1 << REFS0)|(0<<REFS1); // Set ADC reference to AVCC
	// no left ajustment needed in 10 bit mode!
	//ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit reading
   	// No MUX values needed to be changed to use ADC0
	//ADMUX = 6;// |= (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0); //ADC6
	//ADMUX  |= (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0); //ADC7
   	ADCSRA |= (1 << ADFR);  // Set ADC to Free-Running Mode
   	ADCSRA |= (1 << ADEN);  // Enable ADC
   	ADCSRA |= (1 << ADSC);// Start A2D Conversions
	ADCSRA |= (1 << ADIE);//Бит 3 – ADIE (ADC Interrupt Enable) – бит разрешения прерывания аналого–цифрового преобразователя : 1 – разрешено, 0 – запрещено.
	sei();
}