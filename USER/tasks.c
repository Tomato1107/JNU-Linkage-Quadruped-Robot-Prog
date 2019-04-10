#include "tasks.h"

void InitTask(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);
	USART1_Init(115200);
	USART3_Init(115200);
	UART4_Init(100000);

	LED_Init();
	BEEP_Init();
	KEY_Init();
	LCD_Init();
	TIM3_Init();
	SynSig_Init();

	CAN1_Init();
	ActrDevInit();

	ShowModeSelection(-1);
}

void UploadTask(void)
{
	UploadRemoteData(Remote_Get(),7);//FIXME:
}

void ControlTask(void)
{
	extern float actrAngle[12];

	int i = 0;
	if (InvertActrRawData())
	{
		/*for (i = 0; i < 12; i++)
		{
			printf("%.4f ", actrAngle[i]);
		}
		printf("\r\n");*/
		if (CheckActrTargetPosVal())
		{
			for (i = 0; i < 12; i++)
			{
				//if(devIDList[i]==125)//||devIDList[i]==125)
				//printf("Angle:%.2f,ID:%d\r\n",actrAngle[i], devIDList[i]);
				SetActrPosition(actrAngle[i], devIDList[i]);
			}
		}
		else
		{
			//BEEP_Alert(1);
		}
	}
}

/**
 * ��������: DisplayTask
 * ��   ��:	ִ����Ҫ�ظ�ˢ�µĵ����ȼ���ʾ����
 * ��ڲ���: 
 * ���ڲ���: 
 * ��   ע:
 */
void DisplayTask(void)
{
	ShowInfo();
	switch (InterfaceIndex)
	{
	case ROBOT_INTERFACE_MODE:
		break;
	case ROBOT_INTERFACE_ACTR_PWR:
		break;
	case ROBOT_INTERFACE_HOMING_CHECK:
		ShowActrHomingPosVal();
		break;
	case ROBOT_INTERFACE_ACTR_POS:
		ShowActrPosVal();
		break;
	case ROBOT_INTERFACE_ACTR_PARA:
		ShowActrAllPara();
		break;
	case ROBOT_INTERFACE_DEV_PARA:
		ShowDevPara();
		break;
	}
}

int ReportTaskFlag;

/**
 * ��������: ReportTask
 * ��   ��:	��ȡִ��������
 * ��ڲ���: 
 * ���ڲ���: 
 * ��   ע:
 */
void ReportTask(TaskFlagTypedef taskflag)
{
	for (int i = 0; i < 12; i++)
	{
		if (Can1BusyCheck() == CAN_BUS_STATE_FREE)
		{
			switch (taskflag)
			{
			case TASK_FLAG_REPORT_POS:
				GetActrPara(ACTR_CMD_GET_POSTION, devIDList[i]);
				break;
			case TASK_FLAG_REPORT_VEL:
				GetActrPara(ACTR_CMD_GET_SPEED, devIDList[i]);
				break;
			case TASK_FLAG_REPORT_CUR:
				GetActrPara(ACTR_CMD_GET_CURRENT, devIDList[i]);
				break;
			case TASK_FLAG_REPORT_EXECPTION:
				GetActrPara(ACTR_CMD_GET_EXECPTION, devIDList[i]);
				break;
			case TASK_FLAG_REPORT_ALL:
				GetActrPara(ACTR_CMD_GET_ON_OFF, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_CUR_MODE, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_CURRENT, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_SPEED, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_POSTION, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_EXECPTION, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_TSHAP_POS_MAX_SPEED, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_TSHAP_POS_ACCELERATE, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_TSHAP_POS_DECELERATE, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_POSTION_LOWER_LIMIT, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_POSTION_UPPER_LIMIT, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_SPEED_OUTPUT_LOWER_LIMIT, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_SPEED_OUTPUT_UPPER_LIMIT, devIDList[i]);
				GetActrPara(ACTR_CMD_GET_SHUTDOWN_STATE, devIDList[i]);
				break;
			default:
				break;
			}
		}
	}
}

unsigned int LowPirorityTaskFlag = 0;
//	[0]	:REPORT_FLAG
//	[1]	:KEY_FLAG
//	[31-2]:	IDLE

