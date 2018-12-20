#include "DiagCmd.h"
#include "CrcTool.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "DiagPort.h"
#include <sys/time.h>

void MakeDiagCmd0(tx_pkt_buf_type *pRequest);
void MakeDiagCmd1(tx_pkt_buf_type *pRequest);
void MakeDiagCmd2(tx_pkt_buf_type *pRequest);
void MakeDiagCmd3(tx_pkt_buf_type *pRequest);

static const DiagCmdTable g_DiagCmdTable[]=
{
  {0,MakeDiagCmd0},
  {1,MakeDiagCmd1},
  {2,MakeDiagCmd2},
  {3,MakeDiagCmd3}
};

void MakeDiagCmd0(tx_pkt_buf_type *pRequest)
{
  CCrcTool crcTool; 
  pRequest->buf[0]=0;
  pRequest->length=1;
  
  crcTool.MakeCrc(pRequest->buf,pRequest->length);
}
void MakeDiagCmd1(tx_pkt_buf_type *pRequest)
{
}
void MakeDiagCmd2(tx_pkt_buf_type *pRequest)
{
}
void MakeDiagCmd3(tx_pkt_buf_type *pRequest)
{
}

CDiagCmd::CDiagCmd()
{
  memset(&m_pkt_cur_buf,0,sizeof(rx_pkt_buf_type));
  memset(&m_pkt_tmp_buf,0,sizeof(rx_pkt_buf_type));
  m_retry_time= 3;
}

CDiagCmd& CDiagCmd::GetInstance()
{
  static CDiagCmd diagCmd;
  return diagCmd;
}

bool CDiagCmd::SendHexStrCmd(char *pHexStr,tx_pkt_buf_type* pRequest,rx_pkt_buf_type*pResponse)
{
  bool bret = false;
  CCrcTool crcTool;
  HexStr2ByteStream(pHexStr,pRequest);
  pRequest->length = crcTool.MakeCrc(pRequest->buf,pRequest->length);
  DealWithHDLC(pRequest);
  
 // printf("Send: ");
 // for(int i=0;i<pRequest->length;i++)
 // printf("%02X ",pRequest->buf[i]);
 // printf("\n");
  
  SendData(pRequest,pResponse);
  
  if(pResponse->length>0)bret=true;
  
  return bret;
}

bool CDiagCmd::SendRawCmd(tx_pkt_buf_type * pRequest, rx_pkt_buf_type * pResponse)
{
  bool bret = false;
  CCrcTool crcTool;
  //HexStr2ByteStream(pHexStr,pRequest);
  pRequest->length = crcTool.MakeCrc(pRequest->buf,pRequest->length);
  DealWithHDLC(pRequest);
  
 // printf("Send: ");
 // for(int i=0;i<pRequest->length;i++)
 // printf("%02X ",pRequest->buf[i]);
 // printf("\n");
  
  SendData(pRequest,pResponse);
  
  if(pResponse->length>0)bret=true;
  
  return bret;
}


void CDiagCmd::HexStr2ByteStream(char *pHexStr,tx_pkt_buf_type* pRequest)
{
  int i,j;
  int hexStrlen = strlen(pHexStr);
  char *ptmpStr = new char[hexStrlen+1+1];//maybe add extra '0'
  
  for(i=0;i<hexStrlen;i++)//filt the start blank or "0x"
  {
    if(pHexStr[i]!=' ')
    {
      if(pHexStr[i]=='0' &&( pHexStr[i+1]=='x'||pHexStr[i+1]=='X'))
      {
	i+=2;
      }
      
      break;
    }
  }
  for(j=0;i<hexStrlen;i++)
  {
    if(pHexStr[i]!=' ')ptmpStr[j++]=pHexStr[i];
  }
  ptmpStr[j] = 0;
  
  if(strlen(ptmpStr)%2==1)
  {
    memmove(ptmpStr+1,ptmpStr,strlen(ptmpStr)+1);
    ptmpStr[0] = '0';
  }
  
 
  for(j=0,i=0;i<strlen(ptmpStr);j++,i+=2)
  {    
    pRequest->buf[j] = Hex2Byte(ptmpStr[i],ptmpStr[i+1]);
  }
  pRequest->length = j;
}

byte CDiagCmd::Hex2Byte(char chHigh,char chLow)
{
   byte dest;
   if (chHigh > '9')
   {
     dest = ((tolower(chHigh) - 'a') + 10) << 4;   
   }
   else
   { 
     dest = (chHigh - '0') << 4;
   }
   
   if (chLow > '9')
   {
     dest |= ((tolower(chLow) - 'a') + 10);   
   }
   else
   { 
     dest |= (chLow - '0');
   }
   return dest;         
}


