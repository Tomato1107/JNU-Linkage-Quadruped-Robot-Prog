#include "tasks.h"

int ActivateFlag = STOP;
int AutoManualFlag = IDLE;

const float Spd_Factor = 0.50f;

void InitTask(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);
	USART1_Init(115200);
	UART4_Init(100000);

	LED_Init();
	BEEP_Init();
	KEY_Init();
	LCD_Init();
	IO_Init();
	TIM3_Init();
	TIM14_PWM_Init(20000-1,84-1);

	CAN1_Init();
	ActrDevInit();

	ShowModeSelection(-1);

	InitActrPhasePID();
}

void ControlTask(void)
{
	switch (AutoManualFlag)
	{
	case AUTO_L:
		GetCtrlData(CTRL_SRC_AUTO_L);
		break;

	case AUTO_R:
		GetCtrlData(CTRL_SRC_AUTO_R);
		break;

	case MANUAL:
		GetCtrlData(CTRL_SRC_REMOTE);
		break;
	}

	UpdateActrPhase();
	CountActrRevolution();

	if (ActivateFlag == RUN)
	{
		if (CtrlVal_Forward > 0.0f)
		{
			if (actrRefPhase < 2.0f)
			{
				actrSpd[LM1_INDEX] = -Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 1.0f, -0.06f, 1 / sqrt(2 * PI)) - PID_LM1.Output;
				actrSpd[LM2_INDEX] = Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 1.0f, -0.06f, 1 / sqrt(2 * PI));
				actrSpd[RM2_INDEX] = -Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 1.0f, -0.06f, 1 / sqrt(2 * PI)) - PID_RM2.Output;
				actrSpd[RM1_INDEX] = Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 1.0f, -0.06f, 1 / sqrt(2 * PI)) + PID_RM1.Output;
			}
			else
			{
				actrSpd[LM1_INDEX] = -Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 3.0f, -0.06f, 1 / sqrt(2 * PI)) - PID_LM1.Output;
				actrSpd[LM2_INDEX] = Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 3.0f, -0.06f, 1 / sqrt(2 * PI));
				actrSpd[RM2_INDEX] = -Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 3.0f, -0.06f, 1 / sqrt(2 * PI)) - PID_RM2.Output;
				actrSpd[RM1_INDEX] = Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 3.0f, -0.06f, 1 / sqrt(2 * PI)) + PID_RM1.Output;
			}
		}
		else
		{
			if (actrRefPhase < 2.0f)
			{
				actrSpd[LM1_INDEX] = -Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 1.0f, 0.06f, 1 / sqrt(2 * PI)) - PID_LM1.Output;
				actrSpd[LM2_INDEX] = Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 1.0f, 0.06f, 1 / sqrt(2 * PI));
				actrSpd[RM2_INDEX] = -Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 1.0f, 0.06f, 1 / sqrt(2 * PI)) - PID_RM2.Output;
				actrSpd[RM1_INDEX] = Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 1.0f, 0.06f, 1 / sqrt(2 * PI)) + PID_RM1.Output;
			}
			else
			{
				actrSpd[LM1_INDEX] = -Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 3.0f, 0.06f, 1 / sqrt(2 * PI)) - PID_LM1.Output;
				actrSpd[LM2_INDEX] = Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 3.0f, 0.06f, 1 / sqrt(2 * PI));
				actrSpd[RM2_INDEX] = -Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 3.0f, 0.06f, 1 / sqrt(2 * PI)) - PID_RM2.Output;
				actrSpd[RM1_INDEX] = Spd_Factor * CtrlVal_Forward / 128.0f * normpdf_revised(actrRefPhase - 3.0f, 0.06f, 1 / sqrt(2 * PI)) + PID_RM1.Output;
			}
		}

		CalcActrPhasePID();

		if (fabs(actrRefPhase - 1.0f) < 0.5f)
		{
			SetActrPosition(TM_LIMIT * CtrlVal_Turning / 128.0f, devIDList[TM_INDEX]);
		}
		else if (fabs(actrRefPhase - 3.0f) < 0.5f)
		{
			SetActrPosition(-TM_LIMIT * CtrlVal_Turning / 128.0f, devIDList[TM_INDEX]);
		}
	}
	else
	{
		actrSpd[LM1_INDEX] = 0;
		actrSpd[LM2_INDEX] = 0;
		actrSpd[RM2_INDEX] = 0;
		actrSpd[RM1_INDEX] = 0;

		SetActrPosition(0.0f, devIDList[TM_INDEX]);
	}
	SetActrSpeed(actrSpd[LM1_INDEX], devIDList[LM1_INDEX]);
	SetActrSpeed(actrSpd[LM2_INDEX], devIDList[LM2_INDEX]);
	SetActrSpeed(actrSpd[RM2_INDEX], devIDList[RM2_INDEX]);
	SetActrSpeed(actrSpd[RM1_INDEX], devIDList[RM1_INDEX]);
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
	for (int i = 0; i < ACTR_DEV_NUM; i++)
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

