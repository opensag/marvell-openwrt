//#include "stdafx.h"
//#include "bpLog.h"
#include "DmcFilePart.h"

DmcFilePart::DmcFilePart()
{
	m_curEventIDLoc = NULL;
	m_curLogCodeLoc = NULL;
	m_curOTALogCodeLoc = NULL;
	m_curMsgInfoLoc = NULL;
	m_partEnd = NULL;
	m_eventIDEnd = NULL;
	m_LogCodeEnd = NULL;
	m_OTALogCodeEnd = NULL;
	m_msgInfoEnd = NULL;
}

int DmcFilePart::Initialize(const char * pFileText, const char * pStartStr, const char * pEndStr)
{
	char *partStart = NULL;
	partStart = (char *)strstr(pFileText, pStartStr);
	m_partEnd = strstr(partStart, pEndStr);
	if ((NULL == partStart) || (NULL == m_partEnd))
	{
		return 0;
	}

	m_curEventIDLoc = strstr(partStart, "<EventIDs>");
	m_eventIDEnd = strstr(partStart, "</EventIDs>");
	if ((m_curEventIDLoc > m_partEnd) || (m_curEventIDLoc < partStart))
	{
		m_curEventIDLoc = NULL;
		m_eventIDEnd = NULL;
	}
	else
	{
		m_curEventIDLoc += strlen("<EventIDs>");
		SkipCrLf(&m_curEventIDLoc);
		if (m_curEventIDLoc >= m_eventIDEnd)
		{
			m_curEventIDLoc = NULL;		
		}
	}

	m_curLogCodeLoc = strstr(partStart, "<LogCodes>");
	m_LogCodeEnd = strstr(partStart, "</LogCodes>");
	if ((m_curLogCodeLoc > m_partEnd) || (m_curLogCodeLoc < partStart))
	{
		m_curLogCodeLoc = NULL;
		m_LogCodeEnd = NULL;
	}
	else
	{
		m_curLogCodeLoc += strlen("<LogCodes>");
		SkipCrLf(&m_curLogCodeLoc);
		if (m_curLogCodeLoc >= m_LogCodeEnd)
		{
			m_curLogCodeLoc = NULL;
		}
	}

	m_curOTALogCodeLoc = strstr(partStart, "<OTATypes>");
	m_OTALogCodeEnd = strstr(partStart, "</OTATypes>");	
	if ((m_curOTALogCodeLoc > m_partEnd) || (m_curOTALogCodeLoc < partStart))
	{
		m_curOTALogCodeLoc = NULL;
		m_OTALogCodeEnd = NULL;
	}
	else
	{
		m_curOTALogCodeLoc += strlen("<OTATypes>");
		SkipCrLf(&m_curOTALogCodeLoc);
		if (m_curOTALogCodeLoc >= m_OTALogCodeEnd)
		{
			m_curOTALogCodeLoc = NULL;
		}
	}

	m_curMsgInfoLoc = strstr(partStart, "<MessageLevels>");
	m_msgInfoEnd = strstr(partStart, "</MessageLevels>");
	if ((m_curMsgInfoLoc > m_partEnd) || (m_curMsgInfoLoc < partStart))
	{
		m_curMsgInfoLoc = NULL;
		m_msgInfoEnd = NULL;
	}
	else
	{
		m_curMsgInfoLoc += strlen("<MessageLevels>");
		SkipCrLf(&m_curMsgInfoLoc);
		if (m_curMsgInfoLoc >= m_msgInfoEnd)
		{
			m_curMsgInfoLoc = NULL;
		}
	}

	return 1;
}

int DmcFilePart::GetCurrentEventID()
{
	if (NULL == m_curEventIDLoc)
	{
		return -1;
	}

	return atoi(m_curEventIDLoc);
}

int DmcFilePart::GetCurrentEventIDAndGotoNext()
{

	if (NULL == m_curEventIDLoc)
	{
		return -1;
	}

	int ret = GetCurrentEventID();
	
	m_curEventIDLoc += 6;
	SkipCrLf(&m_curEventIDLoc);
	if (m_curEventIDLoc >= m_eventIDEnd)
	{
		m_curEventIDLoc = NULL;		
	}

	return ret;
}

int DmcFilePart::GotoNextEventIDAndReturn()
{
	if (NULL == m_curEventIDLoc)
	{
		return -1;
	}
	m_curEventIDLoc += 6;
	SkipCrLf(&m_curEventIDLoc);
	if (m_curEventIDLoc >= m_eventIDEnd)
	{
		m_curEventIDLoc = NULL;		
	}

	return GetCurrentEventID();
}

