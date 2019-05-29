
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>

#include "ecm_demo_config.h"
#include "ecm_demo_atctl.h"
#include "ecm_demo_msg.h"
#include "ecm_demo_ttydev.h"


#define   TTYUSB_DRV_PATH_MAX 256//FILENAME_MAX

const char *TTYUSB_DRV_PATH = "/sys/bus/usb/devices/";

/*
struct dirent
{
    long d_ino; 
    off_t d_off; 
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name [NAME_MAX+1];
};
struct __dirstream
{
    void *__fd;
    char *__data;
    int __entry_data;
    char *__ptr;
    int __entry_ptr; 
    size_t __allocation; 
    size_t __size;
    __libc_lock_define (, __lock)
};
typedef struct __dirstream DIR;
*/

typedef struct
{
    int          magic;
    char        *idVendor;
    char        *idProduct;    
    char         deviceport[TTYUSB_DRV_PATH_MAX];
    char         dataport[TTYUSB_DRV_PATH_MAX];
    char         rndisport[TTYUSB_DRV_PATH_MAX];
    const char *bIfaceNumb_AT;
    const char *bIfaceNumb_DATA;
    const char *bIfaceNumb_ECM;
    const char *name;
} ttyusb_drv_xport_t ;


// const ? 
const ttyusb_drv_xport_t ttyusb_drv_map_tab[] =
{
	{
		.name		= "ZTE-MF206",
		.idVendor	= "19D2",
		.idProduct	= "0117",
		.bIfaceNumb_AT = "1",
		.bIfaceNumb_DATA = "2",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= NULL,
		.magic = 0 ,
	},

	{
		.name		= "ZTE-AC100",
		.idVendor	= "19D2",
		.idProduct	= "0152",
		.bIfaceNumb_AT = "2",
		.bIfaceNumb_DATA = "0",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= NULL,
        .magic = 0 ,
	},

	{
		.name		= "ZTE-MODEM",
		.idVendor	= "19D2",
		.idProduct	= "0016",
		.bIfaceNumb_AT = "1",
		.bIfaceNumb_DATA = "2",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= NULL,
        .magic = 0 ,
	},

	{
		.name		= "ZTE-MODEM",
		.idVendor	= "19D2",
		.idProduct	= "0017",
		.bIfaceNumb_AT = "1",
		.bIfaceNumb_DATA = "2",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= NULL,
        .magic = 0 ,
	},

	{
		.name		= "ZTE-MF226",
		.idVendor	= "19D2",
		.idProduct	= "0144",
		.bIfaceNumb_AT = "2",
		.bIfaceNumb_DATA = "4",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= NULL,
        .magic = 0 ,
	},

	{
		.name		= "ZTE-MODEM",
		.idVendor	= "19D2",
		.idProduct	= "2003",
		.bIfaceNumb_AT = "1",
		.bIfaceNumb_DATA = "3",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= NULL,
        .magic = 0 ,
	},

	{
		.name		= "ZTE-MODEM",
		.idVendor	= "19D2",
		.idProduct	= "1300",
		.bIfaceNumb_AT = "2",
		.bIfaceNumb_DATA = "0",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= NULL,
        .magic = 0 ,
	},
	{
		.name		= "ZTE-AC200",
		.idVendor	= "19D2",
		.idProduct	= "0094",
		.bIfaceNumb_AT = "2",
		.bIfaceNumb_DATA = "0",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= NULL,
        .magic = 0 ,
	},

	{
		.name		= "ZTE-MF210",
		.idVendor	= "19D2",
		.idProduct	= "1409",
		.bIfaceNumb_AT = "1",
		.bIfaceNumb_DATA = "2",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= NULL,
        .magic = 0 ,
	},

	{
		.name		= "ZTE-ZM8510",
		.idVendor	= "19D2",
		.idProduct	= "0396",
		.bIfaceNumb_AT = "1",
		.bIfaceNumb_DATA = "2",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= "3",
        .magic = 0 ,
	},

	{
		.name		= "ZTE-ME3620",
		.idVendor	= "19D2",
		.idProduct	= "1432",
		.bIfaceNumb_AT = "1",
		.bIfaceNumb_DATA = "2",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= "3",
        .magic = 0 ,
    },

	{
		.name		= "ZTE-ME3620",
		.idVendor	= "19D2",
		.idProduct	= "1433",
		.bIfaceNumb_AT = "1",
		.bIfaceNumb_DATA = "2",
		.deviceport	= "",
		.dataport	= "",
		.rndisport = "",
		.bIfaceNumb_ECM= "3",
		.magic = 0 ,
	},

    {
        .name   = "ZTE-ME3620",
        .idVendor = "19D2",
        .idProduct  = "1476",
        .bIfaceNumb_AT = "1",
        .bIfaceNumb_DATA = "2",
        .deviceport = "",
        .dataport = "",
        .rndisport = "",
        .bIfaceNumb_ECM= "3",
        .magic = 0 ,
    },
    {
        .name   = "ZTE-ME3630",
        .idVendor = "305A",
        .idProduct  = "1406",
        .bIfaceNumb_AT = "1",
        .bIfaceNumb_DATA = "2",
        .deviceport = "",
        .dataport = "",
        .rndisport = "",
        .bIfaceNumb_ECM= "3",
        .magic = 0 ,
    },

};
/*add liwei for port error start*/
unsigned int g_ttyusb_recv_thxd_fail_flag = 0 ;
/*add liwei for port error end*/