void CheckIOTask(void);

/**
 * ��������: HandleLowPirorityTask
 * ��   ��:	���ݱ�־λ��������ȼ�����
 * ��ڲ���: 
 * ���ڲ���: 
 * ��   ע:
 */
void HandleLowPirorityTask(void)
{
	KeyTask();
	if (TASK_SEARCH_FLAG(TASK_FLAG_CONTROL) == 0) //���δ����ControlTask��UploadTask����������ʾ������������
	{
		DisplayTask();
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
	}
	else
	{
		TASK_RESET_FLAG(TASK_FLAG_CONTROL);
		ControlTask();
	}

	CheckFootGroundingTask();
	HandleDevDataTask();
	CheckIOTask();

	if (CheckActrRunningStateTask() == 0)
		ActivateFlag = ERROR;
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
		TIM_SetCompare1(TIM14,500);
		break;
	case KEY1_PRES:
		switch (ModeCursorIndex)
		{
		case MODECURSOR_ACTRON:
			InterfaceIndex = ROBOT_INTERFACE_ACTR_PWR;
			TASK_STOP_REGULAR_REPORT;
			ShowActrPwrStateTitle();
			for (int i = 0; i < ACTR_DEV_NUM; i++)
			{
				SetActrPwrState(PWR_ON, devIDList[i]);
				BEEP_Normal(1);
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
			for (int i = 0; i < ACTR_DEV_NUM; i++)
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
		TIM_SetCompare1(TIM14,2500);
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

extern int AutoTime_Flag;
extern int AutoTime_S;
extern int AutoTime_MS;
extern uint8_t StepIndex;
extern uint8_t StepIndex_Pause;
extern uint8_t StepIndex_PauseFlag;

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
		BEEP_Alert(1);
		ActivateFlag = STOP;
		AutoManualFlag++;
		if (AutoManualFlag == 4)
			AutoManualFlag = IDLE;
		break;

	case KEY1_PRES:
		ActivateFlag = STOP;
		ClearActrPhase();
		ClearActrRevolution();
		AutoTime_S = 0;
		AutoTime_MS = 0;
		AutoTime_Flag = 0;
		switch (ActrPosCursorIndex)
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
		for (int i = 0; i < ACTR_DEV_NUM; i++)
		{
			GetActrPara(ACTR_CMD_GET_CUR_MODE, devIDList[i]);
			pActrParaDev = FindActrDevByID(devIDList[i]);
			if (i != 2)
			{
				if (pActrParaDev->actrMode != ACTR_MODE_TSHAP_SPD)
				{
					SetActrMode(ACTR_MODE_TSHAP_SPD, devIDList[i]);
				}
			}
			else
			{
				if (pActrParaDev->actrMode != ACTR_MODE_TSHAP_POS)
				{
					SetActrMode(ACTR_MODE_TSHAP_POS, devIDList[i]);
				}
			}
			BEEP_Normal(1);
		}

		break;

	case WKUP_PRES:
		if(RemoteData[5] != 2)
		{
			if (AutoManualFlag == AUTO_L || AutoManualFlag == AUTO_R)
			{
				BEEP_Alert(3);
				MoveActrPosCursor();
				AutoTime_Flag = 1;
				ActivateFlag = RUN;
				if (StepIndex == StepIndex_Pause)
				{
					StepIndex_PauseFlag = STEP_CONTINUE;
				}
				InitAutoData();
			}
			else
			{
				ActivateFlag = RUN;
			}
		}
		else
			BEEP_Normal(1);
		break;
	}
}

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
	static ActrParaTypedef *pActrParaDev = NULL;

	id_index = ActrHomingCursorIndex - 1;

	switch (KeyVal)
	{
	case KEY0_PRES:
		BEEP_Alert(2);
		for (int i = 0; i < ACTR_DEV_NUM; i++)
		{
			SetActrMode(ACTR_MODE_TSHAP_POS, devIDList[i]);
			BEEP_Alert(1);
			SetActrPosition(0.0f, devIDList[i]);
		}
		break;
	case KEY1_PRES:
		InterfaceIndex = ROBOT_INTERFACE_MODE;
		ShowModeSelection(-1);
		break;
	case KEY2_PRES:

		GetActrPara(ACTR_CMD_GET_CUR_MODE, devIDList[id_index]);

		if (pActrParaDev->actrMode != ACTR_MODE_TSHAP_POS)
		{
			SetActrMode(ACTR_MODE_TSHAP_POS, devIDList[id_index]);
		}

		switch (id_index)
		{
		case LM1_INDEX:
			SetActrPosition(0.0f, devIDList[id_index]);
			break;

		case LM2_INDEX:
			SetActrPosition(0.0f, devIDList[id_index]);
			break;

		case TM_INDEX:
			SetActrPosition(0.0f, devIDList[id_index]);
			break;

		case RM2_INDEX:
			SetActrPosition(0.0f, devIDList[id_index]);
			break;

		case RM1_INDEX:
			SetActrPosition(0.0f, devIDList[id_index]);
			break;

		default:
			break;
		}

		BEEP_Normal(1);

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
		if (ActrAllParaCursorIndex == ACTR_DEV_NUM)
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

const float Cur_Threshold = 1.2f;
uint8_t FootGrounding;
/**
 * ��������: CheckFootGroundingTask
 * ��   ��:	�㲿������˳����
 * ��ڲ���: 
 * 		
 * ���ڲ���: 
 * ��   ע:	ͨ�����ϥ�ؽڵ���ʵ����ؼ��
 */
void CheckFootGroundingTask(void)
{
	static ActrParaTypedef *pActrParaDev = NULL;

	pActrParaDev = FindActrDevByID(devIDList[0]);
	if (fabs(pActrParaDev->actrCurrent * 33.0f) > Cur_Threshold)
	{
		FootGrounding |= 0x01 << 0;
	}
	else
	{
		FootGrounding &= ~(0x01 << 0);
	}

	pActrParaDev = FindActrDevByID(devIDList[1]);
	if (fabs(pActrParaDev->actrCurrent * 33.0f) > Cur_Threshold)
	{
		FootGrounding |= 0x01 << 1;
	}
	else
	{
		FootGrounding &= ~(0x01 << 1);
	}

	pActrParaDev = FindActrDevByID(devIDList[3]);
	if (fabs(pActrParaDev->actrCurrent * 33.0f) > Cur_Threshold)
	{
		FootGrounding |= 0x01 << 2;
	}
	else
	{
		FootGrounding &= ~(0x01 << 2);
	}

	pActrParaDev = FindActrDevByID(devIDList[4]);
	if (fabs(pActrParaDev->actrCurrent * 33.0f) > Cur_Threshold)
	{
		FootGrounding |= 0x01 << 3;
	}
	else
	{
		FootGrounding &= ~(0x01 << 3);
	}
}

void HandleDevDataTask(void)
{
	Remote_Get();
}

int CheckActrRunningStateTask(void)
{
	if (fabs((actrRefPhase + 4.0f * actrRefRevolution) - (actrPhase[LM1_INDEX] + 4.0f * actrRevolution[LM1_INDEX])) > 1.0f)
		return 0;
	if (fabs((actrRefPhase + 4.0f * actrRefRevolution) - (actrPhase[LM2_INDEX] + 4.0f * actrRevolution[LM2_INDEX])) > 1.0f)
		return 0;
	if (fabs((actrRefPhase + 4.0f * actrRefRevolution) - (actrPhase[RM2_INDEX] + 4.0f * actrRevolution[RM2_INDEX])) > 1.0f)
		return 0;
	if (fabs((actrRefPhase + 4.0f * actrRefRevolution) - (actrPhase[RM1_INDEX] + 4.0f * actrRevolution[RM1_INDEX])) > 1.0f)
		return 0;

	if (RemoteData[5] == 2)
		ActivateFlag = STOP;

	return 1;
}

void CheckIOTask(void)
{
	static uint8_t io_state;
	static uint8_t shawdow_io_state;
	
	io_state = IO;
	
	if(shawdow_io_state != io_state && io_state == 1 && (AutoManualFlag == AUTO_L || AutoManualFlag == AUTO_R))
	{
		AutoTime_Flag = 1;
		ActivateFlag = RUN;
		BEEP_Normal(1);
		InitAutoData();
	}
	
	if(ActivateFlag == RUN && StepIndex == 128)
		TIM_SetCompare1(TIM14,500);
	
	shawdow_io_state = io_state;
		//BEEP_Normal(1);
}
