// DmcFile.h: interface for the DmcFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DMCFILE_H__0F730787_0797_4039_9567_AC4F19C21683__INCLUDED_)
#define AFX_DMCFILE_H__0F730787_0797_4039_9567_AC4F19C21683__INCLUDED_

/*
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
*/
#include <stdio.h>

#include <stdlib.h>

#include <assert.h>

#include <string.h>

#include <ctype.h>

#include "DmcFilePart.h"

const int SET_CMD_MAX = 50;
const int LOG_MAX = 20;
const int MSG_MAX = 30;
const int DIAG_WRITE_CMD_MAX = 20; //DIAG WRITE CMD最大数量限制
const int EFS2_DIAG_WRITE_REQUEST_LEN = 524;//一个DIAG WRITE CMD 的最大长度限制

const int CORE_DUMP = 0x1158;

const int USE_AUTO_CHOOSE = 0;
const int USE_MESSAGE_VIEW = 1;
const int USE_LOG_VIEW = 2;
const int USE_BOTH = 3;

const int MAX_OPEN_VIEWS = 10;

//const int QUERY_CMD_COUNT = 20;

//const int EquipmentIDs =0;
//const int SSIDs = 1;
//const int EventIDs = 2;

/*
class SupportedThings
{
protected:
	int m_nRange;
	int *m_pRangeData;
	char **m_data;
public:
	SupportedThings(char *data, int dataLen);
	virtual ~SupportedLogItems();
public:
	GetRangeNum();
private:
	SupportedThings(const SupportedLogItems &source);
	SupportedThings& operator =(const SupportedLogItems &source);
};
*/
class SupportedThings
{
public:
	unsigned char *m_range;
	unsigned char *m_data;
public:
	SupportedThings(){m_range=NULL;m_data=NULL;};
	~SupportedThings(){if(NULL!=m_data) delete[]m_data;if(NULL!=m_range) delete[]m_range;};
private:
	SupportedThings(const SupportedThings &source){};
	SupportedThings& operator =(const SupportedThings &source){return *this;};
};

class DiagCommandGenerator  
{
protected:
	char *m_dmcFileText;
	unsigned char *m_setCmdList[SET_CMD_MAX];//List of comand for setting propertys 用于设置属性的命令
	int m_setCmdLen[SET_CMD_MAX];
	//unsigned char *m_diagWriteCmdList[DIAG_WRITE_CMD_MAX];
	SupportedThings m_supportedLogItems[LOG_MAX];
	SupportedThings m_supportedMsgs[MSG_MAX];
	SupportedThings m_supportedEventIDs;

	int m_queryCmdIndex;
	int m_setCmdIndex;
	int m_msgQueryCmdIndex;

	int m_supportedLogItemsIndex;
	int m_supportedMsgsIndex;

	int m_needParseDmcFile;
	int m_isEFS2_DIAGClosed;

	int m_curDiagWriteCmdBufLeft;//diag write cmd is stored in m_setCmdList, after the set cmd
	int m_curDiagWriteCmdCount;//diag write cmd is stored in m_setCmdList, after the set cmd
	int m_usableIndexInSetCmdList;

	DmcFilePart m_messageView;
	DmcFilePart m_logView;
	DmcFilePart m_openedViews[MAX_OPEN_VIEWS];
	int m_openCount;

protected:
	void CreateSetCmdList();
	void ClearSetCmdList();
	void ParseEventIDFromFile();
	void ParseLogCodeAndOTALogCode();
	void ParseMsgInfoFromFile();

	void GetSupportedLogItems(const unsigned char *data, int dataLen);
	void GetSupportedMsgSSIDs(const unsigned char *data, int dataLen);
	void GetSupportedMsgLevels(const unsigned char * data, int dataLen);
	void GetSupportedEventIDs(const unsigned char *data, int dataLen);
public:
	void GetDmcFileText(const char *path, int configMode);
	void GetNextCommand(unsigned char *cmdBuffer, unsigned long *cmdLen);
	void SetLastCmdInDiagFormat(const unsigned char *data, int dataLen);
	void SetResponseOfLastCmd(const unsigned char *data, int dataLen);
	int HexCharToInt(char hexChar);
	int HexStr2Int(const char *hexStr);
public:
	DiagCommandGenerator();
	virtual ~DiagCommandGenerator();
private:
	DiagCommandGenerator(const DiagCommandGenerator &source);
	DiagCommandGenerator& operator =(const DiagCommandGenerator &source);
};

#endif // !defined(AFX_DMCFILE_H__0F730787_0797_4039_9567_AC4F19C21683__INCLUDED_)
