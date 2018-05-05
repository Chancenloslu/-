#include "stc15f2k60s2.h"
#include "ds1302.h"
#include "onewire.h"

#define keyport P3
#define setkeyboard(x) P4=((x>>7)<<4)|(((x>>6)&0x01)<<2);P3=x
#define getkeyboard() ((P4&0x10)<<3)|((P4&0x40)<<4)|(P3&0x3f)

sbit buzzer=P0^6;
sbit L1=P0^7;

unsigned char smg_count,key_count,shi,fen,miao,jm_mode,temp_num,display_count,real_time_interval;
unsigned int capture_count,prompt_count,L1_count;

unsigned char smg_com[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
unsigned char smg_duan[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};				//0-9
unsigned char display[8];
unsigned char temp_capture[10];

bit L1_flag=0,blink_flag=0,key_flag=0,capture_flag,display_flag;
unsigned char time_interval[4]={1,5,30,60};
unsigned char time_interval_index;

void Read_time(void);

////void TIMER1_1ms() interrupt 3 using 2
////{
////	

////	if(L1_flag)
////	{
////		if(L1_count++==1000)
////		{
////			P2=0x80;P0^=0x80;P2=0x00;
////		}
////	}
////}

void TIMER0_1ms() interrupt 1 using 1
{
	static char i=0;key_count++;smg_count++;prompt_count++;
	if(key_count==10)
	{
		key_flag=1;
		key_count=0;
	}

	if(smg_count==3)
	{
		smg_count=0;
		P2=0xe0;P0=0;P2=0x00;
		P2=0xc0;P0=smg_com[i];P2=0x00;
//		P2=0xc0;P0=~display[i];P2=0x00;
		P2=0xe0;P0=~display[i];P2=0x00;
		if(i++==8)i=0;
	}

	if(prompt_count==1000)		//1S
	{
		prompt_count=0;
		blink_flag=~blink_flag;

	} 
	
	if(jm_mode==1)
	{
		capture_count++;
		if(capture_count==real_time_interval*1000)
		{
			capture_count=0;
			capture_flag=1;
			
		}
	}
}

void main()
{
	unsigned char trg,cont,readdata;
	AUXR |= 0x80;		//?????1T??
//	AUXR &= 0x7f; //12T
	TMOD &= 0xf0;		//???????
	TL0 = 0xCD;		//??????
	TH0 = 0xD4;		//??????
	TF0 = 0;		//??TF0??
	TR0 = 1;		//???0????
	ET0 = 1;
	
//	TL1 = 0xCD;		//??????
//	TH1 = 0xD4;		//??????	
//	TF1 = 0;
//	TR1 = 1;
//	ET1 = 1;
	EA=1;
	
	P2=0xa0;buzzer=0;P2=0x00;
	set_sfm(23,59,50);
	while(1)
	{
		if(jm_mode==0)
		{
			display[0]=0x00;display[1]=0x00;display[2]=0x00;display[3]=0x00;
			display[4]=0x00;display[5]=0x40;display[6]=smg_duan[time_interval[time_interval_index]/10];display[7]=smg_duan[time_interval[time_interval_index]%10];
		}
		if(jm_mode==1)
		{
			Read_time();
			display[0]=smg_duan[shi/16];display[1]=smg_duan[shi%16];
//			display[2]=0x40;
			display[3]=smg_duan[fen/16];display[4]=smg_duan[fen%16];
//			display[5]=0x40;
			display[6]=smg_duan[miao/16];display[7]=smg_duan[miao%16];
			if(blink_flag)
			{
				display[2]=0x40;
				display[5]=0x40;
			}
			else
			{
				display[2]=0;
				display[5]=0;				
			}

			if(capture_flag)
			{
				capture_flag=0;
				if(temp_num<11)
				{
					ET0=0;
					temp_capture[temp_num]=(unsigned char)rd_temprature_f();
					ET0=1;
					temp_num++;
				}
				else
				{
					L1_flag=1;
					jm_mode=2;
				}
			}
		}
			if(jm_mode==2)
			{

				if(display_count==0)
				{
					display[0]=0x40;
					display[1]=smg_duan[0];display[2]=smg_duan[0];
					display[3]=0x00;display[4]=0x00;display[5]=0x40;
					display[6]=smg_duan[0];display[7]=smg_duan[0];	
				}
				else
				{
					display[0]=0x40;
					display[1]=smg_duan[display_count/10];display[2]=smg_duan[display_count%10];
					display[3]=0x00;display[4]=0x00;display[5]=0x40;
					display[6]=smg_duan[temp_capture[display_count]/10];display[7]=smg_duan[temp_capture[display_count]%10];	
												
				}
				if(L1_flag)
				{
					if(blink_flag)
					{
						P2=0x80;P0=0x7f;P2=0x00;
					}
					else
					{
						P2=0x80;P0=0xff;P2=0x00;
					}
				}
			}
//		if(capture_flag)
//		{
//			capture_flag=0;
//			temp_capture[temp_num]=rd_temprature_f();
//			if(temp_num++==10)
//			{
//				L1_flag=1;
//				temp_num=0;
//			}
//		}
		if(key_flag)
		{
			key_flag=0;
			setkeyboard(0x0f);
			readdata=getkeyboard();
			setkeyboard(0xf0);
			readdata=(readdata|getkeyboard())^0xff;
			trg=readdata&(readdata^cont);
			cont=readdata;
			if(trg==0x81)		//S4
			{
				if(jm_mode==0)
				{
					jm_mode=0;
					time_interval_index++;
					if(time_interval_index==4)time_interval_index=0;
				}
			}
			if(trg==0x82)		//S5
			{
				if(jm_mode==0)
				{
					jm_mode=1;
					real_time_interval=time_interval[time_interval_index];
				}
			}
			if(trg==0x84)		//S6
			{
				L1_flag=0;
				P2=0x80;P0=0xff;P2=0x00;
				display_count++;
				if(display_count==11)display_count=0;
			}
			if(trg==0x88)		//S7
			{
				if(jm_mode==2)
				{
					jm_mode=0;
					temp_num=0;
				}
			}
		}
	}
}

void Read_time(void)
{
	miao=Ds1302_Single_Byte_Read(0x81);
	fen=Ds1302_Single_Byte_Read(0x83);
	shi=Ds1302_Single_Byte_Read(0x85);
}