void CDiagCmd::DealWithHDLC(tx_pkt_buf_type* pRequest)
{
  
    
	//deal with HDLC
	byte * buffer = new byte[pRequest->length * 2];
	//byte buffer[tx_packet.length * 2];
	int i;
	dword ValidLen=0;
	for( i = 0; i < pRequest->length; i++)
	{
		if( (pRequest->buf[i] == ASYNC_HDLC_FLAG) || (pRequest->buf[i] == ASYNC_HDLC_ESC) )
		{
			buffer[ValidLen++] = ASYNC_HDLC_ESC;
			buffer[ValidLen++] = pRequest->buf[i] ^ (byte)ASYNC_HDLC_ESC_MASK;
		}
		else
			buffer[ValidLen++] = pRequest->buf[i];
	}

	pRequest->length=0;

	for(i=0; i < ValidLen; i++)
		pRequest->buf[pRequest->length++]=buffer[i];
	pRequest->buf[pRequest->length++] = ASYNC_HDLC_FLAG;
	delete buffer;
	//deal with HDLC over
}


void CDiagCmd::SendData(tx_pkt_buf_type* pRequest,rx_pkt_buf_type*pResponse)
{
  int tx_packet_ret_len;
  int i;
  unsigned long wait_ms = 3000;
  unsigned long tBegin = 0, tEnd = 0;
  unsigned long tmp_len = 0;	
  
  for(i = 0;i<m_retry_time;i++)
  {
    //write data to port
    tx_packet_ret_len = CDiagPort::GetInstance().Write(pRequest->buf,pRequest->length);
    
    if(tx_packet_ret_len != pRequest->length)
    {
      //printf("Error: Port cannot send desired data to the phone\n");
      continue;
    }
    
    tBegin=tick_count();
    do
    {
	tEnd = tick_count();
	if(ReadResponse(pResponse) && CheckResponse(pRequest,pResponse))
	{
	  break;
	}
	pResponse->length = 0;
    }while((tEnd-tBegin) <= wait_ms);

    if((tEnd-tBegin) > wait_ms)continue;
    
    break;
  }
		
}

/*
bool CDiagCmd::ReadResponse(rx_pkt_buf_type*pResponse)
{
  bool bret = false;
  unsigned long wait_ms = 5000;
  unsigned long tBegin = 0, tEnd = 0;
  unsigned long tmp_len = 0;	
   
  tBegin=tick_count();
  //get rx packet length
  do
  {
    pResponse->length = CDiagPort::GetInstance().ReadWait();
    usleep(3000);
    tmp_len = CDiagPort::GetInstance().ReadWait();
    if(pResponse->length >= 1 && tmp_len == pResponse->length)break;
				
    tEnd=tick_count();
  }while((tEnd-tBegin) <= wait_ms);

  if( pResponse->length < 1 )
  {
    printf("Error: Port cannot receive desired data from the phone\n");
    return bret;
  }
  else
  {
    if(pResponse->length > DIAG_MAX_RX_PKT_SIZ)
    {
      printf("Error: The received data from the phone is too large\n");
      return bret;			  
    }
  }
  //read data from port
  pResponse->length = CDiagPort::GetInstance().Read(pResponse->buf,sizeof(pResponse->buf));
  bret = true;
  
  return bret;
  
}
*/
bool CDiagCmd::ReadResponse(rx_pkt_buf_type*pResponse)
{
  bool bret = false;
  int endPos =  ScanOnePackFromBuf(&m_pkt_cur_buf);
  //////////////
  //printf("current buffer length = %d\n", m_pkt_cur_buf.length);
  //////////////
  if(endPos>-1)
  {
    ReadFromCurBuf(pResponse,endPos);
  }
  else
  {
    ReadFromDiagPort(pResponse);
  }
  
  if(pResponse->length >0)
  {
    bret =CheckReceivedData(pResponse);

    //printf("current item length after check = %d\n", pResponse->length);
#ifdef LINUX_LOG_TOOL_DEBUG
    if (!bret)
    {
        printf("Check Failed!!packet length=%d\n, first byte=0x%2x", pResponse->length, pResponse->buf[0]);
    }
#endif
  }
  return bret;
}
void CDiagCmd::ReadFromCurBuf(rx_pkt_buf_type*pResponse,int pack_end_pos)
{
  pResponse->length = pack_end_pos+1;
  memcpy(pResponse->buf,m_pkt_cur_buf.buf,pResponse->length);
  
  m_pkt_cur_buf.length -= pResponse->length;
  
  memmove(m_pkt_cur_buf.buf,m_pkt_cur_buf.buf+pResponse->length,m_pkt_cur_buf.length);// remove the read data,move the left data forward 
}

