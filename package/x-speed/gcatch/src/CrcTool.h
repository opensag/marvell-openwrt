#pragma once
#include "comm_def.h"
class CCrcTool 
{
public:
  dword MakeCrc(byte* pData,int dataLen);
  bool CheckCrc(byte* pData,int dataLen);
};