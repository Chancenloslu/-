#include "stc15f2k60s2.h"
#include "absacc.h"
#include "iic.h"

#define setkeyboard(x) P4=((x&0x80)>>3)|((x&0x40)>>4);P3=x
#define getkeyboard() ((P4&0x10)<<3)|((P4&0x04)<<4)|(P3&0x3f)

sbit buzzer=P0^6;
sbit relay=P0^4;
bit key_flag,water_flag;

float real_gm_value;
unsigned int water_flow,water_price;
unsigned char read_key_value,trg,cont;
unsigned char display[8];
unsigned char smg_duan[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
													//   0    1    2    3    4    5    6    7    8    9  
unsigned char smg_wei[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
//-----------------------------------------------

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
  static unsigned char smg_count,key_count,water_count,i;
	smg_count++;key_count++;
	if(key_count==10)
	{
		key_count=0;
		key_flag=1;
	}
	if(smg_count==3)
	{
		smg_count=0;
		P2=0xc0;P0=0xff;P2=0;
		P2=0xe0;P0=~display[i];P2=0;
		P2=0xc0;P0=smg_wei[i];P2=0;
	
//		XBYTE[0XC000]=0XFF;
//		XBYTE[0XE000]=~smg_duan[i];
//		XBYTE[0XC000]=(1<<i);
		i++;
		if(i==8)i=0;
	}
	if(water_flag)
	{
		water_count++;
		if(water_count==100)
		{
			water_count=0;
			water_flow++;
		}
	}
}

//-----------------------------------------------
void main()
{
	AUXR |= 0x80;		//?????1T??
	TMOD &= 0xF0;		//???????
	TL0 = 0xCD;		//??????
	TH0 = 0xD4;		//??????
	TF0 = 0;		//??TF0??
	TR0 = 1;		//???0????	
	ET0 = 1;
	EA = 1;
	
	P2=0xa0;buzzer=0;relay=0;P2=0;
	display[0]=0x00;display[1]=smg_duan[0]|0x80;display[2]=smg_duan[5];display[3]=smg_duan[0];
	
	while(1)
	{
		ET0=0;
		real_gm_value=Read_PCF8591(0X01)/255.0*5.0;
		ET0=1;
		if(real_gm_value<1.25)
		{
			P2=0x80;P0=~0x01;P2=0;
		}
		else
		{
			P2=0x80;P0=~0x00;P2=0;
		}
		if(water_flag)			//出水
		{
			display[4]=smg_duan[water_flow/1000];
			display[5]=smg_duan[water_flow/100%10]|0x80;
			display[6]=smg_duan[water_flow/10%10];
			display[7]=smg_duan[water_flow%10];
			if(water_flow==9999)
			{
				P2=0xa0;relay=0;buzzer=0;P2=0;water_flag=0;
			}
		}
		else								//停水
		{
			water_price=water_flow*5;
			display[4]=smg_duan[water_price/10000];
			display[5]=smg_duan[water_price/1000%10]|0x80;
			display[6]=smg_duan[water_price/100%10];
			display[7]=smg_duan[water_price/10%10];
		}
		if(key_flag)
		{
			key_flag=0;
			setkeyboard(0xf0);
			read_key_value = getkeyboard();
			setkeyboard(0x0f);
			read_key_value = (read_key_value|getkeyboard())^0xff;
			trg=read_key_value&(read_key_value^cont);
			cont=read_key_value;
			
			if(water_flag==0)
			{
				switch(trg)
				{
					case 0x81:P2=0xa0;relay=1;buzzer=0;P2=0;
					water_price=0;
					water_flow=0;
					water_flag=1;
					break;//s7				出水
				}
			}
			switch(trg)
			{
				case 0x88:break;
				case 0x84:break;
				case 0x82:P2=0xa0;relay=0;buzzer=0;P2=0;water_flag=0;break;//s6				停水
				
			}
			
		}
	}
}