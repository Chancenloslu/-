#include "stc15f2k60s2.h"
#include "onewire.h"

#define setkeyboard(x) P4=((x&0x80)>>3)|((x&0x40)>>4);P3=x
#define getkeyboard() ((P4&0x10)<<3)|((P4&0x04)<<4)|(P3&0x3f)


bit key_flag,set_flag,blink_flag,error_flag;
unsigned int L1_count;
unsigned char T_min,T_max,real_temprature,blink_value;
unsigned char set_count;set_temp_value;set_temp_index;
unsigned char display[8];set_temp[5]={0};
unsigned char smg_wei[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
unsigned char smg_duan[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
												//   0    1    2    3    4    5    6    7    8    9  
sbit buzzer=P0^6;
sbit relay=P0^4;
//-----------------------------------------------

/* Timer1 interrupt routine */
void tm1_isr() interrupt 3 using 2
{
	static unsigned char key_count,smg_count,i;
	key_count++;smg_count++;
	
	if(key_count==10)
	{
		key_count=0;
		key_flag=1;
	}
	if(smg_count==1)
	{
		smg_count=0;
		
		P2=0xc0;P0=0;P2=0;
		P2=0xe0;P0=~display[i];P2=0;
		P2=0xc0;P0=smg_wei[i];P2=0;
		i++;
		if(i==8)i=0;
	}
}

//-----------------------------------------------
//-----------------------------------------------

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
	L1_count++;

	if(real_temprature<T_min)
	{
		if(L1_count>=800)
		{
			blink_flag=~blink_flag;
			if(blink_flag)
			{
				P2=0x80;P0=~0x01;P2=0x00;
			}
			else
			{
				P2=0x80;P0=0xff;P2=0x00;
			}
			L1_count=0;
		}
		
	}
	else if(real_temprature<=T_max)
	{
		if(L1_count>=400)
		{

			blink_flag=~blink_flag;
			if(blink_flag)
			{
				P2=0x80;P0=~0x01;P2=0x00;
			}
			else
			{
				P2=0x80;P0=0xff;P2=0x00;
			}
			L1_count=0;
		}
	}
	else
	{
		if(L1_count>=200)
		{
			blink_flag=~blink_flag;
			if(blink_flag)
			{
				P2=0x80;P0=~0x01;P2=0x00;
			}
			else
			{
				P2=0x80;P0=0xff;P2=0x00;
			}
			L1_count=0;
		}
	}	
}

//-----------------------------------------------
void main()
{
	unsigned char read_key_data,trg,cont;
	
	AUXR |= 0xC0;		//?????1T??
	TMOD &= 0xFF;		//???????
	TL0 = 0xCD;		//??????
	TH0 = 0xD4;		//??????
	TF0 = 0;		//??TF0??
	TR0 = 1;		//???0????
	TL1 = 0xCD;		//??????
	TH1 = 0xD4;		//??????
	TF1 = 0;		//??TF1??
	TR1 = 1;		//???1????
	ET0 = 1;
	ET1 = 1;
	EA = 1;
	
	T_min=20;
	T_max=30;
	P2=0xa0;buzzer=0;P2=0x00;

	while(1)
	{
		display[0]=0x40;display[3]=display[4]=0x00;
		if(set_temp_index>4)set_temp_index=1;
		
		
		
		if(key_flag)
		{
			key_flag=0;
			
			setkeyboard(0x0f);
			read_key_data=getkeyboard();
			setkeyboard(0xf0);
			read_key_data=(read_key_data|getkeyboard())^0xff;
			trg=read_key_data&(read_key_data^cont);
			cont=read_key_data;
			if(set_flag)
			{
				switch(trg)
				{
						case 0x88:set_temp_value=9;set_temp_index++;break;		//s4
						case 0x84:set_temp_value=6;set_temp_index++;break;		//s5
						case 0x82:set_temp_value=3;set_temp_index++;break;		//s6
						case 0x81:set_temp_value=0;set_temp_index++;break;		//s7
						
						case 0x44:set_temp_value=7;set_temp_index++;break;		//s9
						case 0x42:set_temp_value=4;set_temp_index++;break;		//s10
						case 0x41:set_temp_value=1;set_temp_index++;break;		//s11
						case 0x28:set_temp[0]=0;set_temp[1]=0;set_temp[2]=0;set_temp[3]=0;set_temp[4]=0;set_temp_index=0;break;		//s12
						case 0x24:set_temp_value=8;set_temp_index++;break;		//s13
						case 0x22:set_temp_value=5;set_temp_index++;break;		//s14
						case 0x21:set_temp_value=2;set_temp_index++;break;		///s15
					
						
	//				case 0x18:P2=0xa0;P04=0;P2=0;break;		//s16
	//				case 0x14:P2=0xa0;P04=1;P2=0;break;		//s17
	//				case 0x12:P2=0x80;P0=0x55;P2=0;break;		//s18
	//				case 0x11:P2=0x80;P0=0xaa;P2=0;break;				//s19
				}
			}	
			switch(trg)
			{
				case 0x48:set_flag=1;set_count++;break;		//s8
			}
		}
		
		
		
		if(error_flag==0)
		{
			if(set_flag==0)			//mode 1
			{
				display[5]=0x00;display[2]=0x40;
				real_temprature=(unsigned char)rd_temprature_f();
				display[6]=smg_duan[real_temprature/10];
				display[7]=smg_duan[real_temprature%10];
				if(real_temprature<T_min)
				{
					display[1]=smg_duan[0];
					P2=0xa0;buzzer=0;relay=0;P2=0x00;
					
				}
				else if(real_temprature<=T_max)
				{
					display[1]=smg_duan[1];
					P2=0xa0;buzzer=0;relay=0;P2=0x00;
				}
				else
				{
					display[1]=smg_duan[2];
					P2=0xa0;buzzer=0;relay=1;P2=0x00;
				}
			}
		}
		if(set_flag)				//mode 2
		{
			display[1]=smg_duan[set_temp[1]];
			display[2]=smg_duan[set_temp[2]];
			display[5]=0x40;
			display[6]=smg_duan[set_temp[3]];
			display[7]=smg_duan[set_temp[4]];
			if(set_count==1)
			{
				set_temp[set_temp_index]=set_temp_value;
				
			}
			if(set_count==2)
			{
				T_min=set_temp[3]*10+set_temp[4];
				T_max=set_temp[1]*10+set_temp[2];
				if(T_max<T_min)
				{
					ET0=0;
					error_flag=1;
					P2=0x80;P0|=0xff;P01=0;P2=0;
				}
				else
				{
					ET0=1;
					error_flag=0;
					P2=0x80;P0|=0xff;P2=0;
					set_count=0;
					set_flag=0;
				}
			}
		}
		
	}
}