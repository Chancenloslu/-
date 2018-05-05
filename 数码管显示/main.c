#include "stc89c5Xrc.h"
#define u16 unsigned int
typedef unsigned char u8;

u8 date[8]={0x5b,0x3f,0x06,0x7f,0x40,0x5b,0x40,0x4f };//2018-2-3
u8 smg_com[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};//comλѡ
sbit buzzer=P0^6;
void keyscan()
{
	P3=0x0f;
	
	
}

	void delay(u16 i)
{
	while(i--);
}

main()
{
	u8 i;
	P2=((P2&0X1F)|0Xa0);
	buzzer=0;
	P2&=0x1f;

	while(1)
	{
		for(i=0;i<8;i++)
		{
		P2=((P2&0X1F)|0Xc0);
		P0=smg_com[i];
		P2=((P2&0X1F)|0Xe0);
		P0=~date[i];
		delay(100);
		P0=0x00;
		}
	}
}