/*add liwei for port error start*/
unsigned   int  ttyusb_get_thxd_failed_flg(void)
{
    return g_ttyusb_recv_thxd_fail_flag ;
}
unsigned   int  ttyusb_get_clear_failed_flg(void)
{
    g_ttyusb_recv_thxd_fail_flag = 0 ;
}

/*add liwei for port error end*/

void ttyusb_cancel_thxd(ttyusb_dev_t* p_serial)
{
    if (p_serial)
    {
        if (TTY_RECV_UP==p_serial->tty_ptxd_state)
        {
            /*add liwei for start resolve fd issue start*/
            ttyusb_close(p_serial);
            pthread_cancel(p_serial->tty_ptxd_listen);
            pthread_join(p_serial->tty_ptxd_listen,NULL);
            p_serial->tty_ptxd_state = TTY_RECV_KILL;
            ECM_log(ECM_LOG_L_2,"[info] ttyusb_recv_thxd exit");
            /*add liwei for start resolve fd issue end*/
        }
    }
}

void* ttyusb_recv_thxd(void* para)
{
    unsigned int     error = 0;
    ttyusb_dev_t*    p_serial;
    int              txd_id = 0;
    int              read_len = 0;    
    char             txd_buff[TTYUSB_BUF_LEN];
    int              cancel_var;

    do {
        p_serial=(ttyusb_dev_t*)para;

        if ((!para) || (p_serial->tty_fd<0))
        {
            error= E_TTYUSB_RECV_THXD_PARA_NULL;
            break;
        }

        txd_id = pthread_self();

        p_serial->tty_ptxd_tid = txd_id ;

        ECM_log(ECM_LOG_L_4,"[info] ttyusb_recv_thxd up, tid(%08X)",txd_id);

        p_serial->tty_ptxd_state = TTY_RECV_UP;

        /*add cancel point*/
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&cancel_var);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
        
        while(TTY_RECV_KILL!= p_serial->tty_ptxd_state)
        {

            /*add liwei for start resolve fd issue start*/
            if(-1 != p_serial->tty_fd)
            {
                /*add cancel point*/
                pthread_testcancel();
                
                bzero((void*)txd_buff, TTYUSB_BUF_LEN);

                read_len = read(p_serial->tty_fd, txd_buff, TTYUSB_BUF_LEN-1);/*cancel point*/

                txd_buff[TTYUSB_BUF_LEN-1] = 0;
                if ((read_len>0) && (read_len < TTYUSB_BUF_LEN))
                {
                    txd_buff[read_len] = 0;
                    if (p_serial->tty_recv_cb)
                    {
                        (*(p_serial->tty_recv_cb))(txd_buff,read_len);
                    }
                }
                else
                {
                    /* for user defined nonblock*/
                    usleep(ECM_AT_RECV_INTV);
                
                    /*add liwei for port error start*/
                    ++ g_ttyusb_recv_thxd_fail_flag  ;
                    /*add liwei for port error end*/
                
                    if (TTY_RECV_KILL!= p_serial->tty_ptxd_state)
                    {
						ECM_log(ECM_LOG_L_3,"[info] ttyusb_recv_thxd interv(work:%d)", p_serial->tty_ptxd_state);
                        
                    }
                    else
                    {
                        ECM_log(ECM_LOG_L_3,"[info] ttyusb_recv_thxd interv(suspend)");
                    }
                
                }

            }
            else
            {
                /* for user defined nonblock*/
                usleep(ECM_AT_RECV_INTV);
                
                /*add liwei for port error start*/
                ++ g_ttyusb_recv_thxd_fail_flag  ;
                /*add liwei for port error end*/
                
                if (TTY_RECV_KILL!= p_serial->tty_ptxd_state)
                {                    
					ECM_log(ECM_LOG_L_3,"[info] ttyusb_recv_thxd interv(suspend wait be killed:%d)", p_serial->tty_ptxd_state);
                }
                else
                {
					ECM_log(ECM_LOG_L_3,"[info] ttyusb_recv_thxd interv(suspend but killed)");
                }

            }
            /*add liwei for start resolve fd issue end*/

            /*add cancel point*/
            pthread_testcancel();

        }

        ECM_log(ECM_LOG_L_3,"[info] ttyusb_recv_thxd down inner");

    } while(0);

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_3,"ttyusb_recv_thxd exit, error(%d)",error);
    }
    //exit(0);
    return NULL ;
}

