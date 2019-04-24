#include "remote.h"

/*����ļ��ǻ�ȡң�������ݵĽӿ�*/

const int16_t Pulse_Max = 1478, Pulse_Mid = 1024, Pulse_Min = 568; 

Remote_t Remote;

int8_t RemoteData[7];

void RemoteInit(void)
{
	Remote.LX = 0;
	Remote.LY = 0;
	Remote.RX = 0;
	Remote.RY = 0;
	Remote.LS = 0;
	Remote.RS = 0;
	Remote.VA = 0;
}

void UpdateRemoteInfo(void* PWM_Array)
{
#if RCV_SIG == UPWM_IN
	Remote.RX = ((int16_t*)PWM_Array)[0];
	Remote.RY = ((int16_t*)PWM_Array)[2];
	Remote.LY = ((int16_t*)PWM_Array)[1];
	Remote.LX = ((int16_t*)PWM_Array)[3];
	Remote.LS = ((int16_t*)PWM_Array)[7];
	Remote.RS = ((int16_t*)PWM_Array)[8];
	Remote.VA = ((int16_t*)PWM_Array)[4];
	Remote.VB = ((int16_t*)PWM_Array)[5];
	Remote.VC = ((int16_t*)PWM_Array)[6];
#else
	#if RCV_SIG == SBUS_IN
	/*Prevent Warning*/
	*((uint8_t*)PWM_Array) = *((uint8_t*)PWM_Array);
	
	SBUS_Decode();
	Remote.RX = SBUS_ChanelVal[0];
	Remote.RY = SBUS_ChanelVal[1];
	Remote.LY = SBUS_ChanelVal[2];
	Remote.LX = SBUS_ChanelVal[3];
	Remote.LS = SBUS_ChanelVal[4];
	Remote.VA = SBUS_ChanelVal[5];
	Remote.RS = SBUS_ChanelVal[6];
	#endif
#endif
}

/*�����߸������ǻ�ȡת�����ͨ�����ݵĺ���*/
int8_t Remote_GetLX(void)
{
	return Remote_PulseToVal(Remote_GetLX_Pulse());
}

int8_t Remote_GetLY(void)
{
	return Remote_PulseToVal(Remote_GetLY_Pulse());
}

int8_t Remote_GetRX(void)
{
	return Remote_PulseToVal(Remote_GetRX_Pulse());
}

int8_t Remote_GetRY(void)
{
	return Remote_PulseToVal(Remote_GetRY_Pulse());
}

uint8_t Remote_GetLS(void)
{
	uint16_t PulseWidth = Remote_GetLS_Pulse();
	if(PulseWidth == Pulse_Min)
		return CtrlMode_XYW;
	if(PulseWidth == Pulse_Mid)
		return CtrlMode_NOA;
	if(PulseWidth == Pulse_Max)
		return CtrlMode_RPY;
	return CtrlMode_Error;
}

uint8_t Remote_GetRS(void)
{
	uint16_t PulseWidth = Remote_GetRS_Pulse();
	if(PulseWidth == Pulse_Min)
		return MoveMode_Trot;
	if(PulseWidth == Pulse_Max)
		return MoveMode_Walk;
	if(PulseWidth == Pulse_Mid)
		return MoveMode_Stand;
	return MoveMode_Error;
}

uint8_t Remote_GetVA(void)
{
	return (Remote_GetVA_Pulse() - 340) / 6;
}

int8_t *Remote_Get(void)
{
	RemoteData[0] = Remote_GetLX();
	RemoteData[1] = Remote_GetLY();
	RemoteData[2] = Remote_GetRX();
	RemoteData[3] = Remote_GetRY();
	RemoteData[4] = Remote_GetLS();
	RemoteData[5] = Remote_GetRS();
	RemoteData[6] = Remote_GetVA();
	return RemoteData;
}

/*�����߸�����Ϊ��ȡ���ջ���ԭʼ�������*/

uint16_t Remote_GetLX_Pulse(void)
{
	return *((volatile uint16_t*)&Remote.LX);
}

uint16_t Remote_GetLY_Pulse(void)
{
	return *((volatile uint16_t*)&Remote.LY);
}

uint16_t Remote_GetRX_Pulse(void)
{
	return *((volatile uint16_t*)&Remote.RX);
}

uint16_t Remote_GetRY_Pulse(void)
{
	return *((volatile uint16_t*)&Remote.RY);
}