/**
 * ��������: HandleLowPirorityTask
 * ��   ��:	���ݱ�־λ��������ȼ�����
 * ��ڲ���: 
 * ���ڲ���: 
 * ��   ע:
 */
void HandleLowPirorityTask(void)
{
	DisplayTask();
	KeyTask();

	if (TASK_SEARCH_FLAG(TASK_FLAG_REPORT_POS))
	{
		TASK_RESET_FLAG(TASK_FLAG_REPORT_POS);
		ReportTask(TASK_FLAG_REPORT_POS);
	}
	if (TASK_SEARCH_FLAG(TASK_FLAG_REPORT_VEL))
	{
		TASK_RESET_FLAG(TASK_FLAG_REPORT_VEL);
		ReportTask(TASK_FLAG_REPORT_VEL);
	}
	if (TASK_SEARCH_FLAG(TASK_FLAG_REPORT_CUR))
	{
		TASK_RESET_FLAG(TASK_FLAG_REPORT_CUR);
		ReportTask(TASK_FLAG_REPORT_CUR);
	}
	if (TASK_SEARCH_FLAG(TASK_FLAG_REPORT_EXECPTION))
	{
		TASK_RESET_FLAG(TASK_FLAG_REPORT_EXECPTION);
		ReportTask(TASK_FLAG_REPORT_EXECPTION);
	}
	if (TASK_SEARCH_FLAG(TASK_FLAG_REPORT_ALL))
	{
		TASK_RESET_FLAG(TASK_FLAG_REPORT_ALL);
		ReportTask(TASK_FLAG_REPORT_ALL);
	}
	if (TASK_SEARCH_FLAG(TASK_FLAG_UPLOAD))
	{
		TASK_RESET_FLAG(TASK_FLAG_UPLOAD);
		UploadTask();
	}	
	if (TASK_SEARCH_FLAG(TASK_FLAG_CONTROL))
	{
		TASK_RESET_FLAG(TASK_FLAG_CONTROL);
		ControlTask();
	}	
	
}

/**
 * ��������: KeyTask
 * ��   ��:	���ݵ�ǰ�����жϰ������ܣ���ִ������ˢ�µĵ����ȼ���ʾ����
 * ��ڲ���: 
 * ���ڲ���: 
 * ��   ע:
 */
void KeyTask(void)
{
	uint8_t KeyVal;
	KeyVal = KEY_Scan(0);

	void SubKeyTask_MODE(uint8_t KeyVal);
	void SubKeyTask_ACTR_PWR(uint8_t KeyVal);
	void SubKeyTask_ACTR_POS(uint8_t KeyVal);
	void SubKeyTask_HOMING_CHECK(uint8_t KeyVal);
	void SubKeyTask_ACTR_PARA(uint8_t KeyVal);
	void SubKeyTask_DEV_PARA(uint8_t KeyVal);

	switch (InterfaceIndex)
	{
	case ROBOT_INTERFACE_MODE:
		SubKeyTask_MODE(KeyVal);
		break;
	case ROBOT_INTERFACE_ACTR_PWR:
		SubKeyTask_ACTR_PWR(KeyVal);
		break;
	case ROBOT_INTERFACE_HOMING_CHECK:
		SubKeyTask_HOMING_CHECK(KeyVal);
		break;
	case ROBOT_INTERFACE_ACTR_POS:
		SubKeyTask_ACTR_POS(KeyVal);
		break;
	case ROBOT_INTERFACE_ACTR_PARA:
		SubKeyTask_ACTR_PARA(KeyVal);
		break;
	case ROBOT_INTERFACE_DEV_PARA:
		SubKeyTask_DEV_PARA(KeyVal);
		break;
	}
}

/**
 * ��������: SubKeyTask_MODE
 * ��   ��:	ģʽѡ������µİ���������
 * ��ڲ���: 
 * 		KeyVal	��ֵ
 * ���ڲ���: 
 * ��   ע:
 */