void CDiagCmd::ReadFromDiagPort(rx_pkt_buf_type*pResponse)
{
  
  unsigned long wait_ms = 5000;
  unsigned long tBegin = 0, tEnd = 0;
  unsigned long tmp_len = 0;	
  
  pResponse->length = 0;
   
  tBegin=tick_count();
  //get rx packet length
  do
  {
    m_pkt_tmp_buf.length = CDiagPort::GetInstance().ReadWait();
    usleep(1000);
    tmp_len = CDiagPort::GetInstance().ReadWait();
    if(m_pkt_tmp_buf.length >= 1 && tmp_len == m_pkt_tmp_buf.length)break;
				
    tEnd=tick_count();
  }while((tEnd-tBegin) <= wait_ms);

  if( m_pkt_tmp_buf.length < 1 )
  {
   // printf("Error: Port cannot receive desired data from the phone\n");
    return;
  }
  else
  {
    if(m_pkt_tmp_buf.length > DIAG_MAX_RX_PKT_SIZ)
    {
    //  printf("Error: The received data from the phone is too large\n");	  
      return;
    }
  }
  //read data from port
  m_pkt_tmp_buf.length = CDiagPort::GetInstance().Read(m_pkt_tmp_buf.buf,sizeof(m_pkt_tmp_buf.buf));
  
  //int endPos1 = ScanOnePackFromBuf(m_pkt_cur_buf);
  int endPos = ScanOnePackFromBuf(&m_pkt_tmp_buf);
  
  if(endPos==-1)
  {
    //printf("data lost because endPos equals to -1, temBuffer[0]=%d", m_pkt_tmp_buf.buf[0]);
    //printf("tmpBuffer length = %d", m_pkt_tmp_buf.length);

    //m_pkt_cur_buf.length = 0;//throw old data
    if ((0 < m_pkt_tmp_buf.length) && (0x7E == m_pkt_tmp_buf.buf[0]))
    {
        //printf("return new packet\n");
        m_pkt_cur_buf.buf[m_pkt_cur_buf.length] = 0x7E;
        m_pkt_cur_buf.length++;
        ReadFromCurBuf(pResponse,m_pkt_cur_buf.length-1);
        memmove(m_pkt_cur_buf.buf, m_pkt_tmp_buf.buf+1, m_pkt_tmp_buf.length-1);// copy left data
        m_pkt_cur_buf.length = m_pkt_tmp_buf.length-1;
    }
    else if ((m_pkt_cur_buf.length+m_pkt_tmp_buf.length) > DIAG_MAX_RX_PKT_SIZ)
    {
        printf("data lost because data is too long and no endPos, need memory %d Bytes\n", m_pkt_cur_buf.length+m_pkt_tmp_buf.length);
        m_pkt_cur_buf.length = 0;//throw old data        
    }
    else if (0 < m_pkt_tmp_buf.length)
    {
        //printf("data copied!\n");
        memcpy(m_pkt_cur_buf.buf+m_pkt_cur_buf.length, m_pkt_tmp_buf.buf, m_pkt_tmp_buf.length);
        m_pkt_cur_buf.length += m_pkt_tmp_buf.length;
    }
    else
    {
        printf("received data length= %d\n", m_pkt_tmp_buf.length);
    }
  }
  else
  {
    if((m_pkt_cur_buf.length+endPos+1) > DIAG_MAX_RX_PKT_SIZ)
    {
    printf("data lost because data is too long, need memory %d Bytes\n", m_pkt_cur_buf.length+endPos+1);
      m_pkt_cur_buf.length = 0;//throw old data
    }
    else
    {
      memcpy(m_pkt_cur_buf.buf+m_pkt_cur_buf.length,m_pkt_tmp_buf.buf,endPos+1);//make cur buffer a whole pack data
      m_pkt_cur_buf.length+=endPos+1;
      ReadFromCurBuf(pResponse,m_pkt_cur_buf.length-1);
      memmove(m_pkt_cur_buf.buf,m_pkt_tmp_buf.buf+endPos+1,m_pkt_tmp_buf.length-(endPos+1));// copy left data   
      m_pkt_cur_buf.length = m_pkt_tmp_buf.length-(endPos+1);
    }
  } 
  

}

