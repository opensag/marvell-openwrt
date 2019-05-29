/* ============================================================================
 * @file: ecm_demo_atctl.h
 * @author: Wei,LI
 * @Copyright:                      GoSUNCN
* @Website:                         www.ztewelink.com
* @Email:                           ztewelink@zte.com.cn
* @version:                         "ECM_CALLV1.0.1B04"
* @date:                            "2019-03-11"
* ============================================================================*/

#include <pthread.h>
#include <termios.h>
#include <stdarg.h>
#include "ecm_demo_atctl.h"

#ifndef __ECM_DEMO_TTYDEV__H
#define __ECM_DEMO_TTYDEV__H

typedef enum
{
    TTY_RECV_INIT               = 0x00,
    TTY_RECV_UP                 = 0x01,
    TTY_RECV_KILL               = 0x02,
    TTY_RECV_FAIL               = 0x03,

} TTY_RECV_TXD_STAT_T;

typedef enum
{
    TTY_O_DEFAULT                = 0x00,
    TTY_O_USR_PATH               = 0x01,
    TTY_O_USR_SPC_FD             = 0x02,

} TTY_OPEN_USR_T;


typedef struct  
{
    int                           tty_fd;
    int                           tty_oflag;
    char                          tty_path[TTYUSB_PATH_LEN];

    TTY_OPEN_USR_T                tty_type;

#ifdef POSIX_TERMIOS
    struct termios                tty_tio_bak;
#endif
    struct termios                tty_tio_cfg;

    ecm_recv_cb_t                 tty_recv_cb;

    pthread_t                     tty_ptxd_listen;

    TTY_RECV_TXD_STAT_T           tty_ptxd_state;

    int                           tty_ptxd_tid ;

} ttyusb_dev_t;


void            ttyusb_init   (ttyusb_dev_t* p_serial);


unsigned int ttyusb_config(ttyusb_dev_t* p_serial, char* path, unsigned int path_len);


unsigned int ttyusb_open(ttyusb_dev_t* p_serial);


unsigned int ttyusb_send(ttyusb_dev_t* p_serial, const char* at_cmd, unsigned int at_cmd_len);


unsigned int ttyusb_reg_cb(ttyusb_dev_t* p_serial, ecm_recv_cb_t cb);


unsigned int    ttyusb_start_recv         (ttyusb_dev_t* p_serial);


void            ttyusb_cancel_thxd(ttyusb_dev_t* p_serial);


void            ttyusb_close(ttyusb_dev_t* p_serial);


unsigned int ttyusb_search_port(unsigned int which_port, char* ttyusb_path, unsigned int ttyusb_path_len);


/*add liwei for port error start*/
unsigned   int  ttyusb_get_thxd_failed_flg(void);


unsigned   int  ttyusb_get_clear_failed_flg(void);
/*add liwei for port error end*/


#endif