void SubKeyTask_MODE(uint8_t KeyVal)
{
	switch (KeyVal)
	{
	case KEY0_PRES:
		break;
	case KEY1_PRES:
		switch (ModeCursorIndex)
		{
		case MODECURSOR_ACTRON:
			InterfaceIndex = ROBOT_INTERFACE_ACTR_PWR;
			TASK_STOP_REGULAR_REPORT;
			ShowActrPwrStateTitle();
			for (int i = 0; i < 12; i++)
			{
				SetActrPwrState(PWR_ON, devIDList[i]);
				SetActrSpeedOutputLowerLimit(-0.2, devIDList[i]);
				SetActrSpeedOutputUpperLimit(0.2, devIDList[i]);
				BEEP_Normal(1);
				//GetActrPara(ACTR_CMD_GET_ON_OFF, devIDList[i]);
			}
			ShowActrPwrState();
			TASK_BEGIN_REGULAR_REPORT;
			break;
		case MODECURSOR_SETHMODE:
			TASK_STOP_REGULAR_REPORT;
			InterfaceIndex = ROBOT_INTERFACE_HOMING_CHECK;
			ShowActrHomingTitle(-1);
			break;
		case MODECURSOR_SETPMODE:
			TASK_STOP_REGULAR_REPORT;
			InterfaceIndex = ROBOT_INTERFACE_ACTR_POS;
			ShowActrPosValTitle();
			break;
		case MODECURSOR_ACTRPARA:
			TASK_BEGIN_REGULAR_REPORT;
			InterfaceIndex = ROBOT_INTERFACE_ACTR_PARA;
			ShowActrAllParaTitle();
			break;
		case MODECURSOR_ACTROFF:
			InterfaceIndex = ROBOT_INTERFACE_ACTR_PWR;
			TASK_STOP_REGULAR_REPORT;
			ShowActrPwrStateTitle();
			for (int i = 0; i < 12; i++)
			{
				SetActrPwrState(PWR_OFF, devIDList[i]);
				BEEP_Normal(1);
				//GetActrPara(ACTR_CMD_GET_ON_OFF, devIDList[i]);
			}
			ShowActrPwrState();
			TASK_BEGIN_REGULAR_REPORT;
			break;
		case MODECURSOR_DEVPARA:
			InterfaceIndex = ROBOT_INTERFACE_DEV_PARA;
			TASK_STOP_REGULAR_REPORT;
			ShowDevParaTitle();
			break;
		default:
			break;
		}
		break;
	case KEY2_PRES:
		break;
	case WKUP_PRES:
		ModeCursorIndex++;
		MoveModeCursor();
		break;
	}
}

/**
 * ��������: SubKeyTask_ACTR_PWR
 * ��   ��:	��Դ״̬�����µİ���������
 * ��ڲ���: 
 * 		KeyVal	��ֵ
 * ���ڲ���: 
 * ��   ע:
 */
void SubKeyTask_ACTR_PWR(uint8_t KeyVal)
{
	switch (KeyVal)
	{
	case KEY0_PRES:
		break;
	case KEY1_PRES:
		switch (ActrPwrCursorIndex)
		{
		case 0:
			InterfaceIndex = ROBOT_INTERFACE_MODE;
			ShowModeSelection(-1);
			break;
		default:
			break;
		}
		break;
	case KEY2_PRES:
		break;
	case WKUP_PRES:
		MoveActrPwrCursor();
		break;
	}
}

/**
 * ��������: SubKeyTask_ACTR_POS
 * ��   ��:	λ����ʾ�����µİ���������
 * ��ڲ���: 
 * 		KeyVal	��ֵ
 * ���ڲ���: 
 * ��   ע:
 */
