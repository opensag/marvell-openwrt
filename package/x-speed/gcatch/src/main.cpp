#include "DiagCmd.h"
#include "DiagPort.h"
#include "comm_def.h"
#include "stdio.h"
#include "inifile.h"
#include "stdio.h"
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

#include "DiagCommandGenerator.h"
void send_diag_set_cmd_new(char *dmcFilePath, int howToConfig);
#define MAX_DMC_FILE_PATH_LEN 200


#define MAX_KEY_NAME 200
#define MAX_KEY_VALUE  4*1024
#define MAX_DIAG_PORT_NAME 30
void get_new_file_name(char *str_file_name, const char *tail);
void send_diag_stop_cmd();
void* start_write_log_file(void *arg);
void* start_read_port(void *arg);

bool GetDiagPort(char *pstrDiag);

bool g_stop_log = false;

char store_path[256] = {0};
bool print_log = true;
raw_data_buf raw_buffer[RAW_DATA_BUF_COUNT];
raw_data_buf *buf_for_port[RAW_DATA_BUF_COUNT];
raw_data_buf *buf_for_file[RAW_DATA_BUF_COUNT];
int read_port_thread_exit = 1;
int write_file_thread_exit = 1;
int g_total_read_from_port = 0;
int g_total_write_to_file = 0;
int g_max_file_size = 200*1024*1024;
unsigned char g_one_file_only = 0;

void* simple_thread(void *arg)
{
	int i = 10;
	printf("Simple thread running...\n");
	while (i--)
	{
		printf("time left: %ds\n", i);
	}

	printf("Simple thread end\n");
	return 0;
}

int main(int argc, char **argv) {
  char str_name[256]={0};
  char diag_name[MAX_DIAG_PORT_NAME]={0};
  char *version="LINUX_BP_LOGV1.00.03";
  int res = 0;
  int i = 0;
  
  printf("diaglog version:%s\n",version);

  if (0 == argc%2)
  {
      printf("Wrong format! Enter command like below:\n");
      printf("%s -port [diag port name] -path [path to store] -max [max size in M]\n", argv[0]);
      printf("-port  -path -max is optional\n");
      printf("Example: %s -port /dev/ttyUSB0 -path /home/sss/ -max 50\n", argv[0]);
      printf("The path must ended with \'/\', for example: \"/home/sss/\" is OK, \"/home/sss\" will cause an error\n");     
      return 0;
  }
  if(argc >= 2)
  {
      int i;
      for (i=1; i<argc; i+=2)
      {
          if (0 == strcmp(argv[i], "-port"))
          {
              strncpy(diag_name, argv[i+1], MAX_DIAG_PORT_NAME);
          }
          else if (0 == strcmp(argv[i], "-path"))
          {
            strncpy(store_path, argv[i+1], 256);
          }
          else if (0 == strcmp(argv[i], "-max"))
          {
            g_max_file_size = atoi(argv[i+1])*1024*1024;
            g_one_file_only = 1;
          }
          else
          {
              printf("Wrong format!\n%s -port [diag port name]\n", argv[0]);
              printf("-port is optional\n");
              return 0;              
          }
      }
  }

  if (strlen(diag_name) != 0)
  {
      printf("Waiting for diag port %s ready......\n", diag_name);
      while (!GetDiagPort(diag_name))
      {
          usleep(1000);
      }
      usleep(1000);
      //printf("diag port name : %s\n",diag_name);
      //CDiagPort::GetInstance().Open(diag_name);
  }
  else if(GetDiagPort(diag_name))
  {
     printf("diag port name : %s\n",diag_name);
     CDiagPort::GetInstance().Open(diag_name);
  }
  else
  {
    printf("no diag port!\n");
    return 0;
  }

  
  send_diag_set_cmd_new("config", 0);

  pthread_t log_thread;  
    
  printf("start to capture log\n");
  
  memset(raw_buffer, 0, sizeof(raw_data_buf)*RAW_DATA_BUF_COUNT);
  memset(buf_for_file, 0, sizeof(raw_data_buf*)*RAW_DATA_BUF_COUNT);
  for (i=0; i<RAW_DATA_BUF_COUNT; ++i)
  {
    buf_for_port[i] = &(raw_buffer[i]);
  }
  
  res = pthread_create(&log_thread,NULL,start_read_port,NULL);
  if (res != 0)
  {
      printf("Create read port thread err!\n");
      return 0;
  }

  res = pthread_create(&log_thread,NULL,start_write_log_file,NULL);
  if (res != 0)
  {
      printf("Create write file thread err!\n");
      return 0;
  }
  read_port_thread_exit = 0;
  write_file_thread_exit = 0;
  
  printf("input 'q' to quit application.\n");

  while(true)
  {
    char choice;
    scanf("%c",&choice);
    if(choice == 'q' || choice =='Q')
    {
      g_stop_log = true;
      break;
    }
    else
    {
        print_log = true;
    }
  }

  while ((0 == read_port_thread_exit) || (0 == write_file_thread_exit))
  {
    usleep(10000);
  }
  
  send_diag_stop_cmd();   
  printf("Total read: %d, Write to file: %d\n", g_total_read_from_port, g_total_write_to_file);

  return 0;
}