int DmcFilePart::GetCurrentLogCode()
{
	if (NULL == m_curLogCodeLoc)
	{
		return -1;
	}

	char tempBuffer[7];
	strncpy(tempBuffer, m_curLogCodeLoc, 6);
	tempBuffer[6] = '\0';
	return HexStr2Int(tempBuffer);
}

int DmcFilePart::GetCurrentLogCodeAndGotoNext()
{
	if (NULL == m_curLogCodeLoc)
	{
		return -1;
	}

	int ret = GetCurrentLogCode();

	m_curLogCodeLoc += 7;
	SkipCrLf(&m_curLogCodeLoc);
	if (m_curLogCodeLoc >= m_LogCodeEnd)
	{
		m_curLogCodeLoc = NULL;
	}
	return ret;
}

int DmcFilePart::GotoNextLogCodeAndReturn()
{
	if (NULL == m_curLogCodeLoc)
	{
		return -1;
	}
	
	m_curLogCodeLoc += 7;
	SkipCrLf(&m_curLogCodeLoc);
	if (m_curLogCodeLoc >= m_LogCodeEnd)
	{
		m_curLogCodeLoc = NULL;
	}
	return GetCurrentLogCode();
}

int DmcFilePart::GetCurrentOTALogCode()
{
	if (NULL == m_curOTALogCodeLoc)
	{
		return -1;
	}

	char tempBuffer[7];
	strncpy(tempBuffer, m_curOTALogCodeLoc, 6);
	tempBuffer[6] = '\0';
	return HexStr2Int(tempBuffer);
}

int DmcFilePart::GetCurrentOTALogCodeAndGotoNext()
{
	if (NULL == m_curOTALogCodeLoc)
	{
		return -1;
	}

	int ret = GetCurrentOTALogCode();

	char *temp = strchr(m_curOTALogCodeLoc, ',');
	if (NULL != temp) temp++;
	SkipCrLf(&temp);
	m_curOTALogCodeLoc = (temp >= m_OTALogCodeEnd) ? NULL : temp;

	return ret;
}

int DmcFilePart::GotoNextOTALogCodeAndReturn()
{
	if (NULL == m_curOTALogCodeLoc)
	{
		return -1;
	}

	char *temp = strchr(m_curOTALogCodeLoc, ',');
	if (NULL != temp) temp++;
	SkipCrLf(&temp);
	m_curOTALogCodeLoc = (temp >= m_OTALogCodeEnd) ? NULL : temp;
	return GetCurrentOTALogCode();

}

void DmcFilePart::GetCurrentMsgInfo(int * pSSID, int * pLevel)
{
	//ASSERT(NULL != pSSID);
	//ASSERT(NULL != pLevel);
	if (NULL == m_curMsgInfoLoc)
	{
		*pSSID = -1;
		*pLevel = -1;
		return;
	}

	*pSSID = atoi(m_curMsgInfoLoc);
	
	char tempBuffer[3];
	strncpy(tempBuffer, strchr(m_curMsgInfoLoc, '/')+1, 2);
	tempBuffer[2] = '\0';
	*pLevel = atoi(tempBuffer);
}


void DmcFilePart::GetCurrentMsgInfoAndGotoNext(int * pSSID, int * pLevel)
{
	if ((pSSID != NULL) && (pLevel != NULL))
	{
		if (NULL == m_curMsgInfoLoc)
		{
			*pSSID = -1;
			*pLevel = -1;
			return;
		}

		*pSSID = atoi(m_curMsgInfoLoc);
		
		char tempBuffer[3];
		strncpy(tempBuffer, strchr(m_curMsgInfoLoc, '/')+1, 2);
		tempBuffer[2] = '\0';
		*pLevel = atoi(tempBuffer);
	}
	else
	{
		if (NULL == m_curMsgInfoLoc)
		{
			return;
		}
	}
	
	m_curMsgInfoLoc += 9;
	SkipCrLf(&m_curMsgInfoLoc);
	if (m_curMsgInfoLoc >= m_msgInfoEnd)
	{
		m_curMsgInfoLoc = NULL;
	}
}

void DmcFilePart::GotoNextMsgInfoAndReturn(int *pSSID, int *pLevel)
{
	GetCurrentMsgInfoAndGotoNext(NULL, NULL);
	GetCurrentMsgInfo(pSSID, pLevel);
}

int DmcFilePart::HexStr2Int(const char *hexStr)
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

int DmcFilePart::HexCharToInt(char hexChar)
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

int DmcFilePart::IsEventIDUsed()
{
	return ((NULL == m_curEventIDLoc) ? 0 : 1);
}

//Added for new dmc file begin
void DmcFilePart::SkipCrLf(char **p_str)
{
	if ((NULL == p_str) || (NULL == *p_str)) return;
	while ((0x0d == **p_str) || (0x0a == **p_str))
	{
		(*p_str)++;
	}
}
//Added for new dmc file end