void SubKeyTask_ACTR_POS(uint8_t KeyVal)
{
	static ActrParaTypedef *pActrParaDev = NULL;
	switch (KeyVal)
	{
	case KEY0_PRES:
		break;
	case KEY1_PRES:
		switch (ActrPwrCursorIndex)
		{
		case 0:
			InterfaceIndex = ROBOT_INTERFACE_MODE;
			ShowModeSelection(-1);
			break;
		default:
			break;
		}
		break;
	case KEY2_PRES:
		if (ActrHomingCorrect == 12)
		{
			for (int i = 0; i < 12; i++)
			{
				GetActrPara(ACTR_CMD_GET_CUR_MODE, devIDList[i]);
				if (pActrParaDev->actrMode != ACTR_MODE_TSHAP_POS)
				{
					SetActrMode(ACTR_MODE_TSHAP_POS, devIDList[i]);
				}
				BEEP_Normal(1);
			}
			ActrHomingError = 0;
			ActrHomingCorrect = 0;
			for (int i = 0; i < 12; i++)
			{
				ActrHomingErrorID[i] = 0;
			}
		}
		else
		{
			BEEP_Error(2);
		}
		break;
	case WKUP_PRES:
		MoveActrPosCursor();
		break;
	}
}

int ActrHomingError = 0;
int ActrHomingCorrect = 0;
int ActrHomingErrorID[12];

/**
 * ��������: SubKeyTask_HOMING_CHECK
 * ��   ��:	��λ�������µİ���������
 * ��ڲ���: 
 * 		KeyVal	��ֵ
 * ���ڲ���: 
 * ��   ע:
 */
void SubKeyTask_HOMING_CHECK(uint8_t KeyVal)
{
	char id_index;

	switch (ActrHomingCursorIndex - 1)
	{
	case 0:
	case 1:
	case 2:
		id_index = ActrHomingCursorIndex - 1;
		break;
	case 3:
	case 4:
	case 5:
		id_index = ActrHomingCursorIndex + 2;
		break;
	case 6:
	case 7:
	case 8:
		id_index = ActrHomingCursorIndex + 2;
		break;
	case 9:
	case 10:
	case 11:
		id_index = ActrHomingCursorIndex - 7;
	}

	switch (KeyVal)
	{
	case KEY0_PRES:
		ActrHomingErrorID[ActrHomingError] = devIDList[id_index];
		ActrHomingError++;
		ShowActrHoming(HOMING_FLAG_ERROR);
		break;
	case KEY1_PRES:
		InterfaceIndex = ROBOT_INTERFACE_MODE;
		ShowModeSelection(-1);
		break;
	case KEY2_PRES:
		ActrHomingCorrect++;
		ShowActrHoming(HOMING_FLAG_CORRECT);
		break;
	case WKUP_PRES:
		ActrHomingCursorIndex++;
		MoveActrHomingCursor();
		break;
	}
}

/**
 * ��������: SubKeyTask_ACTR_PARA
 * ��   ��:	���������µİ���������
 * ��ڲ���: 
 * 		KeyVal	��ֵ
 * ���ڲ���: 
 * ��   ע:
 */
void SubKeyTask_ACTR_PARA(uint8_t KeyVal)
{
	switch (KeyVal)
	{
	case KEY0_PRES:
		break;
	case KEY1_PRES:
		switch (DevParaCursorIndex)
		{
		case 0:
			InterfaceIndex = ROBOT_INTERFACE_MODE;
			ShowModeSelection(-1);
			break;
		default:
			break;
		}
		break;
	case KEY2_PRES:
		break;
	case WKUP_PRES:
		ActrAllParaCursorIndex++;
		if (ActrAllParaCursorIndex == 12)
		{
			ActrAllParaCursorIndex = 0;
		}
		MoveActrPosCursor();
		break;
	}
}

/**
 * ��������: SubKeyTask_DEV_PARA
 * ��   ��:	���������ʾ�����µİ���������
 * ��ڲ���: 
 * 		KeyVal	��ֵ
 * ���ڲ���: 
 * ��   ע:
 */
void SubKeyTask_DEV_PARA(uint8_t KeyVal)
{
	switch (KeyVal)
	{
	case KEY0_PRES:
		break;
	case KEY1_PRES:
		switch (ActrPwrCursorIndex)
		{
		case 0:
			InterfaceIndex = ROBOT_INTERFACE_MODE;
			ShowModeSelection(-1);
			break;
		default:
			break;
		}
		break;
	case KEY2_PRES:
		break;
	case WKUP_PRES:
		MoveDevParaCursor();
		break;
	}
}