unsigned int    ttyusb_start_recv(ttyusb_dev_t* p_serial)
{
    unsigned int error = 0 ;
    unsigned int cycle = 0 ;
    int thxd_res = 0;

    do {
        if (!p_serial)
        {
            error = E_TTYUSB_START_RECV_TTY_S_NULL;
            break;
        }

        /*Start Listen the serial port*/
        p_serial->tty_ptxd_state = TTY_RECV_INIT;
        
        thxd_res=pthread_create(&(p_serial->tty_ptxd_listen),
                        NULL, ttyusb_recv_thxd,(void*)p_serial);

        if (0!=thxd_res)
        {
            p_serial->tty_ptxd_state = TTY_RECV_FAIL;
            error = E_TTYUSB_START_RECV_NEW_TXD_FAIL;
            break;
        }

        while ((TTY_RECV_UP!=p_serial->tty_ptxd_state)&&(cycle<10))
        {
            sleep(1);
            ++ cycle;
        }

        if (TTY_RECV_UP!=p_serial->tty_ptxd_state)
        {
            p_serial->tty_ptxd_state = TTY_RECV_KILL;
            error = E_TTYUSB_START_RECV_TXD_BOOT_FAIL;
            break;
        }
    }while(0);

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_3,"ttyusb_start_recv:error(%d)",error);
    }

    return error ;
}

void ttyusb_init(ttyusb_dev_t* p_serial)
{
    if (p_serial)
    {
        /*add liwei for start resolve fd issue start*/
        if(-1 != p_serial->tty_fd)
        {
            ECM_log(ECM_LOG_L_3,"[info] unresolved ttyusb_init fd=%lu",p_serial->tty_fd);
        }
        /*add liwei for start resolve fd issue end*/
        bzero((void*)p_serial,sizeof(ttyusb_dev_t));
        p_serial->tty_fd = -1 ;
    }
}

