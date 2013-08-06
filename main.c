/***������***/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

////////************���������� �������

#define SB(x,y) (x|=(1<<y))		//setbit
#define CB(x,y)	(x&= ~(1<<y))	//clearbit
#define TB(x,y) (x^=(1<<y))		//togglebit
#define CH(x,y) (x&(1<<y))		//checkbit

// ���� ����� ��� ����� C

#define SBC(x) (PORTC|=(1<<x))		//setbit
#define CBC(x)	(PORTC&= ~(1<<x))	//clearbit
#define TBC(x) (PORTC^=(1<<x))		//togglebit
#define CHC(x) (PORTC&(1<<x))		//checkbit
/////////******************������
#define truba_btn (!(PINC&(1<<PC2)))&&(PINC&(1<<PC1))&&(PINC&(1<<PC0)) //100
#define truba_off CBC(5)//(PORTC&= ~(1<<PC5))
#define truba_on SBC(5)//(PORTC|=(1<<PC5))
#define fire_btn ((PINC&(1<<PC2)))&&(!(PINC&(1<<PC1)))&&(PINC&(1<<PC0))// 010
#define podacha_btn ((PINC&(1<<PC2)))&&((PINC&(1<<PC1)))&&(!(PINC&(1<<PC0)))// 001
#define avtomat_btn (!(PINC&(1<<PC2)))&&((PINC&(1<<PC1)))&&(!(PINC&(1<<PC0))) //101
#define count_btn ((PINC&(1<<PC2)))&&(!(PINC&(1<<PC1)))&&(!(PINC&(1<<PC0))) //011
//////////////*******************����������
char SEGMENT[ ] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
volatile int adc_but,BUTTON;
volatile int ch_st=0; // 0 - 1 channel, 1 - 2 channel
volatile int count=0,count1=0,count2=0,i=0;
volatile unsigned int adc6,adc7;//������ � ������� ���
int adc6_a=0,adc7_a=0; //���������� ��� ����������� ������������� �������
//volatile unsigned int r,x;
volatile unsigned char segcounter = 0;
volatile int display1 = 0;
volatile int display2 = 0;
int count_st=0;
int z=10; // �� �������� ��������� ��� ���������� ��������
/////////////**********************������� �������������
void InitDisplay();
void InitTimer();
void InitADC();
void InitControl();
///////////////**************�������
void delay(int x);				//������� ��������
int trigger(int btn,int port);	//������� ������������


//****//////// ************���������� ���������� �� ������������ �������2
ISR (TIMER2_OVF_vect)
{
PORTD = 0xFF; //����� ��� �������
PORTB = (1 << segcounter); //�������� ��������� ������
switch (segcounter){ 						// ����� �������� ������ , ������� �� ������ �������� ������
	case 0:PORTD = ~(SEGMENT[display1  % 6000/600]);delay(z);break; // ����� ������������ ����� �� �������
	case 1:PORTD = ~(SEGMENT[display1  % 600 /60 ]);CB(PORTD,7);delay(z);break; // +�����
	case 2:PORTD = ~(SEGMENT[display1 % 60 /10]);delay(z);break;					// �������� ������� ������ 5 ������������ ��������
	case 3:PORTD = ~(SEGMENT[display1 % 10]);delay(z);break;
}
	
switch (segcounter){ 
	case 4:PORTD = ~(SEGMENT[display2 % 6000 / 600]);delay(z);break; // ����� ������������ ����� �� �������
	case 5:PORTD = ~(SEGMENT[display2 % 600 / 60]);CB(PORTD,7);delay(z);break;
	case 6:PORTD = ~(SEGMENT[display2 % 60 / 10]);delay(z);break;
	case 7:PORTD = ~(SEGMENT[display2 % 10]);delay(z);break;
}
if ((segcounter++) > 6) segcounter = 0;
	i--;
}

//���������� ADC
ISR (ADC_vect){
	//ADMUX  |= (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);adc6=ADCW;
	//ADMUX  |= (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0);adc7=ADCW;
	if (ADMUX==6){ 
		ADMUX=7; // ���������� ���� ADC6
		adc6=ADCW/10; // ����� ��������� � adc6
		ADCSRA|=0x40;// // ������ ��������� �������������
		} 

	else { 
		ADMUX=6; // ���������� ���� ADC7 
		adc7=ADCW/10*60; // ����� ��������� ADC7
		ADCSRA|=0x40; // ������ ��������� �������������
	}
	
	
}
//*/

// ������� ������� , ������ � �������


ISR( TIMER1_OVF_vect )
{
  TCNT1 = 57724; //���������� ��������� �������� TCNT1 (65536 � 1080) =  ��� 11��� 64456
	//(92.593 ��� 65536*92.593*10^-6 = 6.068 �������,  8-�� ������ �  0.0237 �������)
	//�������� ������� / ������������ (1024), 16bit=65536 ����� 8��� = 256 ����� 
	// 4000000/1024=3906 ~ 1/3906=256 msec * 65536 = 16.7 sec
//*	 ������������� �� �������

if (count_st==1){
if (count!=0){
		count--;
	}
	else{
		if (ch_st==0){
			count=adc6;
			count1=count+1;
			ch_st=1;
		}
			else{
			count=adc7;
			count2=count+1;
			ch_st=0;
		}
		
	}

if (ch_st==0) {count2--;}
	else {count1--;}
}

}	
void delay (int x){

	for ( i=x; i>0; ){}
}

int trigger(int btn,int port){
			//*
			if (btn&&(!(CH(PORTC,port)))) {
			delay(250);			
			if (btn&&(!(CH(PORTC,port)))) 
				SBC(port);
				//{SBC(port);while ((btn)){if((!btn))break;};}
			delay(750);
				return 0;
			}				
			// ���������
			if ((CH(PORTC,port))&&btn) {
			delay(250);
			if ((CH(PORTC,port))&&btn) 
				(CB(PORTC,port));
				//{(CB(PORTC,port));while ((btn)){if((!btn))break;};}
			delay(750);
			return 0;
			}		
			return 0;
		
}

/***������� �������***/
int main (void) 
{
	InitDisplay();
	InitTimer();
	InitADC();
	InitControl();

	

while(1){

	display1=count1; if (display1==0) {display1=adc6_a;delay(2000);}
	display2=count2; if (display2==0) {display2=adc7_a;delay(2000);}

	if (adc6_a!=adc6) {display1=adc6;delay(2000);adc6_a=adc6;}
	if (adc7_a!=adc7) {display2=adc7;delay(2000);adc7_a=adc7;}
	
	/////////***********������� ����� ���� , ������� ������ (���� � ���� ������ ���� ���)**********//////////////
	
	if (count_btn&&(count_st==0)){delay(150);if (count_btn&&(count_st==0));count_st=1;delay(750);} 
	if (count_btn&&(count_st==1)){delay(150);if (count_btn&&(count_st==1));count_st=0;CBC(3);count=0;delay(750);} //��������� �������
	if (podacha_btn&&(count_st==0)) {trigger(podacha_btn,3);}
	
	
	//*
	if ((ch_st==1)&&(count_st==1)){PORTC|=(1<<PC3);}
	if ((ch_st==0)&&(count_st==1)){PORTC&= ~(1<<PC3);}
	//*/
	//////***********************����� ������������� �_� ************///////////////	
	
	if (truba_btn){trigger(truba_btn,5);}
	

			
	//////////*********�������************/////////////
	if (fire_btn){trigger(fire_btn,4);} //1 
	
	

	

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
		// ������������� ������� � ������ ����������
	DDRD |= (1 << PD0)|(1 << PD1)|(1 << PD2)|(1 << PD3)|(1 << PD4)|(1 << PD5)|(1 << PD6)|(1 << PD7);
	DDRB |= (1 << PB0)|(1 << PB1)|(1 << PB2)|(1 << PB3)|(1 << PB4)|(1 << PB5)|(1 << PB6)|(1 << PB7);
	PORTD = 0x00;
	PORTB = 0x00;
	TIMSK |= (1 << TOIE2); // ���������� ���������� �� �������2
	TCCR2 |= (0 << CS22)|(1 << CS21)|(0 << CS20); //������������ �� 8 
	

	sei(); //��������� ��������� ����������
}
		// ������������� ��������
void InitTimer(){
	cli();
	//DDRD = ( 1 << PD7 );  // ����������� PC3 �� �����
	TCCR1B = (1<<CS12)|(0<<CS11)|(1<<CS10); // ����������� �������� 100=256 101=1024
	TIMSK |= (1<<TOIE1); // ��������� ���������� �� ������������ ������� 1
	TCNT1 = 57724;        // ���������� ��������� �������� TCNT1 64456 57724
	// � ���� ������ ��� 8��� 65536-7812=57724 8000000/1024=7812 ����� �� ���� �������
	//������� ������� � 57724
	//TCCR1B |= (1 << WGM12)|(0 << CS12)|(1 << CS12)|(0 << CS12); //8MHz/256/(1+31249)=1Hz
	//OCR1A = 31249;
	sei();                // ���������� ��� ������ ���������� ����������
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
	ADCSRA |= (1 << ADIE);//��� 3 � ADIE (ADC Interrupt Enable) � ��� ���������� ���������� ���������������� ��������������� : 1 � ���������, 0 � ���������.
	sei();
}