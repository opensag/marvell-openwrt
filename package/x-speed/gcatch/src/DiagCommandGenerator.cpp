// DiagCommandGenerator.cpp: implementation of the DiagCommandGenerator class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
//#include "bpLog.h"
#include "DiagCommandGenerator.h"
/*
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
///
/*
const char *QueryCmdList[] = {
		"1D",
		"00",
		"7C",
		"1C",
		"0C",
		"63",
		"4B 0F 00 00",
		"4B 09 00 00",
		"4B 08 00 00",
		"4B 08 01 00",
		"4B 04 00 00",
		"4B 04 0F 00",
		"73 00 00 00 01 00 00 00",
		"7D 01",
		"81 00 00 00",
		"4B 13 01 00",
		"4B 13 00 00 01 00 00 00 00 02 00 00 01 00 00 00 00 02 00 00 01 00 00 00 00 02 00 00 01 00 00 00 01 00 00 00 01 00 00 00 00 00 00 00"
};
*/

const unsigned char cmd1[] = {0x01, 0x1D};
const unsigned char cmd2[] = {0x01, 0x00};
const unsigned char cmd3[] = {0x01, 0x7C};
const unsigned char cmd4[] = {0x01, 0x1C};
const unsigned char cmd5[] = {0x01, 0x0C};
const unsigned char cmd6[] = {0x01, 0x63};
const unsigned char cmd7[] = {0x04, 0x4B, 0x0F, 0x00, 0x00};
const unsigned char cmd8[] = {0x04, 0x4B, 0x09, 0x00, 0x00};
const unsigned char cmd9[] = {0x04, 0x4B, 0x08, 0x00, 0x00};
const unsigned char cmd10[] = {0x04, 0x4B, 0x08, 0x01, 0x00};
const unsigned char cmd11[] = {0x04, 0x4B, 0x04, 0x00, 0x00};
const unsigned char cmd12[] = {0x04, 0x4B, 0x04, 0x0F, 0x00};
const unsigned char cmd13[] = {0x08, 0x73, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
const unsigned char cmd14[] = {0x02, 0x7D, 0x01};
const unsigned char cmd15[] = {0x04, 0x81, 0x00, 0x00, 0x00};
const unsigned char cmd16[] = {0x04, 0x4B, 0x13, 0x01, 0x00};
const unsigned char cmd17[] = {0x2C, 0x4B, 0x13, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char *QueryCmdList[] = {cmd1, cmd2, cmd3, cmd4, cmd5, cmd6, cmd7, cmd8, cmd9, cmd10, cmd11, cmd12, cmd13, cmd14, cmd15, cmd16, cmd17};

DiagCommandGenerator::DiagCommandGenerator()
{
	memset(m_setCmdList, 0, sizeof(m_setCmdList));
	memset(m_setCmdLen, 0, sizeof(m_setCmdLen));
	m_setCmdIndex = 0;
	m_queryCmdIndex = 0;
	m_msgQueryCmdIndex = 0;
	m_dmcFileText = NULL;
	m_needParseDmcFile = 0;
	m_isEFS2_DIAGClosed = 0;

	m_supportedLogItemsIndex = 0;
	m_supportedMsgsIndex = 0;

	m_curDiagWriteCmdBufLeft = 0;
	m_curDiagWriteCmdCount = 0;
	m_usableIndexInSetCmdList = 0;

	m_openCount = 0;
}
/*
void DiagCommandGenerator::CloseDmcFile()
{
	if (NULL != m_filePointer)
	{
		fclose(m_filePointer);
		m_filePointer = NULL;
	}
}
*/
DiagCommandGenerator::~DiagCommandGenerator()
{
	//CloseDmcFile();
	ClearSetCmdList();

	if (NULL != m_dmcFileText)
	{
		delete []m_dmcFileText;
		m_dmcFileText = NULL;
	}
}

void DiagCommandGenerator::ClearSetCmdList()
{
	int i = 0;
	for (i=0; i<SET_CMD_MAX; i++)
	{
		if (NULL != m_setCmdList[i])
		{
			delete []m_setCmdList[i];
			m_setCmdList[i] = NULL;
		}
	}
	memset(m_setCmdLen, 0, sizeof(m_setCmdLen));
}

void DiagCommandGenerator::GetDmcFileText(const char *path, int configMode)
{
	int i = 0;
	char *cur_pos = NULL;
	char *openViewEnd = NULL;
	char *temp_1 = NULL;
	char *temp_2 = NULL;
	char tok_buf[16] = {0};
	FILE *filePointer = fopen(path, "rb");
	fseek(filePointer, 0, SEEK_END);
	int size = ftell(filePointer);
	m_dmcFileText = new char[size + 1];
	fseek(filePointer, 0, SEEK_SET);
	fread(m_dmcFileText, 1, size, filePointer);
	m_dmcFileText[size] = '\0';

	fclose(filePointer);

	m_openCount = 0;
	m_messageView.Initialize(m_dmcFileText, "<MessagesView>", "</MessagesView>");
	m_logView.Initialize(m_dmcFileText, "<LoggingView>", "</LoggingView>");

	cur_pos = strstr(m_dmcFileText, "<Displays>");
	openViewEnd = strstr(m_dmcFileText, "</Displays>");

	if ((NULL == cur_pos) || (NULL == openViewEnd))
	{
		m_openedViews[m_openCount++] = m_messageView;
		return;
	}

	temp_1 = strstr(cur_pos, "Messages View");
	temp_2 = strstr(cur_pos, "Log View");
	
	if ((NULL != temp_1) && (temp_1 <openViewEnd))
	{
		m_openedViews[m_openCount++] = m_messageView;
	}

	if ((NULL != temp_2) && (temp_2 <openViewEnd))
	{
		m_openedViews[m_openCount++] = m_logView;
	}

	for (i=0; i<MAX_OPEN_VIEWS; i++)
	{
		sprintf(tok_buf, "<Display%d>", i);
		temp_1 = strstr(cur_pos, tok_buf);
		if ((NULL == temp_1) || (temp_1 > openViewEnd))
		{
			break;
		}
		
		sprintf(tok_buf, "</Display%d>", i);
		temp_2 = strstr(cur_pos, tok_buf);
		if ((NULL == temp_2) || (temp_2 > openViewEnd))
		{
			break;
		}

		cur_pos = strstr(temp_1, "Filtered View");
		if (NULL == cur_pos)
		{
			break;
		}

		if (cur_pos > temp_2)
		{
			cur_pos = temp_2 + strlen(tok_buf);
			continue;
		}

		cur_pos = strstr(temp_1, "<ISVConfig>");
		if (NULL == cur_pos)
		{
			break;
		}

		if (cur_pos > temp_2)
		{
			cur_pos = temp_2 + strlen(tok_buf);
			continue;
		}

		m_openedViews[m_openCount++].Initialize(cur_pos, "<ISVConfig>", "</ISVConfig>");
		cur_pos = temp_2 + strlen(tok_buf);
	}

	if (m_openCount == 0)
	{
		m_openedViews[m_openCount++] = m_messageView;
	}
	
#if 0
	switch (configMode)
	{
	case USE_AUTO_CHOOSE:
		if (NULL != strstr(m_dmcFileText, "Messages View"))
		{
			m_messageView.Initialize(m_dmcFileText, "<MessagesView>", "</MessagesView>");
		}
		if (NULL != strstr(m_dmcFileText, "Log View"))
		{
			m_logView.Initialize(m_dmcFileText, "<LoggingView>", "</LoggingView>");
		}
		if ((NULL == strstr(m_dmcFileText, "Log View")) && (NULL == strstr(m_dmcFileText, "Messages View")))
		{
			m_messageView.Initialize(m_dmcFileText, "<MessagesView>", "</MessagesView>");
		}
		break;
	case USE_MESSAGE_VIEW:
		m_messageView.Initialize(m_dmcFileText, "<MessagesView>", "</MessagesView>");
		break;
	case USE_LOG_VIEW:
		m_logView.Initialize(m_dmcFileText, "<LoggingView>", "</LoggingView>");
		break;
	case USE_BOTH:
		m_messageView.Initialize(m_dmcFileText, "<MessagesView>", "</MessagesView>");
		m_logView.Initialize(m_dmcFileText, "<LoggingView>", "</LoggingView>");
		break;
	default:
		break;
	}
#endif
}

void DiagCommandGenerator::GetNextCommand(unsigned char *cmdBuffer, unsigned long *cmdLen)
{
	if (m_queryCmdIndex < (sizeof(QueryCmdList)/sizeof(unsigned char *)))
	{
		*cmdLen = QueryCmdList[m_queryCmdIndex][0];
		memcpy(cmdBuffer, QueryCmdList[m_queryCmdIndex]+1, *cmdLen);
		m_queryCmdIndex++;
	}
	else if (NULL != m_supportedMsgs[m_msgQueryCmdIndex].m_range)
	{
		*cmdLen = 6;
		cmdBuffer[0] = 0x7D;
		cmdBuffer[1] = 0x02;
		memcpy(cmdBuffer+2, m_supportedMsgs[m_msgQueryCmdIndex].m_range, 4);
		m_msgQueryCmdIndex++;
		if (NULL == m_supportedMsgs[m_msgQueryCmdIndex].m_range)
		{
			m_needParseDmcFile = 1;
		}
	}
	else if (0 != m_needParseDmcFile)
	{
		CreateSetCmdList();
		m_needParseDmcFile = 0;
		*cmdLen = m_setCmdLen[m_setCmdIndex];
		memcpy(cmdBuffer, m_setCmdList[m_setCmdIndex], m_setCmdLen[m_setCmdIndex]);
		m_setCmdIndex++;
	}
	else if (NULL != m_setCmdList[m_setCmdIndex])
	{
		*cmdLen = m_setCmdLen[m_setCmdIndex];
		memcpy(cmdBuffer, m_setCmdList[m_setCmdIndex], m_setCmdLen[m_setCmdIndex]);
		m_setCmdIndex++;
	}
	else if (0 == m_isEFS2_DIAGClosed)
	{
		m_isEFS2_DIAGClosed = 1;
		*cmdLen = 8;
		unsigned char temp[] = {0x4B, 0x13, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
		memcpy(cmdBuffer, temp, 8);
	}
	else
	{
		*cmdLen = 0;
	}
}
/*
int DiagCommandGenerator::CreateCmdList()
{
	if (NULL == m_filePointer)
	{
		return 0;
	}

	ClearSetCmdList();


}
*/
void DiagCommandGenerator::CreateSetCmdList()
{

	//char *messageView_start = strstr(m_dmcFileText, "<MessagesView>");
	//char *messageView_end = strstr(m_dmcFileText, "</MessagesView>");

	ClearSetCmdList();
	m_setCmdIndex = 0;

	ParseEventIDFromFile();
	ParseLogCodeAndOTALogCode();
	ParseMsgInfoFromFile();

	//添加打开文件命令
	unsigned char temp[] = {0x4B, 0x13, 0x02, 0x00, 0x41, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2E, 0x44, 0x49, 0x41, 0x47, 0x49, 0x4E, 0x00};
	m_setCmdLen[m_setCmdIndex] = sizeof(temp);
	m_setCmdList[m_setCmdIndex] = new unsigned char[sizeof(temp)];
	memcpy(m_setCmdList[m_setCmdIndex], temp, sizeof(temp));

	m_setCmdIndex++;
	
	m_usableIndexInSetCmdList = m_setCmdIndex;
	m_setCmdIndex = 0;


/*
	char *messageView_start = strstr(m_dmcFileText, "<MessagesView>");
	char *messageView_end = strstr(m_dmcFileText, "</MessagesView>");


	int index = 0;
	int size = 6;
	m_setCmdList[index] = new char[size];
	
	//解析Event，可能生成1~2条指令
	char *section_start = strstr(messageView_start, "<EventIDs>");
	char *section_end = strstr(messageView_start, "</EventIDs>");

	char *data_start = NULL;
	char *data_end = NULL;
	if ((section_start > messageView_end) || (NULL == section_start))
	{
		strcpy(m_setCmdList[index], "60 00");
	}
	else
	{
		//data_end = strstr(data_start, "</EventIDs>");
		strcpy(m_setCmdList[index], "60 01");
		index++;
		char *temp = m_supportedEventIDs.m_range;
		int byteCount = 0;
		size = HexCharToInt(temp[3])*16*256 + HexCharToInt(temp[4])*256 + HexCharToInt(temp[0])*16 + HexCharToInt(temp[1]);
		byteCount = size/8 + ((size%8 == 0) ? 0 : 1);
		m_setCmdList[index] = new char[byteCount * 3 + 18];
		strcpy(m_setCmdList[index], "82 00 00 00 ");
		strcat(m_setCmdList[index], m_supportedEventIDs.m_range);
		data_start = section_start;
		unsigned char tempByte = '\0';
		int data = -1;
		char tempBuffer[4];
		for (int i=0; i<byteCount; i++)
		{
			tempByte = '\0';
			data = (data_start < section_end) ? atoi(data_start) : -1;
			while((data < (i+1)*8) && (data >= i*8))
			{
				tempByte |= (1<<(data%8));
				data_start += 6;
				data = (data_start < section_end) ? atoi(data_start) : -1;
			}
			tempByte &= (HexCharToInt(m_supportedEventIDs.m_data[3*i])*16 + HexCharToInt(m_supportedEventIDs.m_data[3*i+1]));
			sprintf(tempBuffer, " %02x", tempByte);
			strcat(m_setCmdList[index], tempBuffer);
		}
	}

	//解析Log Packets，生成的指令数不定，随着前面的执行结果而变化
	index++;
	int logIndex = 0;
	section_start = strstr(messageView_start, "<LogCodes>");
	section_end = strstr(messageView_start, "</LogCodes>");
	data_start = section_start;
	while (NULL != m_supportedLogItems[logIndex].m_range)
	{
		char *temp = m_supportedLogItems[logIndex].m_data;
		int byteCount = 0;
		size = HexCharToInt(temp[3])*16*256 + HexCharToInt(temp[4])*256 + HexCharToInt(temp[0])*16 + HexCharToInt(temp[1]);
		byteCount = (size+8)/8 + ((0 == size%8) ? 0 : 1);
		m_setCmdList[index] = new char[(byteCount+16)*3];
		strcpy(m_setCmdList[index], "73 00 00 00 03 00 00 00 ");
		strcat(m_setCmdList[index], m_supportedLogItems[logIndex].m_range);
		strcat(m_setCmdList[index], " 00 00 00");
		char tempBuffer[7];
		sprintf(tempBuffer, " %02x", (size+8)%256);
		strcat(m_setCmdList[index], tempBuffer);
		sprintf(tempBuffer, " %02x", (size+8)/256);
		strcat(m_setCmdList[index], tempBuffer);
		strcat(m_setCmdList[index], " 00 00 ");

		temp = m_supportedLogItems[logIndex].m_range;
		int maxData = 0;
		int minData = 0;
		int data = -1;
		strncpy(tempBuffer, data_start, 6);
		data = (data_start < section_end) ? HexStr2Int(tempBuffer) : -1;
		minData = (HexCharToInt(temp[0])*16 + HexCharToInt(temp[1]))*256*16;
		while ((data != -1) && (data < minData))
		{
			data_start += 7;
			data = (data_start < section_end) ? HexStr2Int(tempBuffer) : -1;
		}

		unsigned char tempByte = '\0';
		for (int i=0; i<byteCount; i++)
		{
			tempByte = '\0';
			strncpy(tempBuffer, data_start, 6);
			data = (data_start < section_end) ? HexStr2Int(tempBuffer) : -1;
			minData = (HexCharToInt(temp[0])*16 + HexCharToInt(temp[1]))*256*16 + i*8;
			maxData = ((size-1) < (minData+8)) ? (size-1) : (minData+8);
			while ((data >= minData) && (data < maxData))
			{
				tempByte |= (1<<(data%8));
				data_start += 7;
				data = (data_start < section_end) ? HexStr2Int(tempBuffer) : -1;
			}
		}
	}
*/	
}

void DiagCommandGenerator::ParseEventIDFromFile()
{
	int i = 0;
	int j = 0;
	unsigned char need_set_event = 0;
	m_setCmdList[m_setCmdIndex] = new unsigned char[2];
	
	//解析Event，可能生成1~2条指令
	//在这里，默认了eventID 最小为0，如果eventID 不是从0开始则此处代码需要修改
	for (j=0; j<m_openCount; j++)
	{
		if (m_openedViews[j].IsEventIDUsed())
		{
			need_set_event = 1;
			break;
		}
	}
	
	if (need_set_event == 0)
	{
		m_setCmdList[m_setCmdIndex][0] = 0x60;
		m_setCmdList[m_setCmdIndex][1] = 0x00;
		m_setCmdLen[m_setCmdIndex] = 2;
	}
	else
	{
		m_setCmdList[m_setCmdIndex][0] = 0x60;
		m_setCmdList[m_setCmdIndex][1] = 0x01;
		m_setCmdLen[m_setCmdIndex] = 2;
		m_setCmdIndex++;
		//char *temp = m_supportedEventIDs.m_range;
		int cmdBufferIndex = 0;
		int size = m_supportedEventIDs.m_range[1] * 256 + m_supportedEventIDs.m_range[0];//HexCharToInt(temp[3])*16*256 + HexCharToInt(temp[4])*256 + HexCharToInt(temp[0])*16 + HexCharToInt(temp[1]);
		int byteCount = size/8 + ((size%8 == 0) ? 0 : 1);
		m_setCmdList[m_setCmdIndex] = new unsigned char[byteCount+6];
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x82;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = m_supportedEventIDs.m_range[0];
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = m_supportedEventIDs.m_range[1];
		//strcat(m_setCmdList[m_setCmdIndex], m_supportedEventIDs.m_range);
		//data_start = section_start;
		unsigned char tempByte = 0x00;
		int data = -1;
		for (i=0; i<byteCount; i++)
		{
			tempByte = 0x00;

			for (j=0; j<m_openCount; j++)
			{
				data = m_openedViews[j].GetCurrentEventID();
				while((data < (i+1)*8) && (data >= i*8))
				{
					tempByte |= (1<<(data%8));
					data = m_openedViews[j].GotoNextEventIDAndReturn();
				}
			}
			
			//tempByte &= m_supportedEventIDs.m_data[i];//(HexCharToInt(m_supportedEventIDs.m_data[3*i])*16 + HexCharToInt(m_supportedEventIDs.m_data[3*i+1]));
			m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = tempByte;
		}
		m_setCmdLen[m_setCmdIndex] = cmdBufferIndex;
	}
	
	m_setCmdIndex++;

}

void DiagCommandGenerator::ParseLogCodeAndOTALogCode()
{
	int i = 0;
	int j = 0;
	int logIndex = 0;
	while (NULL != m_supportedLogItems[logIndex].m_range)
	{
		//char *temp = m_supportedLogItems[logIndex].m_data;
		int cmdBufferIndex = 0;
		int size = m_supportedLogItems[logIndex].m_data[1] * 256 + m_supportedLogItems[logIndex].m_data[0];//HexCharToInt(temp[3])*16*256 + HexCharToInt(temp[4])*256 + HexCharToInt(temp[0])*16 + HexCharToInt(temp[1]);
		int byteCount = (size+8)/8 + ((0 == size%8) ? 0 : 1);
		m_setCmdList[m_setCmdIndex] = new unsigned char[byteCount+16];
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x73;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x03;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = m_supportedLogItems[logIndex].m_range[0];
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = (size+8)%256;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = (size+8)/256;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;

		int maxData = 0;
		int minData = 0;
		int data = -1;
		unsigned char tempByte = '\0';
		//temp = m_supportedLogItems[logIndex].m_range;
		minData = m_supportedLogItems[logIndex].m_range[0] * 256 * 16;//(HexCharToInt(temp[0])*16 + HexCharToInt(temp[1]))*256*16;

		for (j=0; j<m_openCount; j++)
		{
			data = m_openedViews[j].GetCurrentLogCode();
			while ((data != -1) && (data < minData))
			{
				data = m_openedViews[j].GotoNextLogCodeAndReturn();
			}

			data = m_openedViews[j].GetCurrentOTALogCode();
			while ((data != -1) && (data < minData))
			{
				data = m_openedViews[j].GotoNextOTALogCodeAndReturn();
			}
		}
				
		int maxValidIndex = minData + size + 8;
		for (int i=0; i<byteCount; i++)
		{
			tempByte = '\0';
			minData = m_supportedLogItems[logIndex].m_range[0] * 256 * 16 + i*8;
			maxData = (maxValidIndex < (minData+8)) ? maxValidIndex : (minData+8);

			for (j=0; j<m_openCount; j++)
			{
				data = m_openedViews[j].GetCurrentLogCode();
				while ((data >= minData) && (data < maxData))
				{
					tempByte |= (1<<(data%8));
					data = m_openedViews[j].GotoNextLogCodeAndReturn();
				}

				data = m_openedViews[j].GetCurrentOTALogCode();
				while ((data >= minData) && (data < maxData))
				{
					tempByte |= (1<<(data%8));
					data = m_openedViews[j].GotoNextOTALogCodeAndReturn();
				}
			}
			
			if ((CORE_DUMP >= minData) && (CORE_DUMP < maxData))
			{
				tempByte |= (1<<(CORE_DUMP-minData));
			}

			m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = tempByte;
		}
		m_setCmdLen[m_setCmdIndex] = cmdBufferIndex;
		logIndex++;
		m_setCmdIndex++;
	}
}

void DiagCommandGenerator::ParseMsgInfoFromFile()
{
	int i = 0;
	int j = 0;
	int msgIndex = 0;
	while (NULL != m_supportedMsgs[msgIndex].m_range)
	{
		//char *temp = m_supportedMsgs[msgIndex].m_range;
		int cmdBufferIndex = 0;
		int minSSID = m_supportedMsgs[msgIndex].m_range[1] * 256 + m_supportedMsgs[msgIndex].m_range[0];//HexCharToInt(temp[3])*16*256 + HexCharToInt(temp[4])*256 + HexCharToInt(temp[0])*16 + HexCharToInt(temp[1]);
		int maxSSID = m_supportedMsgs[msgIndex].m_range[3] * 256 + m_supportedMsgs[msgIndex].m_range[2];//HexCharToInt(temp[9])*16*256 + HexCharToInt(temp[10])*256 + HexCharToInt(temp[6])*16 + HexCharToInt(temp[7]); 
		int byteCount = 4 * (maxSSID - minSSID + 1);
		m_setCmdList[m_setCmdIndex] = new unsigned char[byteCount+8];
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x7D;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x04;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = m_supportedMsgs[msgIndex].m_range[0];
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = m_supportedMsgs[msgIndex].m_range[1];
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = m_supportedMsgs[msgIndex].m_range[2];
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = m_supportedMsgs[msgIndex].m_range[3];
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;
		m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = 0x00;

		int data_SSID = -1;
		int data_level = -1;

		int mask = 0;
		unsigned char tempByte[4];
		for (int i=minSSID; i<= maxSSID; i++)
		{
			memset(tempByte, 0, 4);

			for (j=0; j<m_openCount; j++)
			{
				m_openedViews[j].GetCurrentMsgInfo(&data_SSID, &data_level);
				while(1)
				{
					if (data_SSID == i)
					{
						tempByte[data_level/8] |= (1<<(data_level%8));
						m_openedViews[j].GotoNextMsgInfoAndReturn(&data_SSID, &data_level);
						continue;
					}
					else if ((data_SSID < i) && (-1 != data_SSID))
					{
						m_openedViews[j].GotoNextMsgInfoAndReturn(&data_SSID, &data_level);
						continue;
					}
					else
					{					
						break;
					}
				}
			}
			
			for (j=0; j<4; j++)
			{
				//mask = HexCharToInt(m_supportedMsgs[msgIndex].m_data[(i-minSSID)*4*3 + j*3]) * 16
					//+ HexCharToInt(m_supportedMsgs[msgIndex].m_data[(i-minSSID)*4*3 + j*3 + 1]);
				mask = m_supportedMsgs[msgIndex].m_data[(i-minSSID)*4 + j];
				tempByte[j] &= mask;
				m_setCmdList[m_setCmdIndex][cmdBufferIndex++] = tempByte[j];
			}
		}
		m_setCmdLen[m_setCmdIndex] = cmdBufferIndex;		
		msgIndex++;
		m_setCmdIndex++;
	}
}

void DiagCommandGenerator::SetResponseOfLastCmd(const unsigned char *data, int dataLen)
{
	//ASSERT(data != NULL);
	//ASSERT(dataLen >= 0 );
	
	if ((dataLen >= 76) && (0x73 == data[0]) && (0x01 == data[4]))
	{
		GetSupportedLogItems(data, dataLen);
	}
	else if ((dataLen >= 2) && (0x7D == data[0]) && (0x01 == data[1]))
	{
		GetSupportedMsgSSIDs(data, dataLen);
	}
	else if ((dataLen >= 1) && (0x81 == data[0]))
	{
		GetSupportedEventIDs(data, dataLen);
	}
	else if ((dataLen >= 2) && (0x7D == data[0]) && (0x02 == data[1]))
	{
		GetSupportedMsgLevels(data, dataLen);
	}
	
}

void DiagCommandGenerator::SetLastCmdInDiagFormat(const unsigned char *data, int dataLen)
{
	//ASSERT(NULL != data);
	if ((0x60 != *data) && (0x82 != *data) && (0x73 != *data) && (0x7D != *data))
	{
		return;
	}

	if ( 0 == m_usableIndexInSetCmdList)
	{
		return;
	}

	//ASSERT((NULL == m_setCmdList[m_setCmdIndex]) && (NULL != m_setCmdList[m_setCmdIndex-1]));

	if (NULL == m_setCmdList[m_usableIndexInSetCmdList])
	{
		m_setCmdList[m_usableIndexInSetCmdList] = new unsigned char[EFS2_DIAG_WRITE_REQUEST_LEN];		
		m_curDiagWriteCmdCount++;
		m_setCmdList[m_usableIndexInSetCmdList][0] = 0x4B;
		m_setCmdList[m_usableIndexInSetCmdList][1] = 0x13;
		m_setCmdList[m_usableIndexInSetCmdList][2] = 0x05;
		m_setCmdList[m_usableIndexInSetCmdList][3] = 0x00;
		m_setCmdList[m_usableIndexInSetCmdList][4] = 0x00;
		m_setCmdList[m_usableIndexInSetCmdList][5] = 0x00;
		m_setCmdList[m_usableIndexInSetCmdList][6] = 0x00;
		m_setCmdList[m_usableIndexInSetCmdList][7] = 0x00;
		m_setCmdList[m_usableIndexInSetCmdList][8] = 0x00;
		m_setCmdList[m_usableIndexInSetCmdList][9] = (m_curDiagWriteCmdCount-1)*2;
		m_setCmdList[m_usableIndexInSetCmdList][10] = 0x00;
		m_setCmdList[m_usableIndexInSetCmdList][11] = 0x00;
		m_curDiagWriteCmdBufLeft = EFS2_DIAG_WRITE_REQUEST_LEN-12;
		m_setCmdLen[m_usableIndexInSetCmdList] = 12;
	}

	while (1)
	{
		if (dataLen <= m_curDiagWriteCmdBufLeft)
		{
			memcpy(m_setCmdList[m_usableIndexInSetCmdList]+EFS2_DIAG_WRITE_REQUEST_LEN-m_curDiagWriteCmdBufLeft, data, dataLen);
			m_curDiagWriteCmdBufLeft -= dataLen;
			m_setCmdLen[m_usableIndexInSetCmdList] += dataLen;
			break;
		}
		else
		{
			memcpy(m_setCmdList[m_usableIndexInSetCmdList]+EFS2_DIAG_WRITE_REQUEST_LEN-m_curDiagWriteCmdBufLeft, data, m_curDiagWriteCmdBufLeft);
			m_setCmdLen[m_usableIndexInSetCmdList] = EFS2_DIAG_WRITE_REQUEST_LEN;
			data += m_curDiagWriteCmdBufLeft;
			dataLen -= m_curDiagWriteCmdBufLeft;
			m_usableIndexInSetCmdList++;
			m_setCmdList[m_usableIndexInSetCmdList] = new unsigned char[EFS2_DIAG_WRITE_REQUEST_LEN];
			m_curDiagWriteCmdCount++;
			m_setCmdList[m_usableIndexInSetCmdList][0] = 0x4B;
			m_setCmdList[m_usableIndexInSetCmdList][1] = 0x13;
			m_setCmdList[m_usableIndexInSetCmdList][2] = 0x05;
			m_setCmdList[m_usableIndexInSetCmdList][3] = 0x00;
			m_setCmdList[m_usableIndexInSetCmdList][4] = 0x00;
			m_setCmdList[m_usableIndexInSetCmdList][5] = 0x00;
			m_setCmdList[m_usableIndexInSetCmdList][6] = 0x00;
			m_setCmdList[m_usableIndexInSetCmdList][7] = 0x00;
			m_setCmdList[m_usableIndexInSetCmdList][8] = 0x00;
			m_setCmdList[m_usableIndexInSetCmdList][9] = (m_curDiagWriteCmdCount-1)*2;
			m_setCmdList[m_usableIndexInSetCmdList][10] = 0x00;
			m_setCmdList[m_usableIndexInSetCmdList][11] = 0x00;
			m_curDiagWriteCmdBufLeft = EFS2_DIAG_WRITE_REQUEST_LEN-12;
			m_setCmdLen[m_usableIndexInSetCmdList] = 12;
		}
	}
	
	
}


void DiagCommandGenerator::GetSupportedLogItems(const unsigned char *data, int dataLen)
{
	int dataIndex = 16;
	int supportedLogItemsIndex = 0;
	for (dataIndex=16; (dataIndex <= 72)&&(dataIndex<dataLen); dataIndex+=4)
	{
		if ((0 != data[dataIndex]) || (0 != data[dataIndex+1]))
		{
			m_supportedLogItems[supportedLogItemsIndex].m_range = new unsigned char;
			m_supportedLogItems[supportedLogItemsIndex].m_range[0] = (dataIndex - 12) / 4;
			m_supportedLogItems[supportedLogItemsIndex].m_data = new unsigned char[2];
			m_supportedLogItems[supportedLogItemsIndex].m_data[0] = data[dataIndex];
			m_supportedLogItems[supportedLogItemsIndex].m_data[1] = data[dataIndex+1];
			supportedLogItemsIndex++;
		}
	}
}

void DiagCommandGenerator::GetSupportedMsgSSIDs(const unsigned char *data, int dataLen)
{
	int dataIndex = 8;
	int supportedMsgsIndex = 0;
	for (dataIndex=8; dataIndex<dataLen; dataIndex+=4)
	{
		m_supportedMsgs[supportedMsgsIndex].m_range = new unsigned char[4];
		memcpy(m_supportedMsgs[supportedMsgsIndex].m_range, data+dataIndex, 4);
		supportedMsgsIndex++;
	}
}

void DiagCommandGenerator::GetSupportedEventIDs(const unsigned char *data, int dataLen)
{
	m_supportedEventIDs.m_range = new unsigned char[2];
	memcpy(m_supportedEventIDs.m_range, data+4, 2);
	m_supportedEventIDs.m_data = new unsigned char[dataLen - 6];
	memcpy(m_supportedEventIDs.m_data, data+6, dataLen-6);
}

void DiagCommandGenerator::GetSupportedMsgLevels(const unsigned char * data, int dataLen)
{
	//ASSERT(m_msgQueryCmdIndex > 0);

	m_supportedMsgs[m_msgQueryCmdIndex-1].m_data = new unsigned char[dataLen-8];
	memcpy(m_supportedMsgs[m_msgQueryCmdIndex-1].m_data, data+8, dataLen-8);
}


int DiagCommandGenerator::HexCharToInt(char hexChar)
{
	if ((hexChar >= 'a') && (hexChar <= 'f'))
	{
		return hexChar - 'a' + 10;
	}
	else if ((hexChar >= 'A') && (hexChar <= 'F'))
	{
		return hexChar - 'A' + 10;
	}
	else
	{
		return hexChar - '0';
	}
}

int DiagCommandGenerator::HexStr2Int(const char *hexStr)
{
	int ret = 0;

	//ASSERT(hexStr[0] == '0');
	//ASSERT(hexStr[1] == 'x');
	hexStr += 2;
	while ('\0' != *hexStr)
	{
		ret = ret * 16 + HexCharToInt(*hexStr);
		hexStr++;
	}

	return ret;
}

