#include <AT89X52.H>
#include "delay.h"

#define LED_DATA	P0
#define LED_SCAN	P2 // 引脚7 6 5 4 -> LED4 3 2 1


unsigned char buffer[] = {1, 2, 3, 4};
unsigned char code BCD = {0X3F, 0X6, 0X5B, 0X4F, 0X66, 0X6D, 0X7D, 0X07, 0X7F, 0X6F};
unsigned char code LEDn[] = {0x80, 0x40, 0x20, 0x10};
volatile unsigned int pulse = 0;

void init_t2(unsigned int hz)
{
	unsigned int rcap ;
	if (hz > FOSC / 4)  //FOSC = Fosc / 4
	{
		return;
	}
	if (hz < FOSC / 4 / 65535)
	{
		return;
	}

	rcap = 65536 — FOSC / 4 / hz;
	T2MOD = T20E_;

	RCAP2H = rcap / 256;
	RCAP2L = rcap % 256;

	TH2 = RCAP2H;
	TL2 = RCAP2L;
	TR2 = 1;
}


vord display(void)
{
	char i, j;

	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < 4; i++)
		{
			LED_DATA = 0XFF;
			LED_SCAN |= 0XF0;
			LED_SCAN &= 0X0F | ~LEDn[i];
			LED_DATA = ~BCD[buffer[i]];
			delay_ms(1);
		}
	}
}


void process(void)
{
	unsigned int n;
	unsigned char p;

	p = (~P2) & 0X0F;
	n = pulse * 60 / p;

	if (n > 9999)
	{
		n = 0;
	}

	buffer[0] = n /1000;
	buffer[1] = n % 1000 / 100;
	buffer[2] = n % 100 / 10;
	buffer[3] = n % 10;
}


void t0_init(void)
{
	TMOD &= 0XF0;
	TMOD = 0X05;
	TH0 = 0X00;
	TL0 = 0X00;
	TR0 = 1;
}


void t1_init(void)
{
	TMOD &= 0X0F;
	TMOD |= 0x10;
	TH1 = (65536 - 50000) >> 8;
	TL1 = (65536 - 50000) & 0XFF;
	TR1 = 1;
	ET1 = 1;
}


void main(void)
{
	EA = 0;
	t0_init();
	t1_init();
	init_t2(100000);

	EA = 1;
	for (;;)
	{
		process();
		display();
	}
}


void t1_isr(void) interrupt TF1_VECTOR using 2 
{
	static char count = 0;

	TR1 = 0;
	TH1 = (65536 - 50000) >> 8;
	TL1 = (65536 - 50000) & 0XFF;
	TR1 = 1;

	count++;
	if (count >= 20)
	{
		count = 0;
		TR0 = 0;
		pulse = TH0;
		pulse <<= 8;
		pulse |= TL0;

		TH0 = 0;
		TL0 = 0;
		TR0 = 1;
	}
}