unsigned int ttyusb_config(ttyusb_dev_t* p_serial, char* path, unsigned int path_len)
{
    unsigned int error = E_TTYUSB_CONFIG_OK;

    do {
        if (!p_serial)
        {
            error = E_TTYUSB_CONFIG_TTY_S_NULL;
            break;
        }

        if (!path)
        {   /*usr un-define the device node*/
            p_serial->tty_type = TTY_O_DEFAULT;
            strcpy(p_serial->tty_path,TTYUSB_DEV_PATH);
        }
        else if (path_len != strlen(path))
        {   /*usr define   error device node*/
            error = E_TTYUSB_CONFIG_PATH_LENG;
            break;
        }
        else if (TTYUSB_PATH_LEN<=path_len)
        {   /*user define device path is upto max permit*/
            error = E_TTYUSB_CONFIG_OVER_MAX;
            break;
        }
        else
        {   /*usr define   correct device node*/
            p_serial->tty_type = TTY_O_USR_PATH;
            strcpy(p_serial->tty_path,path);
        }

        /*config serial port parameter*/
        bzero((void*)(&(p_serial->tty_tio_cfg)),sizeof(struct termios));

        /*config to raw mode*/
        cfmakeraw(&(p_serial->tty_tio_cfg));

        cfsetispeed(&(p_serial->tty_tio_cfg),TTYUSB_BAUDRATE);
        cfsetospeed(&(p_serial->tty_tio_cfg),TTYUSB_BAUDRATE);

        p_serial->tty_tio_cfg.c_cflag |= CLOCAL |CREAD ;
        p_serial->tty_tio_cfg.c_cflag &= ~CSIZE;
        p_serial->tty_tio_cfg.c_cflag |= CS8;
        
        p_serial->tty_tio_cfg.c_cflag &= ~PARENB;/*No Parity func*/
        p_serial->tty_tio_cfg.c_cflag &= ~INPCK; /*Disable parity*/
        p_serial->tty_tio_cfg.c_cflag &= ~CSTOPB;/*1 bit stopbits*/
        p_serial->tty_tio_cfg.c_cc[VTIME] = 0;
        p_serial->tty_tio_cfg.c_cc[VMIN] = 1;

    } while(0);

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_3,"ttyusb_config, error(%d)",error);
    }

    return error ;
}

unsigned int ttyusb_open(ttyusb_dev_t* p_serial)
{
    int tmp_var = 0;
    unsigned int error = 0;   

    do {
        if (!p_serial)
        {
            error = E_TTYUSB_OPEN_TTY_S_NULL ;
            break;
        }

        /*open serial port*/
        p_serial->tty_oflag=O_RDWR ;//|O_NOCTTY|O_NONBLOCK|O_NDELAY 
        p_serial->tty_fd = open(p_serial->tty_path, p_serial->tty_oflag);

        if(p_serial->tty_fd<0)
        {  
            error = E_TTYUSB_OPEN_TTY_DEV_FAIL;
            break;
        }

        /*add liwei for start resolve fd issue start*/
        if(-1 != p_serial->tty_fd)
        {
            ECM_log(ECM_LOG_L_2,"[info] ttyusb_open fd=%lu",p_serial->tty_fd);
        }
        /*add liwei for start resolve fd issue end*/


#ifdef POSIX_TERMIOS
        /*backup serial port parameter*/
        if ((TTY_O_DEFAULT==p_serial->tty_type) || (TTY_O_USR_PATH==p_serial->tty_type))
        {
            if (0!=tcgetattr(p_serial->tty_fd,&(p_serial->tty_tio_bak)))
            {
                error = E_TTYUSB_OPEN_TTY_BAK_CFG;                
                /*add liwei for start resolve fd issue start*/
                if(0 != close(p_serial->tty_fd))
                {
                    ECM_log(ECM_LOG_L_1,"[info] ttyusb_open close fd fail pos0");
                }
                else
                {
                    p_serial->tty_fd = -1 ;
                    ECM_log(ECM_LOG_L_3,"[info] ttyusb_open close fd ok pos0");
                }
                /*add liwei for start resolve fd issue end*/
                break;
            }
        }
#endif
        tcflush(p_serial->tty_fd,TCIFLUSH);
        tcsetattr(p_serial->tty_fd,TCSANOW,&p_serial->tty_tio_cfg);

        tmp_var = fcntl(p_serial->tty_fd, F_GETFL, 0);
        //tmp_var = fcntl(p_serial->tty_fd, F_SETFL, tmp_var & ~O_NDELAY);
        if (tmp_var < 0)
        {
            error = E_TTYUSB_OPEN_TTY_FCNTL_ERR;
            /*add liwei for start resolve fd issue start*/
            if(0 != close(p_serial->tty_fd))
            {
                ECM_log(ECM_LOG_L_1,"[info] ttyusb_open close fd fail pos1");
            }
            else
            {
                p_serial->tty_fd = -1 ;
                ECM_log(ECM_LOG_L_3,"[info] ttyusb_open close fd ok pos1");
            }
            /*add liwei for start resolve fd issue end*/


            break;
        }

    } while(0);

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_3,"ttyusb_open:error(%d)",error);
    }

    return error;

}

