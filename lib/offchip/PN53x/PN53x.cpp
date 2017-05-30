/**
*@file PN53x.cpp
*@author Neucrack
*@version v0.1
*@brief RFID reader PN53x driver
*@date create:2017.5.29
*      update:2017.5.29
*
*/

#include "PN53x.h"
#include "TaskManager.h"
#include "string.h"

PN53x::PN53x(USART& com)
:mUsart(com)
{

}

bool PN53x::PcdWakeUp(void)
{
    u8 data[24];
//55 55 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF 03 FD D4 14 01 17 00
		data[0]=0x55;
		data[1]=0x55;
		data[2]=0x00;
		data[3]=0x00;
		data[4]=0x00;
		data[5]=0x00;
		data[6]=0x00;
		data[7]=0x00;
		data[8]=0x00;
		data[9]=0x00;
		data[10]=0x00;
		data[11]=0x00;
		data[12]=0x00;
		data[13]=0x00;
		data[14]=0x00;
		data[15]=0x00;
		
		data[16]=0xFF;
		data[17]=0x03; //pack length
		data[18]=0xFD; //pack length parity 0x100-data[3]
		data[19]=0xD4; //cmd flag
		data[20]=0x14; //cmd flag
		data[21]=0x01;
		data[22]=0x17; //data parity   0x100-
		data[23]=0x00;
		
		mUsart.ClearReceiveBuffer();
		mUsart.SendData(data,24);
		float time = TaskManager::Time();
    while(1)
		{
			
			if(mUsart.ReceiveBufferSize() >= 15)//00 00 FF 00 FF 00 00 00 FF 02 FE D5 15 16 00    
			{
				mUsart.GetReceivedData(data,15);
				if(0x100 - data[11]-data[12] == data[13])
					return true;
				else return false;
			}
			if(TaskManager::Time()-time>MAX_WAIT_TIME_OUT)
				return false;
		}
}



//config 106 kbps type A (ISO/IEC14443 Type A),
//00 00 FF 04 FC D4 4A 01 00 E1 00
/**
  *@param sak: size
  *@param ats: 
  *
  */
bool  PN53x::PcdListPassiveTarget(uint16_t* type, uint8_t* uid,uint8_t* sak,uint8_t* ats)
{
    u8 data[50];
		data[0]=0x00;
		data[1]=0x00;
		data[2]=0xFF;
		data[3]=0x04; //pack length
		data[4]=0xFC; //pack length parity 0x100-data[3]
		data[5]=0xD4; //...
		data[6]=0x4A; //...
		data[7]=0x01;
		data[8]=0x00;
		data[9]=0xE1; //...   0x100-
		data[10]=0x00;
    mUsart.ClearReceiveBuffer();
		mUsart.SendData(data,11);
		float time = TaskManager::Time();
		bool flag=false;
		uint8_t length=0;
//00 00 FF 00 FF 00   00 00 FF 1C   E4  (  D5 4B 01 01     00 04   28       04       B6 29 AE BD     10  78 80 A0 02 20 90 00 00 00 00 00 B6 29 AE BD     ） C0 00 
//00 00 FF 00 FF 00   00 00 FF 0C   F4  (  D5 4B 01 01     00 04   08       04       B0 20 34 52   )  78 00 
//00 00 FF 00 FF 00   00 00 FF len  ..    D5 4B 01 01     type    size     UID len   UID            ats  parity=0x100-(sum)
    while(1)
		{
			if(!flag)
			{
				if(mUsart.ReceiveBufferSize() >= 11)//00 00 FF 00 FF 00 00 00 FF 02 FE D5 15 16 00    
				{
					mUsart.GetReceivedData(data,11);
					if(! (data[0]==0&&data[1]==0&&data[2]==0xff&&data[3]==0&&data[4]==0xff&&data[5]==0) )
						return false;
					length = data[9];
					flag = true;
				}
			}
			else
			{
				if(mUsart.ReceiveBufferSize()>=length+2)
				{
					mUsart.GetReceivedData(data,length+2);
					uint8_t sum=0;
					for(uint8_t i=0;i<length;++i)
						sum+=data[i];
					if(0x100-sum != data[length])
						return false;
					if(type)
						*type = (uint16_t)data[4]<<8|data[5];
					if(uid)
						memcpy(uid,data+8,4);
					if(sak)
						*sak = data[6];
					if(ats)
						memcpy(sak,data+13,data[12]-1);
					return true;
				}
			}
			if(TaskManager::Time()-time>MAX_WAIT_TIME_OUT)
				return false;
		}
}



