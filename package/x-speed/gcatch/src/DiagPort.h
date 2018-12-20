#pragma once
#include "unistd.h"
#include "comm_def.h"
#include <fcntl.h>
class CDiagPort
{
public:
  ~CDiagPort(); 
  static CDiagPort& GetInstance();
  bool Open(char *diagPort);
  void Close();
  int Write(byte *pData,dword dataLen);
  int Read(byte *pData,dword dataLen);
  int ReadWait();
  
private:
  CDiagPort();
 
  
private:
  int m_fields;
};