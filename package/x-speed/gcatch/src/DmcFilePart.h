#ifndef DMC_FILE_PART_H
#define DMC_FILE_PART_H

#include <stdio.h>

#include <stdlib.h>

#include <assert.h>

#include <string.h>

#include <ctype.h>

class DmcFilePart
{
private:
	char *m_curEventIDLoc;
	char *m_curLogCodeLoc;
	char *m_curOTALogCodeLoc;
	char *m_curMsgInfoLoc;
	char *m_partEnd;
	char *m_eventIDEnd;
	char *m_LogCodeEnd;
	char *m_OTALogCodeEnd;
	char *m_msgInfoEnd;
public:
	DmcFilePart();
	int Initialize(const char *pFileText, const char *pStartStr, const char *pEndStr);
	int IsEventIDUsed();
	int GetCurrentEventID();
	int GetCurrentEventIDAndGotoNext();
	int GotoNextEventIDAndReturn();
	int GetCurrentLogCode();
	int GetCurrentLogCodeAndGotoNext();
	int GotoNextLogCodeAndReturn();
	int GetCurrentOTALogCode();
	int GetCurrentOTALogCodeAndGotoNext();
	int GotoNextOTALogCodeAndReturn();
	void GetCurrentMsgInfo(int *pSSID, int *pLevel);
	void GetCurrentMsgInfoAndGotoNext(int *pSSID, int *pLevel);
	void GotoNextMsgInfoAndReturn(int *pSSID, int *pLevel);
	int HexCharToInt(char hexChar);
	int HexStr2Int(const char *hexStr);
	void SkipCrLf(char **p_str);

};
#endif
