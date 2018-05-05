#include "stc15f2k60s2.h"
#include "absacc.h"

#define setkeyboard(x) P4=((x&0x80)>>3)|((x&0x40)>>4);P3=x
#define getkeyboard ((P4&0x10)<<3)|((P4&0x04)<<4)|(P3&0x3f)

unsigned char read_key_data,trg,cont;
sbit buzzer=P0^6;
unsigned char smg_com[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
unsigned char smg_duan[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
												//   0    1    2    3    4    5    6    7    8    9  
bit key_flag;

void tm1_isr() interrupt 3 using 2
{
	
}

void tm0_isr() interrupt 1 using 1
{
   static unsigned char i,smg_count,key_count;
	smg_count++;key_count++;
	if(key_count==10)
	{
		key_count=0;
		key_flag=1;
	}
	

	if(smg_count==3)
	{
		smg_count=0;
		P2=0xc0;P0=0;P2=0x00;
		P2=0xe0;P0=~smg_duan[i];P2=0x00;		
		P2=0xc0;P0=smg_com[i];P2=0x00;
		i++;
		if(i==8)i=0;
	}
}


void main()
{
	AUXR |= 0xC0;		//?????1T??
	TMOD &= 0xFF;		//???????
	TL0 = 0xCD;		//??????
	TH0 = 0xD4;		//??????
	TF0 = 0;		//??TF0??
	TR0 = 1;		//???0????
	ET0 = 1;
	TL1 = 0xAE;		//??????
	TH1 = 0xFB;		//??????
	TF1 = 0;		//??TF1??
	TR1 = 1;		//???1????
	ET1 = 1;
	EA = 1;
	
	P2=0xa0;buzzer=0;P2=0x00;
	while(1)
	{
//	XBYTE[0XC000]=0X00;
//	XBYTE[0XE000]=~smg_duan[i];
//	XBYTE[0XC000]=(1<<i);
//	if(++i==8)
//	{
//		i=0;
//	}
		if(key_flag)
		{
			key_flag=0;
			setkeyboard(0x0f);
			read_key_data|=getkeyboard;
			setkeyboard(0xf0);
			read_key_data=(read_key_data|getkeyboard)^0xff;
			trg=read_key_data&(read_key_data^cont);
			cont=read_key_data;
			switch(trg)
			{
				case 0x88:P2=0x80;P0=0x55;P2=0x00;break;	//s4
				case 0x84:P2=0x80;P0=0xaa;P2=0x00;break;	//s5
				case 0x82:P2=0xa0;buzzer=1;P2=0x00;break;	//s6
				case 0x81:P2=0xa0;buzzer=0;P2=0x00;break;	//s7
				case 0x48:break;	//s8
				case 0x44:break;	//s9
				case 0x42:break;	//s10
				case 0x41:break;	//s11
				case 0x28:break;	//s12
				case 0x24:break;	//s13
				case 0x22:break;	//s14
				case 0x21:break;	//s15
				case 0x18:break;	//s16
				case 0x14:break;	//s17
				case 0x12:break;	//s18
				case 0x11:break;	//s19
				
			}
		}
	}
}