int CDiagCmd::ScanOnePackFromBuf(rx_pkt_buf_type *pbuf)
{
  int i;
  for(i=0;i<pbuf->length;i++)
  {
    if(pbuf->buf[i]==0x7e)break;
  }
  
  if(i==0 || i==pbuf->length)
  {
    i = -1;
  }
  return i;
}

bool CDiagCmd::CheckResponse(tx_pkt_buf_type* pRequest,rx_pkt_buf_type*pResponse)
{

  bool bret = false;
  if((pResponse->buf[0]==pRequest->buf[0]) || ((pResponse->buf[0] == 0x13) && (pResponse->buf[1] == pRequest->buf[0])))
  {
   /* byte * tmpbuf;
    byte * tmpbuf2;
    unsigned short ValidLen = 0, i = 0;
    unsigned long RxLength = pResponse->length;
	
    tmpbuf = new byte[pResponse->length];
	
    for(ValidLen = 0; i < pResponse->length; i++)
    {
      if( pResponse->buf[i] == ASYNC_HDLC_ESC)
      {
	tmpbuf[ValidLen++] = pResponse->buf[++i] ^ ASYNC_HDLC_ESC_MASK;
      }
      else
      {
	tmpbuf[ValidLen++] = pResponse->buf[i];
      }
			        
    }
     
    if(tmpbuf[ValidLen-1] != ASYNC_HDLC_FLAG)
    {
	printf("Error: The received data is wrong without end flag(0x7e)\n");
        delete tmpbuf;
        return bret;
    }
			
    pResponse->length = 0;
    for(i=0; i < ValidLen-1; i++)pResponse->buf[pResponse->length++] = tmpbuf[i];
    delete tmpbuf;

    CCrcTool crctool;
    if(!crctool.MakeCrc(pResponse->buf,pResponse->length))
    {
	 printf("Error: The received data is wrong with wrong CRC Checksum\n");
	 return bret;
    }
    */
   // printf("Recv: ");
   // for(int i=0;i<pResponse->length;i++)printf("%02X ",pResponse->buf[i]);
   // printf("7E\n");
    bret = true;
			
  }

  else
  {
    /*
    if(pResponse->buf[0] == 0x13)printf("Bad command\n");
    else if(pResponse->buf[0] == 0x14)printf("The Request packet has invalid or inconsistent parameters\n");
    else if(pResponse->buf[0] == 0x15)printf("The Request packet has an invalid length\n");
    else if(pResponse->buf[0] == 0x18)printf("the DMSS isn't in a particular mode\n");
    else if(pResponse->buf[0] == 0x42)printf("the correct SPC has not yet been entered to unlock service programming\n");
    else if(pResponse->buf[0] == 0x47)printf("the correct Security Password has not yet been entered to unlock the phone\n");
    *///else printf("unknow reason\n");
    
  }
  return bret;
}

bool CDiagCmd::CheckReceivedData(rx_pkt_buf_type*pResponse)
{
  bool bret = false;
  byte * tmpbuf;
  unsigned short ValidLen = 0, i = 0;
  unsigned long RxLength = pResponse->length;
	
  tmpbuf = new byte[pResponse->length];
	
  for(ValidLen = 0; i < pResponse->length; i++)
  {
    if( pResponse->buf[i] == ASYNC_HDLC_ESC)
    {
      tmpbuf[ValidLen++] = pResponse->buf[++i] ^ ASYNC_HDLC_ESC_MASK;
    }
    else
    {
	tmpbuf[ValidLen++] = pResponse->buf[i];
    }
			        
  }
     
  if(tmpbuf[ValidLen-1] != ASYNC_HDLC_FLAG)
  {
        delete tmpbuf;
        return bret;
  }
			
  pResponse->length = 0;
  for(i=0; i < ValidLen-1; i++)pResponse->buf[pResponse->length++] = tmpbuf[i];
  delete tmpbuf;

  CCrcTool crctool;
  if(!crctool.CheckCrc(pResponse->buf,pResponse->length))
  {
#ifdef LINUX_LOG_TOOL_DEBUG
    printf("Error: The received data is wrong with wrong CRC Checksum\n");
#endif
    return bret;
  }
  
  pResponse->length-=2;//remove crc two bytes
  bret = true;
  
  return bret;
}

unsigned long CDiagCmd::tick_count()
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday( &tv, &tz);

    unsigned long ms = ( unsigned long )tv.tv_sec;
    unsigned long us = ( unsigned long )tv.tv_usec;

    return ( unsigned long )( ms * 1000 + us / 1000 );
}

int  CDiagCmd::SetRetryTime(int time)
{
  int old_retry_time = m_retry_time;
  m_retry_time = time;
  return old_retry_time;
}