void ttyusb_close(ttyusb_dev_t* p_serial)
{
    unsigned int error = 0 ;

    do {
        if (!p_serial)
        {
            error = E_TTYUSB_CLOSE_TTY_S_NULL;
            break;
        }
        if(p_serial->tty_fd<0)/*no need to close*/
        {  
            error = E_TTYUSB_CLOSE_TTY_FD_ERR;
            break;
        }
        
#ifdef POSIX_TERMIOS
        /*restore serial port parameter*/
        if ((TTY_O_DEFAULT==p_serial->tty_type) || (TTY_O_USR_PATH==p_serial->tty_type))
        {
            tcsetattr(p_serial->tty_fd,TCSANOW,&(p_serial->tty_tio_bak));
        }
#endif
        /*close file description*/
        if ((TTY_O_DEFAULT==p_serial->tty_type) || (TTY_O_USR_PATH==p_serial->tty_type))
        {
            /*add liwei for start resolve fd issue start*/
            if(0 != close(p_serial->tty_fd))
            {
                ECM_log(ECM_LOG_L_1,"[info] ttyusb_close close fd fail");
            }
            else
            {
                p_serial->tty_fd = -1;
                ECM_log(ECM_LOG_L_3,"[info] ttyusb_close close fd ok");
            }
            /*add liwei for start resolve fd issue end*/

        }
        else if (TTY_O_USR_SPC_FD==p_serial->tty_type)
        {
            /*user close the spc fd himself */
        }
    } while(0);

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_3,"ttyusb_close:error(%d)",error);
    }

}

unsigned int ttyusb_send(ttyusb_dev_t* p_serial,
    const char* at_cmd, unsigned int at_cmd_len)
{
    unsigned int error = 0;
    unsigned int data_send_len = 0;
    unsigned int data_rest_len = at_cmd_len;
    int res = 0;

    do {
        if ((!p_serial) || (!at_cmd) || (0==at_cmd_len))
        {
            error = E_TTYUSB_SEND_PARA_ERR;
            break;
        }
        if (strlen(at_cmd) != at_cmd_len)
        {
            error = E_TTYUSB_SEND_LENTH_ERR;
            break;
        }
        if (p_serial->tty_fd<0)/*not open*/
        {
            error = E_TTYUSB_SEND_FD_ERR;
            break;
        }
        do
        {
            res = write(p_serial->tty_fd,(char*)(at_cmd+data_send_len), data_rest_len);
            if ((res<=0) || (res>data_rest_len))
            {
                error = E_TTYUSB_SEND_ABNORMAL;
                break ;
            }
            data_send_len += res;
            data_rest_len -= res;
        } while(data_rest_len>0);

    } while(0);

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_3,"[info] ttyusb_send:error(%d)",error);
    }

    return error;

}


unsigned int ttyusb_reg_cb(ttyusb_dev_t* p_serial, ecm_recv_cb_t cb)
{
    unsigned int error = 0 ;

    do {

        if ((!p_serial) || (!cb))
        {
            error = E_TTYUSB_REG_CB_PARA_NULL;
            break;
        }

        p_serial->tty_recv_cb = cb ;

    } while(0);

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_3,"ttyusb_reg_cb:error(%d)",error);
    }

    return error;
}



int  ttyusb_drv_read_file(char *path, char *content, size_t size)
{
    int ret;
    FILE *f;

    f = fopen(path, "r");

    if (f == NULL)
    {
        ECM_log(ECM_LOG_L_4,"[info] open(%s)failed", path);
        return -1;
    }

    ret = fread(content, 1, size, f);

    fclose(f);

    return ret;
}


unsigned int    ttyusb_drv_is_equal(       const ttyusb_drv_xport_t *device, const char *idv, const char *idp)
{
    unsigned int is_equal = 0;
    long pvid = 0xffff;
    long ppid = 0xffff;    
    long t_vid;
    long t_pid;

    do {
        if ((NULL==device) || (NULL==idv) || (NULL==idp))
        {
            break;
        }

        t_vid = strtol(device->idVendor, NULL, 16);
        t_pid = strtol(device->idProduct, NULL, 16);
        pvid =  strtol(idv, NULL, 16);
        ppid =  strtol(idp, NULL, 16);

        if ((t_vid == pvid) && (t_pid == ppid))
        {
            is_equal = 1;
        }
        else
        {
            is_equal = 0;
        }
    } while(0);

    return is_equal;
}


