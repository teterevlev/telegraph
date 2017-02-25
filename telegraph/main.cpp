/*
 * telegraph.cpp
 *
 * Created: 21.02.2017 18:35:29
 * Author : odama
 */ 

#define F_CPU 16000000UL
#define NOISE_CANCEL 10
#define DASH_DURATION 300
#define DOT_DURATION 100
#define BEEP_MASK 0b00000010
#define INVERT(mask) (mask) ^ (0xff)
#define DASH '_'
#define DOT '.'
#define OPEN 1
#define ADMIN 2
#define TIME_TO_LOAD 15000

#include <avr/io.h>
#include <util/delay.h>
#include "WString.h"
#define BAUD 9600
#include <util/setbaud.h>

String code = "..._.__.._..__.";
String inputString = "nnnnnnnn";
String admin = "___._..__.__.._";
void uart_init(void) {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}
void uart_putchar(uint8_t c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
	UDR0 = c;
}
void printInputString(){
	for(uint8_t i=0;i<inputString.length();i++){
		uart_putchar(inputString[i]);
	}
	uart_putchar('\n');
}

void delay(uint16_t ms){
	while((ms--) > 0){
		_delay_ms(1);
	}
}
void delay_us(uint16_t ms){
	while((ms--) > 0){
		_delay_us(1);
	}
}
bool keyDown(){
	if(PINC & 0b00000100)
	{
		return false;
	}
	return true;
}
void beep(uint16_t d){
	PORTC |= BEEP_MASK;
	delay(d);
	PORTC &= INVERT(BEEP_MASK);
}
void push(uint8_t c){
	if(inputString.length() >= code.length()){
		inputString = inputString.substring(1, inputString.length());
	}
	inputString+=(char)c;
}
uint8_t checkCode(){
	if(inputString == code){
		return OPEN;
	}else if(inputString == admin){
		return ADMIN;
	}
	return 0;
}
void step1(){
	PORTB = 0b00000011;
	PORTC = 0b00000100;
	PORTD = 0b10000000;
}
void step4(){
	PORTB = 0b00011100;
	PORTC = 0b00000100;
	PORTD = 0b00000000;
}
void step3(){
	PORTB = 0b00000000;
	PORTC = 0b00110100;
	PORTD = 0b00001000;
}
void step2(){
	PORTB = 0b0000000;
	PORTC = 0b00000100;
	PORTD = 0b01110000;
}

void lap(uint16_t d){
	step1();
	delay_us(d);
	step2();
	delay_us(d);
	step3();
	delay_us(d);
	step4();
	delay_us(d);
}
void stop(){
	PORTB = 0;
	PORTC = 0b100;
	PORTD = 0;
}
void accelerate(){
	for(int i=10000; i>3000; i-=100){
		lap(i);
	}
}
void print(){
	//delay(5000);
	accelerate();
	for(int i=0;i<1000;i++){
		lap(4000);
	}
	stop();
}
void reverse(uint16_t d){
	step1();
	delay_us(d);
	step4();
	delay_us(d);
	step3();
	delay_us(d);
	step2();
	delay_us(d);
}
void load(){
	delay(500);
	for(uint8_t i=0;i<10;i++){
		beep(40);
		delay(40);
	}
	
	uint16_t millis=0;
	while(millis<TIME_TO_LOAD){
		while(keyDown()){
			reverse(10000);
			millis = 0;
		}
		stop();
		millis++;
		delay(1);
	}
	for(uint8_t i=0;i<10;i++){
		beep(40);
		delay(40);
	}
}
int main(void)
{
	
	/*
	B: 0b00011122
	C: 0b00440KB0
	D: 0b23334000
	*/
	DDRB = 0b00111111;
	DDRC = 0b00111010;
	DDRD = 0b11111000;
	
	PORTC = 0b00000100;
	uart_init();
	uint16_t millis = 0;
	while(0){
		step1();
		delay(100);
		step2();
		delay(100);
		step3();
		delay(100);
		step4();
		delay(100);
	}
    while (1) 
    {
		while(!keyDown());
		millis = 0;
		if(keyDown()){
			_delay_ms(NOISE_CANCEL);
			while(keyDown()){
				_delay_ms(1);
				millis++;
				if(millis>300){
					millis = DASH_DURATION; // to not overflow
					push(DASH);
					beep(DASH_DURATION);
					while(keyDown());
				}
			}
		}
		
		if(millis < DASH_DURATION){
			push(DOT);
			beep(DOT_DURATION);
		}
		uint8_t check = checkCode();
		if(check == OPEN){
			print();
		}else if(check==ADMIN){
			load();
		}
		printInputString();
	}
}