void get_new_file_name(char *str_file_name, const char *tail)
{
 time_t now;
 struct tm timenow;
 time(&now);
 timenow = *localtime(&now);
 
 sprintf(str_file_name,"%s%04d-%02d-%02d-%02d-%02d-%02d%s",store_path, timenow.tm_year+1900,timenow.tm_mon+1,timenow.tm_mday,timenow.tm_hour,timenow.tm_min,timenow.tm_sec, tail);
 
 //printf("file name:%s\n",str_file_name);
}

void* start_read_port(void *arg)
{
  int i = 0;
  raw_data_buf *p_cur_buf = NULL;

  while (!g_stop_log)
  {
    if (NULL == buf_for_port[i])
    {
      printf("Warning! All buffer full\n");
      sleep(1);
      continue;
    }

    buf_for_port[i]->used = CDiagPort::GetInstance().Read(buf_for_port[i]->buf, RAW_DATA_BUF_SIZE);
    if (buf_for_port[i]->used <= 0)
    {
      usleep(1000);
      continue;
    }

    g_total_read_from_port += buf_for_port[i]->used;

    if (buf_for_file[i] != NULL)
    {
      printf("ERROR! Buffer not NULL\n");
      exit(0);
    }
    else
    {
      p_cur_buf = buf_for_port[i];
      buf_for_port[i] = NULL;
      buf_for_file[i] = p_cur_buf;
      p_cur_buf = NULL;
      ++i;
      if (i >= RAW_DATA_BUF_COUNT)
      {
        i = 0;
      }
    }
  }

  read_port_thread_exit = 1;
  return 0;
}

void* start_write_log_file(void *arg)
{
  int i=0;
  raw_data_buf *p_cur_buf = NULL;
  FILE *fp_log = NULL;
  FILE *fp_head = NULL;
  int len_left = 0;
  int pos = 0;
  char file_name_buff[256] = {0};

  get_new_file_name(file_name_buff, ".log");
  fp_log = fopen(file_name_buff, "wb");

  if (NULL == fp_log)
  {
    printf("Error!Can not open file for written\n");
    exit(0);
  }

  if (g_one_file_only)
  {
    strcat(file_name_buff, ".head");
    fp_head = fopen(file_name_buff, "wb");
    
    if (NULL == fp_head)
    {
      printf("Error!Can not open file for written\n");
      exit(0);
    }

    len_left = 1;
    fwrite(&len_left, 1, sizeof(int), fp_head);
    pos = ftell(fp_head);
    len_left = 0;
    fwrite(&len_left, 1, sizeof(int), fp_head);
    fseek(fp_head, pos, SEEK_SET);
  }
    
  while(1)
  {
    if (NULL == buf_for_file[i])
    {
      if (!g_stop_log)
      {
        usleep(1000);
        continue;
      }
      else
      {
        break;
      }
    }

    if (g_one_file_only)
    {
      if (g_total_write_to_file + buf_for_file[i]->used > g_max_file_size)
      {
        if (g_max_file_size !=  g_total_write_to_file)
        {
          fwrite(buf_for_file[i]->buf, 1, g_max_file_size - g_total_write_to_file, fp_log);
        }
        len_left = buf_for_file[i]->used + g_total_write_to_file - g_max_file_size;
        fseek(fp_log, 0, SEEK_SET);
        fwrite(buf_for_file[i]->buf + g_max_file_size - g_total_write_to_file, 1, len_left, fp_log);
        g_total_write_to_file = len_left;
      }
      else
      {
        fwrite(buf_for_file[i]->buf, 1, buf_for_file[i]->used, fp_log);
        g_total_write_to_file += buf_for_file[i]->used;
      }
      
      fwrite(&g_total_write_to_file, 1, sizeof(int), fp_head);
      fseek(fp_head, pos, SEEK_SET);
    }
    else
    {
      if (g_total_write_to_file + buf_for_file[i]->used > g_max_file_size)
      {
        fclose(fp_log);
        get_new_file_name(file_name_buff, ".log");
        fp_log = fopen(file_name_buff, "wb");

        if (NULL == fp_log)
        {
          printf("Error!Can not open file for written\n");
          exit(0);
        }

        g_total_write_to_file = 0;
      }

        fwrite(buf_for_file[i]->buf, 1, buf_for_file[i]->used, fp_log);
        g_total_write_to_file += buf_for_file[i]->used;
    }
    

    if (buf_for_port[i] != NULL)
    {
      printf("ERROR! Buffer not NULL\n");
      exit(0);
    }
    else
    {
      p_cur_buf = buf_for_file[i];
      buf_for_file[i] = NULL;
      buf_for_port[i] = p_cur_buf;
      p_cur_buf = NULL;
      ++i;
      if (i >= RAW_DATA_BUF_COUNT)
      {
        i = 0;
      }
    }

    if (print_log)
    {
      print_log = false;
      printf("Total read: %d, Write to file: %d\n", g_total_read_from_port, g_total_write_to_file);
    }
  }

  if (NULL != fp_log) fclose(fp_log);
  if (NULL != fp_head) fclose(fp_head);
  write_file_thread_exit = 1;
  //pthread_exit("end"); 
  return 0;
}

