/* ============================================================================
 * @file: ecm_demo_atctl.h
 * @author: Wei,LI
 * @Copyright:                      GoSUNCN
* @Website:                         www.ztewelink.com
* @Email:                           ztewelink@zte.com.cn
* @version:                         "ECM_CALLV1.0.1B04"
* @date:                            "2019-03-11"
* ============================================================================*/


#ifndef __ECM_DEMO_ATCTL__H
#define __ECM_DEMO_ATCTL__H

#include "ecm_demo_config.h"
#include "ecm_demo_msg.h"
#include "ecm_demo_ttydev.h"

typedef enum {
    ECM_SM_ENTRY_EVT,
    ECM_SM_QUERY_EVT,
    ECM_SM_AT_RSP_EVT,
    ECM_SM_TIMEOUT_EVT,
    ECM_SM_KILL_EVT,
    ECM_SM_EXIT_EVT,

} ECM_SM_EVENT_T;

typedef enum {
    ECM_SM_EXEC_KEEP,
    ECM_SM_EXEC_OK,
    ECM_SM_EXEC_FAIL,
    ECM_SM_EXEC_SUCC,
    ECM_SM_EXEC_TERM,

} ECM_SM_RSLT_T;


/*auto connect state machine here*/
typedef enum {
    ECM_AUTO_SM_ENTRY_EVT,
    ECM_AUTO_SM_QUERY_EVT,
    ECM_AUTO_SM_AT_RSP_EVT,
    ECM_AUTO_SM_TIMEOUT_EVT,
    ECM_AUTO_SM_KILL_EVT,
    ECM_AUTO_SM_EXIT_EVT,
    
    ECM_AUTO_SM_REQ_QUIT_EVT,

    ECM_AUTO_SM_REQ_REBOOT_EVT,


} ECM_AUTO_SM_EVENT_T;


typedef enum {
    ECM_AUTO_SM_NO_SWITCH,
    ECM_AUTO_SM_JUMP_CALLUP,
    ECM_AUTO_SM_JUMP_CONNECT,
    ECM_AUTO_SM_JUMP_DISCON,
    ECM_AUTO_SM_JUMP_CALLDOWN,
    ECM_AUTO_SM_JUMP_REBOOT,
    ECM_AUTO_SM_TERMINAL,
} ECM_AUTO_SM_JUMP_T;


typedef enum {
    ECM_AUTO_SM_STATE_UNINIT,
    ECM_AUTO_SM_STATE_CALLUP,
    ECM_AUTO_SM_STATE_DISCON,
    ECM_AUTO_SM_STATE_CONNECT,
    ECM_AUTO_SM_STATE_CALLDOWN,
    ECM_AUTO_SM_STATE_REBOOT,
} ECM_AUTO_SM_STATE_T;
/*auto connect state machine finish*/


typedef enum
{
    ECM_AUTO_CALL_INIT            = 0x00,
    ECM_AUTO_CALL_CFG_ERR                 = 0x01,
    ECM_AUTO_CALL_OPEN_FAIL                 = 0x02,
    ECM_AUTO_CALL_REG_FAIL                 = 0x03,
    ECM_AUTO_CALL_RCV_FAIL =0x04,
    ECM_AUTO_CALL_ONLINE                   = 0x05,
    ECM_AUTO_CALL_OFFLINE                  = 0x06,
    ECM_AUTO_CALL_EXIT                   = 0x07,
    ECM_AUTO_CALL_CLOSE                  = 0x08,

} ECM_AUTO_CALL_STATUS_T;


typedef struct  
{
    int   para_apn_flag;
    char  para_apn[ECM_APN_MAX_LEN];
    int   para_ipv4v6_flag;
    int   para_ipv4v6[ECM_V4V6_MAX_LEN];

} user_cfg_item_t;



typedef struct {

    char       ttyusb_dev[TTYUSB_PATH_LEN];
    char       v4v6[ECM_V4V6_MAX_LEN];
    char       apn[ECM_APN_MAX_LEN];

} ECM_auto_call_txd_para_t ;


typedef  ECM_SM_RSLT_T (*ecm_sm_t)(    ttyusb_dev_t* p_serial,
                                            ECM_SM_EVENT_T evt,
                                            user_cfg_item_t* cfg,
                                            unsigned int* error,
                                            void* in,
                                            void* out) ;


unsigned int ECM_call(char* tty_path, ECM_USR_OPS_T op, char* v4v6para, char* apn);



unsigned int ECM_call_ext(int fd, ECM_USR_OPS_T op, char* v4v6para, char* apn);



unsigned int ECM_auto_call(char* tty_path, char* v4v6para, char* apn);


void            ECM_set_personalization_at(char* person_at);


unsigned int ECM_auto_demo_start(char* tty_path, char* v4v6para, char* apn);



unsigned int    ECM_auto_demo_online(void);



void            ECM_auto_demo_quit_msg(void);



void            ECM_auto_demo_reboot_msg(void);



void            ECM_auto_demo_get_monitor(ECM_auto_monitor_t* ptr);



void            ECM_auto_demo_get_imei(char* imei, unsigned int imei_buf_len);



void            ECM_auto_demo_get_sim_status(char* sim_status, unsigned int sim_status_buf_len);



void            ECM_auto_demo_get_iccid(char* iccid, unsigned int iccid_buf_len);



void            ECM_auto_demo_get_net_type(char* net_type, unsigned int net_type_buf_len);



unsigned int    ECM_auto_demo_get_signal_strength(void);



unsigned int    ECM_auto_demo_get_connect_status(void);



unsigned int    ECM_auto_demo_get_retry_times(void);



unsigned int    ECM_auto_demo_get_error_code(void);


/*add liwei for ali project led operation*/
#if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
void ECM_auto_led_func_start(void);
#endif

/*add liwei for ali project signal led operation*/
#if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
void ECM_auto_siglevl_led_func_start(void);
#endif

/*add liwei for fix gswerr id 0000 start */
#if (ECM_CALL_FIX_GSWERR_ID_0000    ==ECM_DEMO_ON)

unsigned int    ECM_get_gswerr_id_000_status(void);


void            ECM_clr_gswerr_id_000_status(void);

#endif
/*add liwei for fix gswerr id 0000 end */

#endif


