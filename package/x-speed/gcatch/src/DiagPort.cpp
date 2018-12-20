#include "DiagPort.h"
#include <sys/time.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>



CDiagPort& CDiagPort::GetInstance()
{
  static CDiagPort diagPort;
  return diagPort;
}
CDiagPort::CDiagPort ()
{
  m_fields = -1;
}
CDiagPort::~CDiagPort ()
{
  close(m_fields);
}
bool CDiagPort::Open (char *diagPort)
{
  bool bret = false;
  struct termios attr;
  int flags;

  m_fields = open(diagPort, O_RDWR | O_NONBLOCK | O_NOCTTY | O_TRUNC);

  if (m_fields < 0)
    {
      //printf ("Open Diag Port Error\n");
      return bret;
    }

  if (tcgetattr (m_fields, &attr) < 0)
  {
      close (m_fields);
      return bret;  
  }

  //attr.c_iflag = IXON|IXOFF;
  attr.c_iflag &= ~(IXON | IXOFF | INLCR | IGNCR | ICRNL);
  attr.c_oflag = 0;
  //attr.c_oflag &= ~OPOST;
  attr.c_cflag &= ~(CSIZE | CFLAGS_TO_CLEAR | CFLAGS_HARDFLOW);
  attr.c_cflag |= (CS8 | CFLAGS_TO_SET);
  attr.c_cflag |= CFLAGS_HARDFLOW;
  attr.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ISIG);

  attr.c_cc[VMIN] = 1;
  attr.c_cc[VTIME] = 0;

  cfsetispeed (&attr, BAUDRATE);
  cfsetospeed (&attr, BAUDRATE);

  if (tcsetattr (m_fields, TCSANOW, &attr) < 0)
  {
    close (m_fields);
    return bret;
  }


  flags = fcntl (m_fields, F_GETFL, 0);
  if (flags < 0)
  {
    close (m_fields);
    return bret;
  }

  if (fcntl (m_fields, F_SETFL, flags & ~O_NONBLOCK) < 0)
  {
    close (m_fields);
    return bret;
  }
  bret = true;
  return bret;
}
void CDiagPort::Close ()
{
  if(m_fields!=-1)
  {
    close(m_fields);
    m_fields = -1;
  }
}

int CDiagPort::Write (byte *pData,dword dataLen)
{
    if (m_fields < 0)
    {
        return -1;
    }

    struct timeval now, end, timeout;
    fd_set fds;
    gettimeofday(&end, NULL);

    int result=0;

    gettimeofday(&now, NULL);

    FD_ZERO(&fds);
    FD_SET(m_fields, &fds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 5000;

    result = select(m_fields+1, NULL, &fds, NULL, NULL);
    if (result<= 0)
    {
        return -1;
    }

    if (m_fields > 0)
    {
        int n = write(m_fields, pData, dataLen);
        return n;
    }
    return -1;
}

int CDiagPort::Read (byte *pData,dword dataLen)
{
    int nread = 0, n = 0;
    char *pr = (char*)NULL;
 
    int length=ReadWait();
    int temp_length = length;

    //printf("data availiable: %d\n", length);

    if (length>=dataLen)
    {
        length=dataLen;
    }
    
    if (m_fields > 0)
    {
        do
        {
            if ((n = read(m_fields, &((char *)pData)[nread], (length) - nread)) == -1)
	    {
	        if (errno == EINTR)
                    continue;
                else
                    return -1;
            }
            if (n == 0)
            {
                break;
            }
            nread +=n;          
        }while (nread < length );
/*		
	if(temp_length>1024)
	{
            printf("temp_length>1024, buffer cleared, left = %d, ", ReadWait());
            tcflush(m_fields,TCIFLUSH);
	}
*/
        return nread;
    }

    return -1;
}
int CDiagPort::ReadWait ()
{
    int char_waiting=1;
    ioctl(m_fields,FIONREAD,&char_waiting);

    return char_waiting;
}