int ttyusb_drv_startwith(const char *line, const char *prefix)
{
    for ( ; *line != '\0' && *prefix != '\0' ; line++, prefix++) {
        if (*line != *prefix) {
            return 0;
        }
    }

    return *prefix == '\0';
}


int ttyUSB_drv_get_path(const ttyusb_drv_xport_t * device, char * path, char *ttyusbx)
{
    DIR   *ttyusb_dir;
    struct dirent *dent;

    ttyusb_dir = opendir(path);

    if(NULL == ttyusb_dir)
    {
        ECM_log(ECM_LOG_L_4, "[info] open path(%s)fail", path);
        return -1;
    }
	
    while ((dent = readdir(ttyusb_dir)) != NULL) 
    {
        if (strcmp(dent->d_name, ".") == 0
            || strcmp(dent->d_name, "..") == 0 
            || (!ttyusb_drv_startwith(dent->d_name, "ttyUSB")))
        {
            continue;
        }
        else
        {
            ECM_log(ECM_LOG_L_4, "[info] ttyUSB_drv_get_path=/dev/%s ", dent->d_name);
            strcpy(ttyusbx, dent->d_name);
            closedir(ttyusb_dir);
            return 0;
        }
    }

    closedir(ttyusb_dir);
    return -1;
}


unsigned int ttyusb_search_port(unsigned int which_port, char* ttyusb_path, unsigned int ttyusb_path_len)
{
    struct dirent        *dent    = NULL;
    DIR                  *usbdir  = NULL;
    const ttyusb_drv_xport_t *device = NULL;
    unsigned int          error   = 0;

    int                   ret = 0;
    char                  idvendor[64] = {0};
    char                  idproduct[64] = {0};
    char                  bConfigurationValue[64] = {0};
    char                  bNumInterfaces[64] = {0};

    char                 *str_temp_path = NULL;
    char                 *str_ttyusb_x = NULL;

    int                   i   = 0;
    int                   size = 0;

    do {
        ECM_log(ECM_LOG_L_4,"[info] ttyusb_search_port(%01d) TTYUSB_DRV_PATH_MAX=(%02d)",
            which_port, TTYUSB_DRV_PATH_MAX);
    
        if (NULL == ttyusb_path)
        {
            error = 1 ;
            break;
        }

        if (TTYUSB_DRV_PATH_MAX > ttyusb_path_len)
        {
            error = 2 ;
            break;
        }

        str_temp_path = malloc(TTYUSB_DRV_PATH_MAX);        

        if (NULL == str_temp_path)
        {
            error = 3 ;
            break;
        }

        str_ttyusb_x = malloc(TTYUSB_DRV_PATH_MAX);

        if (NULL == str_ttyusb_x)
        {
            error = 4 ;
            break;
        }

        usbdir = opendir(TTYUSB_DRV_PATH);

        if (NULL == usbdir) 
        {
            error = 5 ;
            break;
        }


        while (NULL != (dent = readdir(usbdir)))
        {
            if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            {
                continue;
            }
           
            bzero(idvendor,            sizeof(idvendor));
            bzero(idproduct,           sizeof(idproduct));
            bzero(bConfigurationValue, sizeof(bConfigurationValue));
            bzero(bNumInterfaces,      sizeof(bNumInterfaces));

            bzero(str_temp_path, TTYUSB_DRV_PATH_MAX) ;
            sprintf(str_temp_path,"%s%s%s",TTYUSB_DRV_PATH,dent->d_name,"/idVendor");
            ECM_log(ECM_LOG_L_4,"[info] vid read(%s)",str_temp_path);
            ret = ttyusb_drv_read_file(str_temp_path, idvendor, 4);
            if ((ret <= 0) && (4!=ret))
            {
                continue;
            }

            bzero(str_temp_path, TTYUSB_DRV_PATH_MAX) ;
            sprintf(str_temp_path,"%s%s%s",TTYUSB_DRV_PATH,dent->d_name,"/idProduct");
            ECM_log(ECM_LOG_L_4,"[info] pid read(%s)",str_temp_path);
            ret = ttyusb_drv_read_file(str_temp_path, idproduct, 4);
            if ((ret <= 0) && (4!=ret))
            {
                continue;
            }

            bzero(str_temp_path, TTYUSB_DRV_PATH_MAX) ;
            sprintf(str_temp_path,"%s%s%s",TTYUSB_DRV_PATH,dent->d_name,"/bConfigurationValue");
            ECM_log(ECM_LOG_L_4,"[info] cfg read(%s)",str_temp_path);
            ret = ttyusb_drv_read_file(str_temp_path, bConfigurationValue, 1);
            if ((ret <= 0) && (1!=ret))
            {
                continue;
            }

            bzero(str_temp_path, TTYUSB_DRV_PATH_MAX) ;
            sprintf(str_temp_path,"%s%s%s",TTYUSB_DRV_PATH,dent->d_name,"/bNumInterfaces");
            ECM_log(ECM_LOG_L_4,"[info] iface read(%s)",str_temp_path);
            ret = ttyusb_drv_read_file(str_temp_path, bNumInterfaces, 1);
            if ((ret <= 0) && (1!=ret))
            {
                continue;
            }

            ECM_log(ECM_LOG_L_4,"[info] vid=%s,pid=%s,cfg=%01d,iface=%01d",
                idvendor,idproduct,(unsigned char)(bConfigurationValue[0]),(unsigned char)(bNumInterfaces[0]));

            for (i = 0, device = NULL, size = sizeof(ttyusb_drv_map_tab)/sizeof(ttyusb_drv_xport_t);
                 i < size;
                 i++)
            {
                if (1==ttyusb_drv_is_equal(&ttyusb_drv_map_tab[i], idvendor, idproduct))
                {
                    device = &ttyusb_drv_map_tab[i];
                    break;
                }
            }

            if (NULL != device)
            {                
                ECM_log(ECM_LOG_L_4,"[info] ttyusb matched name(%s)vid(%s)pid(%s)",
                    device->name, device->idVendor, device->idProduct);

                bzero(str_ttyusb_x,TTYUSB_DRV_PATH_MAX);                
                bzero(str_temp_path, TTYUSB_DRV_PATH_MAX);
                if (0==which_port)
                {
                    sprintf(str_temp_path,"%s%s/%s:%s.%s/",TTYUSB_DRV_PATH,dent->d_name,dent->d_name,bConfigurationValue,device->bIfaceNumb_AT);
                }
                else
                {
                    sprintf(str_temp_path,"%s%s/%s:%s.%s/",TTYUSB_DRV_PATH,dent->d_name,dent->d_name,bConfigurationValue,device->bIfaceNumb_DATA);
                }
                ECM_log(ECM_LOG_L_4,"[info] AT Interface path = %s",str_temp_path);
                ret = ttyUSB_drv_get_path(device, str_temp_path, str_ttyusb_x);
                if( (!ret) && ttyusb_drv_startwith(str_ttyusb_x, "ttyUSB") )
                {
                    bzero(ttyusb_path,TTYUSB_DRV_PATH_MAX);
                    sprintf(ttyusb_path,"%s%s","/dev/",str_ttyusb_x);
                    ECM_log(ECM_LOG_L_3,"[info] get ttyusb_search_port success port(%s)",ttyusb_path);
                    break;
                }
                else
                {
                    device = NULL ;
                    ECM_log(ECM_LOG_L_4,"[info] not correct ttyUSBX, continue");
                    continue;
                }
            }
        }

        closedir(usbdir);

        if (NULL==device)
        {
            error = 6 ;            
        }

    } while(0);


    if (NULL != str_temp_path)
    {
        free(str_temp_path);
        str_temp_path = NULL;
    }
  
    if (NULL != str_ttyusb_x)
    {
        free(str_ttyusb_x);
        str_ttyusb_x = NULL;
    }

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_4,"[info] ttyusb_search_matched_port error %01d",error);
    }

    return error ;
}


