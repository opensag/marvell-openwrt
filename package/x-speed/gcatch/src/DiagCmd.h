#pragma once
#include "comm_def.h"

typedef void (*DIAGMAKECMD)(tx_pkt_buf_type *pRequest);
typedef struct _tagDiagCmdTable
{
  int charCode;
  DIAGMAKECMD  fnMakeCmd;
}DiagCmdTable;


class CDiagCmd
{
public:
  
  static CDiagCmd& GetInstance();
  bool SendCmd(int charCode,tx_pkt_buf_type* pRequest,rx_pkt_buf_type*pResponse);
  bool SendRawCmd(tx_pkt_buf_type * pRequest, rx_pkt_buf_type * pResponse);

  bool SendHexStrCmd(char *pHexStr,tx_pkt_buf_type* pRequest,rx_pkt_buf_type*pResponse);
  bool ReadResponse(rx_pkt_buf_type*pResponse);
  int SetRetryTime(int time);
private:
  CDiagCmd(); 
  bool MakeDiagCmd(int charCode,tx_pkt_buf_type * pRequest);
  void HexStr2ByteStream(char *pHexStr,tx_pkt_buf_type* pRequest);
  byte Hex2Byte(char chHigh,char chLow);
  void DealWithHDLC(tx_pkt_buf_type* pRequest);
  void SendData(tx_pkt_buf_type* pRequest,rx_pkt_buf_type*pResponse);
  bool CheckResponse(tx_pkt_buf_type* pRequest,rx_pkt_buf_type*pResponse);
  bool CheckReceivedData(rx_pkt_buf_type*pResponse);
  unsigned long tick_count();
  
  void ReadFromCurBuf(rx_pkt_buf_type*pResponse,int pack_end_pos);
  void ReadFromDiagPort(rx_pkt_buf_type*pResponse);
  int ScanOnePackFromBuf(rx_pkt_buf_type *pbuf);
  
private:
  rx_pkt_buf_type m_pkt_cur_buf;// to store data that read from diag port
  rx_pkt_buf_type m_pkt_tmp_buf;// to read diag port
  int	m_retry_time;
};

