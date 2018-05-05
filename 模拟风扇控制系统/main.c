#include "stc15f2k60s2.h"
#include "onewire.h"

#define setkeyboard(x) P4=((x&0x80)>>3)|((x&0x40)>>4);P3=x
#define getkeyboard() ((P4&0x10)<<3)|((P4&0x04)<<4)|(P3&0x3f)
sbit buzzer=P0^6;

unsigned char key_count,smg_count,read_key_data,trg,cont,mode_index,time_minute_set,read_temp,pwm_count=0;
unsigned int time_second_set,time_count;
bit key_flag,display_flag,mode_index_flag;
unsigned char display[8];
unsigned char smg_com[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
unsigned char smg_duan[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
												//   0    1    2    3    4    5    6    7    8    9 

/* Timer0 interrupt routine */
void tm1_isr() interrupt 3 using 2
{
	static unsigned char i;
	smg_count++;
	if(smg_count==3)
	{
		smg_count=0;
		P2=((P2&0x1f)|0xc0);P0=0;P2&=0x1f;
		P2=((P2&0x1f)|0xe0);P0=~display[i];P2&=0x1f;		
		P2=((P2&0x1f)|0xc0);P0=smg_com[i];P2&=0x1f;
//		P2=0xc0;P0=~display[i];P2=0x00;
		i++;
		if(i==8)i=0;
		
	}
}
void tm0_isr() interrupt 1 using 1
{
	key_count++;
	if(key_count==100)
	{
		key_count=0;
		key_flag=1;
	}


	if(time_second_set>0)
	{
		pwm_count++;
		switch(mode_index)
		{
			case 1:{if(pwm_count==8)P34=1;}break;
			case 2:{if(pwm_count==7)P34=1;}break;
			case 3:{if(pwm_count==3)P34=1;}break;
		}
		if(pwm_count==10)
		{
			pwm_count=0;
			P34=0;
		}
		time_count++;
		if(time_count==10000)
		{
			time_count=0;
			time_second_set--;
		}
	}
	else
	{
		P2=0x80;P0=~0x00;P2=0x00;
		P34=0;
	}

}

void main()
{
	
	AUXR |= 0x80;		//1T
	AUXR |= 0x40;		//1T
	TMOD &= 0xFF;		//设置定时器模式
	TL0 = 0xAE;		//
	TH0 = 0xFB;		//初值
	TF0 = 0;		//清零TF0
	TR0 = 1;		//T0
	TL1 = 0xCD;		//
	TH1 = 0xD4;		//初值1ms
	TF1 = 0;		//清零TF1
	TR1 = 1;		//T1
	ET0 = 1;
	ET1 = 1;
	EA = 1;
	
//	PCA_PWM0&=0x3f;		//选择8位PWM
//	CCAPM0=0x42;
	//CCAP0H = CCAP0L = CDH//20%;CCAP0H = CCAP0L = B3H//30%;CCAP0H = CCAP0L = 4DH;//70%
	mode_index=1;
	time_second_set=0;
	P2=((P2&0x1f)|0xa0);buzzer=0;P2&=0x1f;
	display[0]=0x40;display[2]=0x40;display[3]=0x00;
	while(1)
	{

		read_temp=(unsigned char)rd_temprature_f();
		
		switch(mode_index)
		{
			case 1:
			{
				display[1]=smg_duan[1];
				if(time_second_set>0)
				{
					P2=((P2&0x1f)|0x80);P0=~0x01;P2&=0x1f;
				}
			}break;
			case 2:
			{
				display[1]=smg_duan[2];
				if(time_second_set>0)
				{
					P2=((P2&0x1f)|0x80);P0=~0x02;P2&=0x1f;				
				}
			}break;
			case 3:
			{
				display[1]=smg_duan[3];
				if(time_second_set>0)
				{
					P2=((P2&0x1f)|0x80);P0=~0x04;P2&=0x1f;
				}	
			}break;
			case 4:display[1]=smg_duan[4];break;
		}

		if(mode_index==4)
		{
			display[4]=0x00;
			display[5]=smg_duan[read_temp/10];
			display[6]=smg_duan[read_temp%10];
			display[7]=0x39;
		}
		else
		{
			display[4]=smg_duan[0];
			display[5]=smg_duan[time_second_set/100];
			display[6]=smg_duan[time_second_set/10%10];
			display[7]=smg_duan[time_second_set%10];
		}
	
		if(key_flag)
		{
			key_flag=0;
			
			setkeyboard(0x0f);
			read_key_data=getkeyboard();
			setkeyboard(0xf0);
			read_key_data=(read_key_data|getkeyboard())^0xff;
			trg=read_key_data&(read_key_data^cont);
			cont=read_key_data;
			switch(trg)
			{
				case 0x81:		
				{
					mode_index_flag=~mode_index_flag;
					if(mode_index_flag)
					{
						mode_index=4;
					}
					else
					{
						mode_index=1;
					}
				};break;//s7
				case 0x82:		
				{
					time_second_set=0;
					P2=((P2&0x1f)|0x80);P0=~0x00;P2&=0x1f;
					P34=0;
				};break;//s6

				case 0x84:						
				{
					time_second_set+=60;
					if(time_second_set>120)time_second_set=0;
				}break;				//s5
				case 0x88:				
				{
					mode_index++;
					if(mode_index>=4)mode_index=1;
				}break;			//s4
			}
			
		}
	}
}