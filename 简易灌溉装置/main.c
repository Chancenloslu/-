#include "stc15f2k60s2.h"
#include "ds1302.h"
#include "iic.h"

#define setkeyboard(x) P4=((x&0x80)>>3)|((x&0x40)>>4);P3=x
#define getkeyboard() ((P4&0x10)<<3)|((P4&0x04)<<4)|(P3&0x3f)

sbit buzzer=P0^6;
sbit relay=P0^4;
bit key_flag,wet_threshold_flag,buzzer_flag,work_state_flag,relay_flag;
float real_wet_value;
unsigned char wet_degree,wet_threshold,read_Eeprom,shi,fen,miao;
unsigned char read_key_value,trg,cont;
unsigned char smg_com[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
unsigned char smg_duan[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
												//   0    1    2    3    4    5    6    7    8    9  
unsigned char display[8];
void display1(void);
void display2(void);

void Read_time()
{
	shi=Read_Ds1302(0x85);
	fen=Read_Ds1302(0x83);
	miao=Read_Ds1302(0x81);
}

void display1(void)
{
		display[0]=0x40;
		display[1]=0x40;
		display[2]=0x00;
		display[3]=smg_duan[read_Eeprom/10];
		display[4]=smg_duan[read_Eeprom%10];
		display[5]=0x00;
		display[6]=smg_duan[wet_threshold/10];
		display[7]=smg_duan[wet_threshold%10];			
}
void display2(void)
	{
		Write_AT24C02(0x01,wet_threshold);
		display[0]=smg_duan[fen/16];
		display[1]=smg_duan[fen%16];
		display[2]=0x40;
		display[3]=smg_duan[miao/16];
		display[4]=smg_duan[miao%16];
		display[5]=0x00;
		display[6]=smg_duan[(unsigned char)real_wet_value/10];
		display[7]=smg_duan[(unsigned char)real_wet_value%10];		

}
//-----------------------------------------------


//-----------------------------------------------

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
  static unsigned char key_count;
	static unsigned char smg_count,i;
	key_count++;
	if(key_count==100)
	{
		key_count=0;
		key_flag=1;
	}

	smg_count++;
	if(smg_count==1)
	{
		smg_count=0;
		P2=0xc0;P0=0;P2=0;
		P2=0xe0;P0=~display[i];P2=0;
		P2=0xc0;P0=smg_com[i];P2=0;
		i++;
		if(i==8)i=0;
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
	
	set_sfm(8,30,0);
	P2=0xa0;buzzer=0;relay=0;P2=0;

	wet_threshold=50;
	
	while(1)
	{
		ET0=0;
		Read_time();
		read_Eeprom=Read_at24c02(0x01);
		real_wet_value=Read_adc(0x03)/255.0*99;
		ET0=1;

//		if(work_state==2)		//工作状态
//		{
//			work_state=0;
//		}
		if(wet_threshold_flag)
		{
			display1();
		}
		else
		{
			display2();
		}
		if(work_state_flag==0)		//自动  L1
		{
			P2=0x80;P0=~0x01;P2=0;
			if(real_wet_value<wet_threshold){	P2=0xa0;relay=1;buzzer=0;P2=0;}
			else {P2=0xa0;relay=0;buzzer=0;P2=0;}
		}
		else								//手动		L2
		{
			P2=0x80;P0=~0x02;P2=0;
//			P2=0xa0;relay=0;P2=0;
			if(relay_flag)
			{
				P2=0xa0;relay=1;P2=0x00;
			}
			else
			{
				P2=0xa0;relay=0;P2=0x00;
			}
			if(buzzer_flag)
			{
				if(real_wet_value<wet_threshold){P2=0xa0;buzzer=1;P2=0;}
				else {P2=0xa0;buzzer=0;P2=0;}
			}
			else
			{
				P2=0xa0;buzzer=0;P2=0;
			}
		}
		
		if(key_flag)
		{
			key_flag=0;
			
			setkeyboard(0x0f);
			read_key_value=getkeyboard();
			setkeyboard(0xf0);
			read_key_value=(read_key_value|getkeyboard())^0xff;
			trg = read_key_value&(read_key_value^cont);
			cont = read_key_value;
			
			switch(trg)
			{
				case 0x81:work_state_flag=~work_state_flag;break;			//s7
			}
			if(work_state_flag==0)				//自动模式
			{
				switch(trg)
				{
					case 0x88:wet_threshold--;if(wet_threshold==0xff)wet_threshold=99;break;			//s4		
					case 0x84:wet_threshold++;if(wet_threshold==100)wet_threshold=0;break;			//s5
					case 0x82:wet_threshold_flag=~wet_threshold_flag;break;			//s6
					
					
				}
			}
			if(work_state_flag==1)				//手动模式
			{
				switch(trg)
				{
					case 0x88:relay_flag=0;break;			//s4		
					case 0x84:relay_flag=1;break;			//s5
					case 0x82:buzzer_flag=~buzzer_flag;break;			//s6
					
				}
			}			
		}
	}
}