void send_diag_set_cmd_new(char *dmcFilePath, int howToConfig)
{
  printf("sending diag setting command......\n");

  tx_pkt_buf_type *ptxBuf = new tx_pkt_buf_type;
  rx_pkt_buf_type *prxBuf = new rx_pkt_buf_type;
  DiagCommandGenerator diagCmdGenerator;
  diagCmdGenerator.GetDmcFileText(dmcFilePath, howToConfig);
  while (1)
  {
    diagCmdGenerator.GetNextCommand(ptxBuf->buf, &(ptxBuf->length));
    if (0 == ptxBuf->length) break;
    if (!CDiagCmd::GetInstance().SendRawCmd(ptxBuf, prxBuf)) break;
    diagCmdGenerator.SetResponseOfLastCmd(prxBuf->buf, prxBuf->length);
    diagCmdGenerator.SetLastCmdInDiagFormat(ptxBuf->buf, ptxBuf->length);
  }
  
  delete ptxBuf;
  delete prxBuf;
}


void send_diag_stop_cmd()
{
  printf("sending diag stop command......\n");
  tx_pkt_buf_type *ptxBuf = new tx_pkt_buf_type;
  rx_pkt_buf_type *prxBuf = new rx_pkt_buf_type;
  char *cmdValue[] = {"60 00", "73 00 00 00 00 00 00 00", "7d 05 00 00 00 00 00 00"};
  for(int i=0;i<sizeof(cmdValue)/sizeof(char *);i++)
  {
    CDiagCmd::GetInstance().SendHexStrCmd(cmdValue[i],ptxBuf,prxBuf);     
  }
  
  delete ptxBuf;
  delete prxBuf;
}

bool GetDiagPort(char *pstrDiag)
{
  if(strlen(pstrDiag) == 0) printf("detecting diag port......\n");
  bool bret = false;
  char usb_port_name[MAX_DIAG_PORT_NAME]={0};
  tx_pkt_buf_type *ptxBuf = new tx_pkt_buf_type;
  rx_pkt_buf_type *prxBuf = new rx_pkt_buf_type;
  
   
  if(strlen(pstrDiag)!=0)//allowed user to pass port name to check if it is a diag port
  {
    strcpy(usb_port_name,pstrDiag);
    if(CDiagPort::GetInstance().Open(usb_port_name))
    {
      bret = CDiagCmd::GetInstance().SendHexStrCmd("00",ptxBuf,prxBuf); 
    }
  }
  else//if no port name is transmitted ,auto detecting ttyUSB
  {
    int old_retry_time =  CDiagCmd::GetInstance().SetRetryTime(1);//reduce send retry times to save get diag port time
    for(int i=0;i<10;i++)//try  ttyUSB
    {
      sprintf(usb_port_name, "/dev/ttyUSB%d",i);
      if(CDiagPort::GetInstance().Open(usb_port_name))
      {
	bret = CDiagCmd::GetInstance().SendHexStrCmd("00",ptxBuf,prxBuf); 
	CDiagPort::GetInstance().Close();
	if(bret)break;
      }
      else
      {
	continue;
      }
    }  
    CDiagCmd::GetInstance().SetRetryTime(old_retry_time);
  }


  if(bret)
  {
    strcpy(pstrDiag,usb_port_name);
  }
  delete ptxBuf;
  delete prxBuf;
  return bret;

}