uint16_t Remote_GetLS_Pulse(void)
{
	return *((volatile uint16_t*)&Remote.LS);
}

uint16_t Remote_GetRS_Pulse(void)
{
	return *((volatile uint16_t*)&Remote.RS);
}

uint16_t Remote_GetVA_Pulse(void)
{
	return *((volatile uint16_t*)&Remote.VA);
}

void RemoteAnalyzeTask(void)
{
#if RCV_SIG == UPWM_IN
	static uint8_t ch;
	static uint8_t RawData[22] = {0};
	static uint8_t RxState = 0, RxDataIndex = 0;
	if(USART_GetFlagStatus(USART2, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		ch = USART_ReceiveData(USART2);
		switch(RxState)
		{
			case 0x00:				//��δ��⵽��ʼ��־����ʼƥ����ʼ��־��һ�ֽ�
				if(ch == 0x55)
					RxState++;
				else
					RxState = 0;
				break;
			case 0x01:				//��ʼ��־��һ�ֽ���ƥ�䣬��ʼƥ����ʼ��־�ڶ��ֽ�
				if(ch == 0xAA)
				{
					RxDataIndex = 0;
					RxState++;
				}
				else
					if(ch == 0x55);
					else
						RxState = 0;
				break;
			case 2:					//��ʼ��־�Ѿ�ƥ�䣬��ʼ����ԭʼ����
				RawData[RxDataIndex] = ch;
				RxDataIndex++;
				if(RxDataIndex >= 18)
				{
					RxDataIndex = 0;
					RxState++;
				}
				break;
			case 3:					//�����Ѿ�������ϣ���ʼƥ�������־��һ�ֽ�
				if(ch == 0xAA)
					RxState++;
				else
					RxState = 0;
				break;
			case 4:					//������־��һ�ֽ���ƥ�䣬��ʼƥ�������־�ڶ��ֽ�
				if(ch == 0x55)
				{
					UpdateRemoteInfo((void*)&RawData[0]);
					RemoteUpdated = 30;
				}
				RxState = 0;
				break;
			default:
				RxState = 0;
		}
	}
#else
	#if RCV_SIG == SBUS_IN
	static uint8_t ch;
	static uint8_t RxState = 0, RxDataIndex = 0;
	if(USART_GetFlagStatus(UART4, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(UART4, USART_IT_RXNE);
		ch = USART_ReceiveData(UART4);
		switch(RxState)
		{
			case 0:				//��δ��⵽��ʼ��־����ʼƥ����ʼ��־
				if(ch == 0x0F)
				{
					RxState++;
					SBUS_MsgPack[0] = ch;
					RxDataIndex = 1;
				}
				else
					RxState = 0;
				break;
			case 1:					//��ʼ��־ƥ�䣬��ʼ����ԭʼ����
				SBUS_MsgPack[RxDataIndex] = ch;
				RxDataIndex++;
				if(RxDataIndex >= 23)
				{
					RxDataIndex = 0;
					RxState++;
				}
				break;
			case 2:					//�����Ѿ�������ϣ���ʼƥ�������־��һ�ֽ�
					SBUS_MsgPack[23] = ch;
					RxState++;
				break;
			case 3:					//������־��һ�ֽ���ƥ�䣬��ʼƥ�������־�ڶ��ֽ�
				if(ch == 0x00)
				{
					UpdateRemoteInfo((void*)&SBUS_MsgPack[0]);
					SBUS_MsgPack[24] = 0x00;
				}
				RxState = 0;
				break;
			default:
				RxState = 0;
		}
	}
	#endif
#endif
}

#if RCV_SIG == SBUS_IN
#else
	#if RCV_SIG == UPWM_IN
const uint16_t MidPulse = 1520;
const uint16_t MidError = 20;
	#endif
#endif

/*
 *	����Ҫ��ԭʼ����ת��int8_t
 *	���ﻹ�����Ż�����Ϊ����SBUS��ԭʼ����δ���������Ļ���
 *	����һ������ 568 - 1478 �ڣ����ԣ�456/12t����������int8_t
*	�������ǵ�һ�㲻�����ܵ�����ٶȣ�Ҳ���Բ����Ա���������У׼�����������
 */
int8_t Remote_PulseToVal(uint16_t PulseWidth)
{
	return (PulseWidth - Pulse_Mid) / 6;
}
