#include "JY901.h"

struct STime stcTime;
struct SAcc stcAcc;
struct SGyro stcGyro;
struct SAngle stcAngle;
struct SMag stcMag;
struct SDStatus stcDStatus;
struct SPress stcPress;
struct SLonLat stcLonLat;
struct SGPSV stcGPSV;
struct SQ stcQ;

//CopeSerialDataΪ����2�жϵ��ú���������ÿ�յ�һ�����ݣ�����һ�����������
void AnalyzeGyroData(unsigned char ucData)
{
	/*static unsigned char ucRxBuffer[250];
	static unsigned char ucRxCnt = 0;

	ucRxBuffer[ucRxCnt++] = ucData; //���յ������ݴ��뻺������
	if (ucRxBuffer[0] != 0x55)		//����ͷ���ԣ������¿�ʼѰ��0x55����ͷ
	{
		ucRxCnt = 0;
		return;
	}
	if (ucRxCnt < 11)
	{
		return;
	} //���ݲ���11�����򷵻�
	else
	{
		switch (ucRxBuffer[1]) //�ж��������������ݣ�Ȼ���俽������Ӧ�Ľṹ���У���Щ���ݰ���Ҫͨ����λ���򿪶�Ӧ������󣬲��ܽ��յ�������ݰ�������
		{
		case 0x50:
			memcpy(&stcTime, &ucRxBuffer[2], 8);
			break; //memcpyΪ�������Դ����ڴ濽��������������"string.h"�������ջ��������ַ����������ݽṹ�����棬�Ӷ�ʵ�����ݵĽ�����
		case 0x51:
			memcpy(&stcAcc, &ucRxBuffer[2], 8);
			break;
		case 0x52:
			memcpy(&stcGyro, &ucRxBuffer[2], 8);
			break;
		case 0x53:
			memcpy(&stcAngle, &ucRxBuffer[2], 8);
			break;
		case 0x54:
			memcpy(&stcMag, &ucRxBuffer[2], 8);
			break;
		case 0x55:
			memcpy(&stcDStatus, &ucRxBuffer[2], 8);
			break;
		case 0x56:
			memcpy(&stcPress, &ucRxBuffer[2], 8);
			break;
		case 0x57:
			memcpy(&stcLonLat, &ucRxBuffer[2], 8);
			break;
		case 0x58:
			memcpy(&stcGPSV, &ucRxBuffer[2], 8);
			break;
		case 0x59:
			memcpy(&stcQ, &ucRxBuffer[2], 8);
			break;
		}
		ucRxCnt = 0; //��ջ�����
	}*/
}

void PrintfGyroData(void)
{
	char str[100];
	//���ʱ��
	sprintf(str, "Time:20%d-%d-%d %d:%d:%.3f\r\n", stcTime.ucYear, stcTime.ucMonth, stcTime.ucDay, stcTime.ucHour, stcTime.ucMinute, (float)stcTime.ucSecond + (float)stcTime.usMiliSecond / 1000);
	printf(str);
	delay_ms(10);
	//������ٶ�
	sprintf(str, "Acc:%.3f %.3f %.3f\r\n", (float)stcAcc.a[0] / 32768 * 16, (float)stcAcc.a[1] / 32768 * 16, (float)stcAcc.a[2] / 32768 * 16);
	printf(str);
	delay_ms(10);
	//������ٶ�
	sprintf(str, "Gyro:%.3f %.3f %.3f\r\n", (float)stcGyro.w[0] / 32768 * 2000, (float)stcGyro.w[1] / 32768 * 2000, (float)stcGyro.w[2] / 32768 * 2000);
	printf(str);
	delay_ms(10);
	//����Ƕ�
	sprintf(str, "Angle:%.3f %.3f %.3f\r\n", (float)stcAngle.Angle[0] / 32768 * 180, (float)stcAngle.Angle[1] / 32768 * 180, (float)stcAngle.Angle[2] / 32768 * 180);
	printf(str);
	delay_ms(10);
	//����ų�
	sprintf(str, "Mag:%d %d %d\r\n", stcMag.h[0], stcMag.h[1], stcMag.h[2]);
	printf(str);
	delay_ms(10);
	//�����ѹ���߶�
	sprintf(str, "Pressure:%ld Height%.2f\r\n", stcPress.lPressure, (float)stcPress.lAltitude / 100);
	printf(str);
	delay_ms(10);
	//����˿�״̬
	sprintf(str, "DStatus:%d %d %d %d\r\n", stcDStatus.sDStatus[0], stcDStatus.sDStatus[1], stcDStatus.sDStatus[2], stcDStatus.sDStatus[3]);
	printf(str);
	delay_ms(10);
	//�����γ��
	sprintf(str, "Longitude:%ldDeg%.5fm Lattitude:%ldDeg%.5fm\r\n", stcLonLat.lLon / 10000000, (double)(stcLonLat.lLon % 10000000) / 1e5, stcLonLat.lLat / 10000000, (double)(stcLonLat.lLat % 10000000) / 1e5);
	printf(str);
	delay_ms(10);
	//�������
	sprintf(str, "GPSHeight:%.1fm GPSYaw:%.1fDeg GPSV:%.3fkm/h\r\n", (float)stcGPSV.sGPSHeight / 10, (float)stcGPSV.sGPSYaw / 10, (float)stcGPSV.lGPSVelocity / 1000);
	printf(str);
	delay_ms(10);
	//�����Ԫ��
	sprintf(str, "Four elements:%.5f %.5f %.5f %.5f\r\n\r\n", (float)stcQ.q[0] / 32768, (float)stcQ.q[1] / 32768, (float)stcQ.q[2] / 32768, (float)stcQ.q[3] / 32768);
	printf(str);
	delay_ms(10); //�ȴ��������
}