/**
 * ��������: SubmissiveCtrlTask
 * ��   ��:	�㲿������˳����
 * ��ڲ���: 
 * 		TriggerData	����С��ĽŲ��������ش������ݣ������Դ���
 * ���ڲ���: 
 * ��   ע:
 */
void SubmissiveCtrlTask(uint8_t TriggerData)
{
	static uint8_t TriggerDataOld = 0;

	switch (TriggerData ^ TriggerDataOld)
	{
	case 0x01:
		if (TriggerData & 0x01)
		{
			SetActrSpeedOutputLowerLimit(-0.001, devIDList[0 * 3 + 1]);
			SetActrSpeedOutputLowerLimit(-0.001, devIDList[0 * 3 + 2]);
			SetActrSpeedOutputUpperLimit(0.001, devIDList[0 * 3 + 1]);
			SetActrSpeedOutputUpperLimit(0.001, devIDList[0 * 3 + 2]);
		}
		else
		{
			SetActrSpeedOutputLowerLimit(-0.005, devIDList[0 * 3 + 1]);
			SetActrSpeedOutputLowerLimit(-0.005, devIDList[0 * 3 + 2]);
			SetActrSpeedOutputUpperLimit(0.005, devIDList[0 * 3 + 1]);
			SetActrSpeedOutputUpperLimit(0.005, devIDList[0 * 3 + 2]);
		}
		break;
	case 0x02:
		if (TriggerData & 0x02)
		{
			SetActrSpeedOutputLowerLimit(-0.01, devIDList[1 * 3 + 1]);
			SetActrSpeedOutputLowerLimit(-0.01, devIDList[1 * 3 + 2]);
			SetActrSpeedOutputUpperLimit(0.01, devIDList[1 * 3 + 1]);
			SetActrSpeedOutputUpperLimit(0.01, devIDList[1 * 3 + 2]);
		}
		else
		{
			SetActrSpeedOutputLowerLimit(-0.05, devIDList[1 * 3 + 1]);
			SetActrSpeedOutputLowerLimit(-0.05, devIDList[1 * 3 + 2]);
			SetActrSpeedOutputUpperLimit(0.05, devIDList[1 * 3 + 1]);
			SetActrSpeedOutputUpperLimit(0.05, devIDList[1 * 3 + 2]);
		}
		break;
	case 0x04:
		if (TriggerData & 0x04)
		{
			SetActrSpeedOutputLowerLimit(-0.005, devIDList[2 * 3 + 1]);
			SetActrSpeedOutputLowerLimit(-0.005, devIDList[2 * 3 + 2]);
			SetActrSpeedOutputUpperLimit(0.005, devIDList[2 * 3 + 1]);
			SetActrSpeedOutputUpperLimit(0.005, devIDList[2 * 3 + 2]);
		}
		else
		{
			SetActrSpeedOutputLowerLimit(-0.001, devIDList[2 * 3 + 1]);
			SetActrSpeedOutputLowerLimit(-0.001, devIDList[2 * 3 + 2]);
			SetActrSpeedOutputUpperLimit(0.001, devIDList[2 * 3 + 1]);
			SetActrSpeedOutputUpperLimit(0.001, devIDList[2 * 3 + 2]);
		}
		break;
	case 0x08:
		if (TriggerData & 0x08)
		{
			SetActrSpeedOutputLowerLimit(-0.001, devIDList[3 * 3 + 1]);
			SetActrSpeedOutputLowerLimit(-0.001, devIDList[3 * 3 + 2]);
			SetActrSpeedOutputUpperLimit(0.001, devIDList[3 * 3 + 1]);
			SetActrSpeedOutputUpperLimit(0.001, devIDList[3 * 3 + 2]);
		}
		else
		{
			SetActrSpeedOutputLowerLimit(-0.005, devIDList[3 * 3 + 1]);
			SetActrSpeedOutputLowerLimit(-0.005, devIDList[3 * 3 + 2]);
			SetActrSpeedOutputUpperLimit(0.005, devIDList[3 * 3 + 1]);
			SetActrSpeedOutputUpperLimit(0.005, devIDList[3 * 3 + 2]);
		}
		break;
	}

	TriggerDataOld = TriggerData;
	
	BEEP_Normal(1);
}
