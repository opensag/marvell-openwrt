
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>

#include "ecm_demo_config.h"
#include "ecm_demo_atctl.h"
#include "ecm_demo_ttydev.h"
#include "ecm_demo_msg.h"

#define   ECM_EXIT_METHOD_SLEEP                      1
#define   ECM_EXIT_METHOD_PJOIN                      2
#define   ECM_EXIT_METHOD_DETACH                     3


#define ECM_AUTO_SAFE_EXIT_TIME 10

//#define ECM_CALL_EXIT_METHOD                        ECM_EXIT_METHOD_PJOIN
#define ECM_CALL_EXIT_METHOD                        ECM_EXIT_METHOD_DETACH


typedef enum
{
    ECM_NET_NO_SERVICE=0,
    ECM_NET_LIMIT_SERVICE=1,
    ECM_NET_GSM=2,
    ECM_NET_GPRS=3,
    ECM_NET_CDMA=4,
    ECM_NET_EVDO=5,
    
    ECM_NET_EHRPD=6,
    ECM_NET_UMTS=7,
    ECM_NET_HSDPA=8,
    ECM_NET_HSUPA=9,
    ECM_NET_HSPA=10,
    ECM_NET_HSPA_P=11,
    ECM_NET_LTE=12,
    ECM_NET_TDSCDMA=13,
    ECM_NET_OTHER=14,

} ECM_NET_TYPE_VAL_T ;


static const char           s_network_noserver[]                       = "NO SERVICE" ;
static const char           s_network_limited_server[]                 = "LIMITED SERVICE" ;
static const char           s_network_gsm[]                            = "GSM";
static const char           s_network_gprs[]                           = "GPRS";
static const char           s_network_cdma[]                           = "CDMA";
static const char           s_network_evdo[]                           = "EVDO";
static const char           s_network_ehrpd[]                          = "EHRPD";
static const char           s_network_umts[]                           = "UMTS";
static const char           s_network_hsdpa[]                          = "HSDPA";
static const char           s_network_hsupa[]                          = "HSUPA";
static const char           s_network_hspa[]                           = "HSPA";
static const char           s_network_hspa_plus[]                      = "HSPA+";
static const char           s_network_lte[]                            = "LTE";
static const char           s_network_tdscdma[]                        = "TD-SCDMA";
static const char           s_network_other[]                          = "NET_OTHER";



extern pthread_mutex_t      itc_msg_mux        ;
extern pthread_cond_t       itc_msg_cond;

extern unsigned int         itc_msg_timer           ;
extern unsigned int         itc_msg_terminal;
extern unsigned int         itc_msg_quit;
extern unsigned int         itc_msg_reboot;
extern unsigned int         itc_msg_atcmd           ;

char                        itc_tty_at_buff[TTYUSB_BUF_LEN] = {0};
int                         itc_tty_at_len            = 0;

static unsigned int         s_net_signal_strength = ECM_DFLT_SIG_STRENGTH ;
static unsigned int         s_net_type_val = 0 ;

static char                 s_imei[16] = {0};
static char                 s_iccid[24] = {0};
static char                 s_sim_status[64] = "SIM_NOT_INSERT";

static char                 ECM_personal_at[ECM_PERSONAL_AT_LEN]="at+cops?\r\n";

static unsigned int         s_csq_signal_strength = 0 ;

static struct itimerval   ECM_call_timer;

pthread_mutex_t             usr_monitor_mux         =  PTHREAD_MUTEX_INITIALIZER;
ECM_auto_monitor_t          usr_monitor_info ;

pthread_t                   ECM_auto_call_txd_id = 0 ;

ECM_AUTO_CALL_STATUS_T      ECM_auto_call_status = ECM_AUTO_CALL_INIT ;


ECM_auto_call_txd_para_t ECM_auto_call_configure ;


static char                 ECM_auto_port_at_path[256] = {0};
unsigned int                ECM_auto_port_flag = 0 ;


/*add liwei for issue start*/
ttyusb_dev_t                g_ecm_auto_serial_demo;
/*add liwei for issue end*/

/*add liwei for fix gswerr id 0000 start */
#if (ECM_CALL_FIX_GSWERR_ID_0000    ==ECM_DEMO_ON)
unsigned int                g_gswerr_id_000_find = 0 ;
#endif
/*add liwei for fix gswerr id 0000 end */

/*add liwei for ali project led operation*/
#if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
static unsigned char        ECM_auto_led_function_start = 0 ;
#endif



/*add liwei for ali project signal led operation*/
#if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
static unsigned char        ECM_auto_sig_level_led_func_start = 0 ;
#endif


unsigned int ECM_auto_check_port(ttyusb_dev_t* p_serial);


/*add liwei for ali project led operation*/
#if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
void ECM_auto_led_func_start(void)
{
    ECM_auto_led_function_start = 1 ;
}
#endif

/*add liwei for ali project signal led operation*/
#if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
void ECM_auto_siglevl_led_func_start(void)
{
    ECM_auto_sig_level_led_func_start = 1 ;
}
#endif


void ECM_set_personalization_at(char* person_at)
{
    if (NULL != person_at)
    {
        if (strlen(person_at)<ECM_PERSONAL_AT_LEN)
        {
            bzero(ECM_personal_at,sizeof(ECM_personal_at));
            strcpy(ECM_personal_at,person_at);
            strcat(ECM_personal_at,"\r\n");
            ECM_personal_at[ECM_PERSONAL_AT_LEN-1]='\0';
        }
    }
}

static void ECM_auto_clear_usr_monitor(void)
{
    pthread_mutex_lock(&usr_monitor_mux);
    
    bzero((void*)&usr_monitor_info,sizeof(ECM_auto_monitor_t));
    //strcpy(usr_monitor_info.net_type_str,s_network_noserver) ;
    usr_monitor_info.net_recv_strength = ECM_DFLT_SIG_STRENGTH;
    usr_monitor_info.rf_recv_strength = 0 ;
    pthread_mutex_unlock(&usr_monitor_mux);
}

static void   ECM_auto_save_imei(void)
{
    int i = 0 ;
    int j = 0 ;
    for (j=0,i=0;(j<itc_tty_at_len)&&(j<itc_tty_at_len);j++)
    {            
        if (('0'<=itc_tty_at_buff[j]) && (itc_tty_at_buff[j]<='9'))
        {
            if (i<=(sizeof(s_imei)-1)) {
                s_imei[i] = itc_tty_at_buff[j];                    
                i++;
            }
        }
    }

    if (ECM_IMEI_NUMBERS==strlen(s_imei))
    {
        /*correct imei number*/
        pthread_mutex_lock(&usr_monitor_mux);
        memcpy((void*)(usr_monitor_info.imei), (void*)s_imei, ECM_IMEI_NUMBERS);
        pthread_mutex_unlock(&usr_monitor_mux);
    }
    
}


static void    ECM_auto_save_iccid(void)
{
    int i = 0 ;
    int j = 0 ;
    bzero((void*)s_iccid,sizeof(s_iccid));
    for (j=0,i=0;(j<strlen(itc_tty_at_buff))&&(j<itc_tty_at_len);j++)
    {            
        if (('0'<=itc_tty_at_buff[j]) && (itc_tty_at_buff[j]<='9'))
        {
            if (i<(sizeof(s_iccid)-1)) {
                s_iccid[i] = itc_tty_at_buff[j];
                i++;
            }
        }
    }

    if (ECM_ICCID_NUMBERS >= strlen(s_iccid))
    {
        /*correct iccid number*/
        pthread_mutex_lock(&usr_monitor_mux);
        memcpy((void*)(usr_monitor_info.iccid), (void*)s_iccid, ECM_ICCID_NUMBERS);
        pthread_mutex_unlock(&usr_monitor_mux);
    }

}


static void    ECM_auto_save_csq_strength(void)
{
    int i = 0 ;
    int j = 0 ;

    s_csq_signal_strength = 0 ;

    if ( (NULL!=strstr(itc_tty_at_buff,"+csq")) || (NULL!=strstr(itc_tty_at_buff,"+CSQ")) )
    {
        for (j=0,i=0;(j<strlen(itc_tty_at_buff))&&(j<itc_tty_at_len)&&(i<3);j++)
        {            
            if (('0'<=itc_tty_at_buff[j]) && (itc_tty_at_buff[j]<='9'))
            {
                i++;
                s_csq_signal_strength = s_csq_signal_strength * 10 + ((unsigned int)(itc_tty_at_buff[j]-'0'));                
            }
            else if ( ',' == itc_tty_at_buff[j])
            {
                break;
            }
        }
    }

    /*correct iccid number*/
    pthread_mutex_lock(&usr_monitor_mux);
    usr_monitor_info.rf_recv_strength = s_csq_signal_strength ;
    pthread_mutex_unlock(&usr_monitor_mux);

}


static unsigned int     ECM_auto_save_signal_strength(unsigned int pos)/*pos=5 or post =6 */
{
    unsigned int error = 0;
    unsigned int i = 0 ;
    unsigned int j = 0 ;
    unsigned int cur = 0 ;
    unsigned int signal_strenght = 0;

    do 
    {
        for (i=0,j=0;(i<itc_tty_at_len)&&(i<TTYUSB_BUF_LEN);i++)
        {
            if (itc_tty_at_buff[i]==',')
            {
                j++;
                if (j==pos)
                {
                    cur = i+1 ;
                }
            }
        }

        if (j<=8)
        {
            error = 1;
            break;
        }

        for (i=cur;(i<itc_tty_at_len)&&(i<TTYUSB_BUF_LEN);i++)
        {
            if (itc_tty_at_buff[i]=='-')
            {
                continue;
            }
            else if (itc_tty_at_buff[i]=='.')
            {
                break;
            }
            else if (itc_tty_at_buff[i]==',')
            {
                break;
            }
            else if (('0'<=itc_tty_at_buff[i]) && (itc_tty_at_buff[i]<='9'))
            {
                signal_strenght = signal_strenght*10 + ((unsigned int)(itc_tty_at_buff[i]-'0'));
            }
            else
            {
                error = 2;
                break;
            }
        }

    }    while(0);

    if (0!=error)
    {
        signal_strenght = ECM_DFLT_SIG_STRENGTH ;
    }

    /*save signal strength*/
    #if 0
    pthread_mutex_lock(&usr_monitor_mux);
    usr_monitor_info.net_recv_strength = signal_strenght;
    pthread_mutex_unlock(&usr_monitor_mux);
    #endif

    return signal_strenght ;

}




void  ECM_auto_demo_get_net_type(char* net_type, unsigned int net_type_buf_len)
{
    if ((net_type_buf_len>=32)&&(net_type))
    {
        bzero((void*)net_type,net_type_buf_len);

        pthread_mutex_lock(&usr_monitor_mux);
        memcpy((void*)net_type,(void*)(usr_monitor_info.net_type_str),sizeof(usr_monitor_info.net_type_str)-1);
        pthread_mutex_unlock(&usr_monitor_mux);
    }
}


unsigned int    ECM_auto_demo_get_connect_status(void)
{
    unsigned int var_connect_status = 0 ;

    pthread_mutex_lock(&usr_monitor_mux);
    var_connect_status = usr_monitor_info.connet_status ;
    pthread_mutex_unlock(&usr_monitor_mux);

    return var_connect_status;
}

unsigned int    ECM_auto_demo_get_retry_times(void)
{
    unsigned int var_connect_status = 0 ;

    pthread_mutex_lock(&usr_monitor_mux);
    var_connect_status = usr_monitor_info.retry_times ;
    pthread_mutex_unlock(&usr_monitor_mux);

    return var_connect_status;
}


unsigned int    ECM_auto_demo_get_error_code(void)
{
    unsigned int var_connect_status = 0 ;

    pthread_mutex_lock(&usr_monitor_mux);
    var_connect_status = usr_monitor_info.error_code ;
    pthread_mutex_unlock(&usr_monitor_mux);

    return var_connect_status;
}


void ECM_auto_demo_get_iccid(char* iccid, unsigned int iccid_buf_len)
{
    if ((iccid_buf_len>=21)&&(iccid))
    {
        bzero((void*)iccid,iccid_buf_len);
        pthread_mutex_lock(&usr_monitor_mux);
        memcpy((void*)iccid, (void*)(usr_monitor_info.iccid), ECM_ICCID_NUMBERS);
        pthread_mutex_unlock(&usr_monitor_mux);
    }
}

void  ECM_auto_demo_get_monitor(ECM_auto_monitor_t* ptr)
{
    if (NULL!=ptr)
    {
        pthread_mutex_lock(&usr_monitor_mux);
        memcpy((void*)ptr, (void*)&usr_monitor_info, sizeof(ECM_auto_monitor_t));
        pthread_mutex_unlock(&usr_monitor_mux);
    }
}

void ECM_auto_demo_get_imei(char* imei, unsigned int imei_buf_len)
{
    if ((imei_buf_len>=ECM_IMEI_NUMBERS)&&(imei))
    {
        bzero((void*)imei,imei_buf_len);
        pthread_mutex_lock(&usr_monitor_mux);
        memcpy((void*)imei, (void*)(usr_monitor_info.imei), ECM_IMEI_NUMBERS);
        pthread_mutex_unlock(&usr_monitor_mux);
    }
}

void ECM_auto_demo_get_sim_status(char* sim_status, unsigned int sim_status_buf_len)
{
    if ((sim_status_buf_len>=ECM_SIM_STATUS_LEN)&&(sim_status))
    {
        bzero((void*)sim_status,sim_status_buf_len);
        pthread_mutex_lock(&usr_monitor_mux);
        memcpy((void*)sim_status, (void*)(usr_monitor_info.sim_status), ECM_SIM_STATUS_LEN);
        pthread_mutex_unlock(&usr_monitor_mux);
    }
}


unsigned int    ECM_auto_demo_get_signal_strength(void)
{
    unsigned int var_strength=0;
    pthread_mutex_lock(&usr_monitor_mux);
    var_strength = usr_monitor_info.net_recv_strength ;
    pthread_mutex_unlock(&usr_monitor_mux);
    return var_strength;
}

void ECM_error_map(unsigned int error, char* error_info, char* error_ext_info)
{
    char str_error_tip[ECM_AUTOCFG_ERR_INFO_LEN] = {0};
    char str_error_ext_tip[ECM_AUTOCFG_ERR_INFO_LEN] = {0};


    sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_OTHER);

    switch (error)
    {
    case E_ECM_CALL_SUCCESS:
        sprintf(str_error_tip,"ECM call success");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SUCCESS);
        break;
    case E_TTYUSB_CONFIG_TTY_S_NULL:
        sprintf(str_error_tip,"TTY struct is empty");
        break;
    case E_TTYUSB_CONFIG_PATH_NULL:
        sprintf(str_error_tip,"Undefined path");
        break;
    case E_TTYUSB_CONFIG_PATH_LENG:
        sprintf(str_error_tip,"Device path length error");
        break;
    case E_TTYUSB_CONFIG_OVER_MAX:
        sprintf(str_error_tip,"Device path over max 63 bytes");
        break;
    case E_TTYUSB_OPEN_TTY_S_NULL:
        sprintf(str_error_tip,"TTY struct is empty");
        break;
    case E_TTYUSB_OPEN_TTY_DEV_FAIL:
        sprintf(str_error_tip,"Open device ttyUSBX fail");
        break;
    case E_TTYUSB_OPEN_TTY_BAK_CFG:
        sprintf(str_error_tip,"Back device configure fail");
        break;
    case E_TTYUSB_OPEN_TTY_FCNTL_ERR:
        sprintf(str_error_tip,"Back device configure fail");
        break;
    case E_TTYUSB_CLOSE_TTY_S_NULL:
        sprintf(str_error_tip,"TTY struct is empty");
        break;
    case E_TTYUSB_CLOSE_TTY_FD_ERR:
        sprintf(str_error_tip,"TTY fd error");
        break;
    case E_TTYUSB_SEND_PARA_ERR:
        sprintf(str_error_tip,"TTY send para error");
        break;
    case E_TTYUSB_SEND_LENTH_ERR:
        sprintf(str_error_tip,"TTY send at cmd length error");
        break;
    case E_TTYUSB_SEND_FD_ERR:
        sprintf(str_error_tip,"TTY fd error");
        break;
    case E_TTYUSB_SEND_ABNORMAL:
        sprintf(str_error_tip,"TTY send abnormal error");
        break;
    case E_TTYUSB_REG_CB_PARA_NULL:
        sprintf(str_error_tip,"TTY receive callback null");
        break;
    case E_TTYUSB_START_RECV_TTY_S_NULL:
        sprintf(str_error_tip,"TTY struct is empty");
        break;
    case E_TTYUSB_START_RECV_NEW_TXD_FAIL:
        sprintf(str_error_tip,"Create thread fail");
        break;
    case E_TTYUSB_START_RECV_TXD_BOOT_FAIL:
        sprintf(str_error_tip,"Recv thread run abnormal");
        break;
    case E_TTYUSB_RECV_THXD_PARA_NULL:
        sprintf(str_error_tip,"Recv thread data fail");
        break;
    case E_TTYUSB_RECV_THXD_BE_TERMINAL:
        sprintf(str_error_tip,"Recv thread killed");
        break;
    case E_ECM_CALL_START_EXT_FD_ERR:
        sprintf(str_error_tip,"User input error fd");
        break;
    case E_ECM_EXEC_AT_FLOW_PARA_NULL:
        sprintf(str_error_tip,"Parameter empty");
        break;
    case E_ECM_EXEC_AT_FLOW_SM0_RET_FAIL:
        sprintf(str_error_tip,"State machine return fail");
        break;
    case E_ECM_EXEC_AT_FLOW_SM_RET_FAIL:
        sprintf(str_error_tip,"State machine return fail");
        break;
    case E_ECM_EXEC_AT_FLOW_SM_BE_TERM:
        sprintf(str_error_tip,"State machine be terminal");
        break;
    case E_ECM_EXEC_AT_FLOW_SM_SWITCH_FAIL:
        sprintf(str_error_tip,"State machine switch fail");
        break;
    case E_ECM_CALL_PARA_UNPACK_FAIL:
        sprintf(str_error_tip,"User input error para");
        break;
    case E_ECM_CALL_USR_TERMINAL:
        sprintf(str_error_tip,"User Terminated");
        break;
    case E_ECM_CALL_TTY_USR_OPEN_FAIL:
        sprintf(str_error_tip,"TTY User open fail");
        break;
    case E_ECM_PORT_JUMP:
        sprintf(str_error_tip,"Port error");
        break;

    case E_ECM_ATI_SEND_FAIL:
        sprintf(str_error_tip,"ATI Send Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ATI_RECV_ISSUE:
        sprintf(str_error_tip,"ATI Recv Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ATI_SEND_MAX:
        sprintf(str_error_tip,"ATI Send Over Max");        
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ATI_RESEND_ERR:
        sprintf(str_error_tip,"ATI Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ATI_OTHER_ERR:
        sprintf(str_error_tip,"ATI Send Other Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);        
        break;
    case E_ECM_ATI_TERMINAL:
        sprintf(str_error_tip,"ATI Send Terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_ZSWITCH_QUERY_FAIL:
        sprintf(str_error_tip,"at+zswitch? Send Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZSWITCH_QRECV_ISSUE:
        sprintf(str_error_tip,"at+zswitch? Recv Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZSWITCH_QUERY_MAX:
        sprintf(str_error_tip,"at+zswitch? Send Over Max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZSWITCH_QRESEND_ERR:
        sprintf(str_error_tip,"at+zswitch? Re-send Over Max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZSWITCH_QUERY_TERM:
        sprintf(str_error_tip,"at+zswitch? Send Terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    case E_ECM_ZSWITCH_QOTHER_ERR:
        sprintf(str_error_tip,"at+zswitch? Send Other Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);        
        break;

    case E_ECM_ZSWITCH_SEND_FAIL:
        sprintf(str_error_tip,"at+zswitch=L Send Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZSWITCH_RECV_ISSUE:
        sprintf(str_error_tip,"at+zswitch=L Recv Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZSWITCH_SEND_MAX:
        sprintf(str_error_tip,"at+zswitch=L Send Over Max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZSWITCH_RESEND_ERR:
        sprintf(str_error_tip,"at+zswitch=L Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZSWITCH_OTHER_ERR:
        sprintf(str_error_tip,"at+zswitch=L Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_ZSWITCH_TERMINAL:
        sprintf(str_error_tip,"at+zswitch=L Send Terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_ZADSET_QUERY_FAIL:
        sprintf(str_error_tip,"AT+ZADSET? Send Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZADSET_QRECV_ISSUE:
        sprintf(str_error_tip,"AT+ZADSET? Recv Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZADSET_QUERY_MAX:
        sprintf(str_error_tip,"AT+ZADSET? Send Over Max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZADSET_QRESEND_ERR:
        sprintf(str_error_tip,"AT+ZADSET? Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZADSET_QUERY_TERM:
        sprintf(str_error_tip,"AT+ZADSET? Send Terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    case E_ECM_ZADSET_QOTHER_ERR:
        sprintf(str_error_tip,"AT+ZADSET? Send Other Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;

    case E_ECM_ZADSET_SEND_FAIL:
        sprintf(str_error_tip,"AT+ZADSET=E Send Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZADSET_RECV_ISSUE:
        sprintf(str_error_tip,"AT+ZADSET=E Recv Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZADSET_SEND_MAX:
        sprintf(str_error_tip,"AT+ZADSET=E Send Over Max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZADSET_RESEND_ERR:
        sprintf(str_error_tip,"AT+ZADSET=E Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZADSET_OTHER_ERR:
        sprintf(str_error_tip,"AT+ZADSET=E Send Other Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_ZADSET_TERMINAL:
        sprintf(str_error_tip,"AT+ZADSET=E Send Terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_CFUN_QUERY_FAIL:
        sprintf(str_error_tip,"at+cfun? Send Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CFUN_QRECV_ISSUE:
        sprintf(str_error_tip,"at+cfun? Recv Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_CFUN_QUERY_MAX:
        sprintf(str_error_tip,"at+cfun? Send Over Max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_CFUN_QRESEND_ERR:
        sprintf(str_error_tip,"at+cfun? Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CFUN_QUERY_TERM:
        sprintf(str_error_tip,"at+cfun? Send Terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    case E_ECM_CFUN_QOTHER_ERR:
        sprintf(str_error_tip,"at+cfun? Send Other Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;

    case E_ECM_CPIN_QUERY_FAIL:
        sprintf(str_error_tip,"at+cpin? send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CPIN_QNOT_INSERT:
        sprintf(str_error_tip,"at+cpin? SIM not insert");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_SIM_NOT_INSERT);
        break;
    case E_ECM_CPIN_QRECV_ISSUE:
        sprintf(str_error_tip,"at+cpin? recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_CPIN_QUERY_MAX:
        sprintf(str_error_tip,"at+cpin? send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_CPIN_QRESEND_ERR:
        sprintf(str_error_tip,"at+cpin? Resend Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CPIN_QUERY_TERM:
        sprintf(str_error_tip,"at+cpin? Send Terminiated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    case E_ECM_CPIN_QOTHER_ERR:
        sprintf(str_error_tip,"at+cpin? Send Other Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_CPIN_Q_SIM_BUSY:
        sprintf(str_error_tip,"at+cpin? SIM busy");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_SIM_BUSY);
        break;
    case E_ECM_CPIN_Q_SIM_PIN:
        sprintf(str_error_tip,"at+cpin? SIM PIN");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_SIM_PIN);
        break;
    case E_ECM_CPIN_Q_SIM_PUK:
        sprintf(str_error_tip,"at+cpin? SIM PUK");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_SIM_PUK);
        break;
    case E_ECM_CPIN_Q_SIM_FAILURE:
        sprintf(str_error_tip,"at+cpin? SIM failure");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_SIM_FAILURE);        
        break;

    case E_ECM_ZBAND_QUERY_FAIL:
        sprintf(str_error_tip,"at+zband? send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZBAND_QRECV_ISSUE:
        sprintf(str_error_tip,"at+zband? recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZBAND_QUERY_MAX:
        sprintf(str_error_tip,"at+zband? send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZBAND_QRESEND_ERR:
        sprintf(str_error_tip,"at+zband? Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZBAND_QUERY_TERM:
        sprintf(str_error_tip,"at+zband? Send Terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    case E_ECM_ZBAND_QOTHER_ERR:
        sprintf(str_error_tip,"at+zband? Send Other Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;

    case E_ECM_CSQ_QUERY_FAIL:
        sprintf(str_error_tip,"at+csq Send Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CSQ_QRECV_ISSUE:
        sprintf(str_error_tip,"at+csq Recv Fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_CSQ_QUERY_MAX:
        sprintf(str_error_tip,"at+csq Send Over Max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_CSQ_QRESEND_ERR:
        sprintf(str_error_tip,"at+csq Re-send Error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CSQ_QUERY_TERM:
        sprintf(str_error_tip,"at+csq Send Terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    case E_ECM_CSQ_QOTHER_ERR:
        sprintf(str_error_tip,"at+csq Send Other Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;

    case E_ECM_CREG_QUERY_FAIL:
        sprintf(str_error_tip,"at+creg? send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CREG_QRECV_ISSUE:
        sprintf(str_error_tip,"at+creg? recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_NET_REG_ERROR);
        break;
    case E_ECM_CREG_QUERY_MAX:
        sprintf(str_error_tip,"at+creg? send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_CREG_QRESEND_ERR: 
        sprintf(str_error_tip,"at+creg? Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CREG_QUERY_TERM:
        sprintf(str_error_tip,"at+creg? send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    case E_ECM_CREG_QOTHER_ERR:
        sprintf(str_error_tip,"at+creg: send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;

    case E_ECM_CGDCONT_SEND_FAIL:
        sprintf(str_error_tip,"at+cgdcont=,, send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);       
        break;
    case E_ECM_CGDCONT_RECV_ISSUE:
        sprintf(str_error_tip,"at+cgdcont=,, recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_CGDCONT_SEND_MAX:
        sprintf(str_error_tip,"at+cgdcont=,, send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_CGDCONT_RESEND_ERR:
        sprintf(str_error_tip,"at+cgdcont=,, Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CGDCONT_OTHER_ERR:
        sprintf(str_error_tip,"at+cgdcont=,, send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_CGDCONT_TERMINAL:
        sprintf(str_error_tip,"at+cgdcont: send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_ZECMCALL1_SEND_FAIL:
        sprintf(str_error_tip,"at+zecmcall=1 send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_DIAL_FAIL);
        break;
    case E_ECM_ZECMCALL1_RECV_ISSUE:
        sprintf(str_error_tip,"at+zecmcall=1 recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_DIAL_BUT_NO_RES);
        break;
    case E_ECM_ZECMCALL1_SEND_MAX:
        sprintf(str_error_tip,"at+zecmcall=1 send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_DIAL_BUT_NO_RES);
        break;
    case E_ECM_ZECMCALL1_RESEND_ERR:
        sprintf(str_error_tip,"at+zecmcall=1 Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_DIAL_FAIL);
        break;
    case E_ECM_ZECMCALL1_OTHER_ERR:
        sprintf(str_error_tip,"at+zecmcall=1 send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_ZECMCALL1_TERMINAL:
        sprintf(str_error_tip,"at+zecmcall=1: send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_ZECMCALL0_SEND_FAIL:
        sprintf(str_error_tip,"at+zecmcall=0 send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZECMCALL0_RECV_ISSUE:
        sprintf(str_error_tip,"at+zecmcall=0 recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZECMCALL0_SEND_MAX:
        sprintf(str_error_tip,"at+zecmcall=0 send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZECMCALL0_RESEND_ERR:
        sprintf(str_error_tip,"at+zecmcall=0 Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZECMCALL0_OTHER_ERR:
        sprintf(str_error_tip,"at+zecmcall=0 send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_ZECMCALL0_TERMINAL:
        sprintf(str_error_tip,"at+zecmcall=0 send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_ZECMCALL_C_QUERY_FAIL:
        sprintf(str_error_tip,"at+zecmcall? send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZECMCALL_C_QRECV_ISSUE:
        sprintf(str_error_tip,"at+zecmcall? recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZECMCALL_C_QSEND_MAX:
        sprintf(str_error_tip,"at+zecmcall? send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZECMCALL_C_QRESEND_ERR:
        sprintf(str_error_tip,"at+zecmcall? Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZECMCALL_C_OTHER_ERR:
        sprintf(str_error_tip,"at+zecmcall? Send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_ZECMCALL_C_TERMINAL:
        sprintf(str_error_tip,"at+zecmcall? send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    
    case E_ECM_CFUN_1_1_SEND_FAIL:
        sprintf(str_error_tip,"at+cfun=1,1 send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CFUN_1_1_RECV_ISSUE:
        sprintf(str_error_tip,"at+cfun=1,1 recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_CFUN_1_1_SEND_MAX:
        sprintf(str_error_tip,"at+cfun=1,1 send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_CFUN_1_1_RESEND_ERR:
        sprintf(str_error_tip,"at+cfun=1,1 Re-send Err");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CFUN_1_1_OTHER_ERR:
        sprintf(str_error_tip,"at+cfun=1,1: Send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_CFUN_1_1_TERMINAL:
        sprintf(str_error_tip,"at+cfun=1,1: Send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;


    case E_ECM_ZPDPTYPE_QUERY_FAIL:
        sprintf(str_error_tip,"at+zpdpcall? send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZPDPTYPE_QRECV_ISSUE:
        sprintf(str_error_tip,"at+zpdpcall? recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_NET_DROP);
        break;
    case E_ECM_ZPDPTYPE_QSEND_MAX:
        sprintf(str_error_tip,"at+zpdpcall? send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZPDPTYPE_QRESEND_ERR:
        sprintf(str_error_tip,"at+zpdpcall? resend error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZPDPTYPE_OTHER_ERR:
        sprintf(str_error_tip,"at+zpdpcall? send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_ZPDPTYPE_TERMINAL:
        sprintf(str_error_tip,"at+zpdpcall? send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_CGSN_QUERY_FAIL:
        sprintf(str_error_tip,"at+cgsn send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CGSN_QRECV_ISSUE:
        sprintf(str_error_tip,"at+cgsn recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_CGSN_QSEND_MAX:
        sprintf(str_error_tip,"at+cgsn send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_CGSN_QRESEND_ERR:
        sprintf(str_error_tip,"at+cgsn resend error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_CGSN_OTHER_ERR:
        sprintf(str_error_tip,"at+cgsn send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_CGSN_TERMINAL:
        sprintf(str_error_tip,"at+cgsn send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_ZGETICCID_QUERY_FAIL:
        sprintf(str_error_tip,"at+zgeticcid send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZGETICCID_QRECV_ISSUE:
        sprintf(str_error_tip,"at+zgeticcid recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZGETICCID_QSEND_MAX:
        sprintf(str_error_tip,"at+zgeticcid send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZGETICCID_QRESEND_ERR:
        sprintf(str_error_tip,"at+zgeticcid retry-send error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZGETICCID_OTHER_ERR:
        sprintf(str_error_tip,"at+zgeticcid send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_ZGETICCID_TERMINAL:
        sprintf(str_error_tip,"at+zgeticcid send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;


    case E_ECM_ZPAS_QUERY_FAIL:
        sprintf(str_error_tip,"at+zpas? send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;        
    case E_ECM_ZPAS_QNO_SERVICE:
        sprintf(str_error_tip,"at+zpas? no servicel");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_NO_SERVICE);
        break;
    case E_ECM_ZPAS_QLIMIT_SERVICE:
        sprintf(str_error_tip,"at+zpas? limit service");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_LIMIT_SERVICE);
        break;
    case E_ECM_ZPAS_QRECV_ISSUE:
        sprintf(str_error_tip,"at+zpas? recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZPAS_QSEND_MAX:
        sprintf(str_error_tip,"at+zpas? send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZPAS_QRESEND_ERR:
        sprintf(str_error_tip,"at+zpas? resend error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZPAS_OTHER_ERR:
        sprintf(str_error_tip,"at+zpas? send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_ZPAS_TERMINAL:
        sprintf(str_error_tip,"at+zpas? send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_ZSDT_SEND_FAIL:
        sprintf(str_error_tip,"at+zsdt=,, send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZSDT_RECV_ISSUE:
        sprintf(str_error_tip,"at+zsdt=,, recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZSDT_SEND_MAX:
        sprintf(str_error_tip,"at+zsdt=,, send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZSDT_RESEND_ERR:
        sprintf(str_error_tip,"at+zsdt=,, re-send error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZSDT_OTHER_ERR:
        sprintf(str_error_tip,"at+zsdt=,, send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_ZSDT_TERMINAL:
        sprintf(str_error_tip,"a+zsdt=,, send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_ZSDT_QUERY_FAIL:
        sprintf(str_error_tip,"at+zsdt? send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZSDT_QRECV_ISSUE:
        sprintf(str_error_tip,"at+zsdt? recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZSDT_QUERY_MAX:
        sprintf(str_error_tip,"at+zsdt? send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZSDT_QRESEND_ERR:
        sprintf(str_error_tip,"at+zsdt? re-send error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZSDT_QOTHER_ERR:
        sprintf(str_error_tip,"at+zsdt? send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
    case E_ECM_ZSDT_QUERY_TERM:
        sprintf(str_error_tip,"a+zsdt? send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;

    case E_ECM_ZCDS_QUERY_FAIL:
        sprintf(str_error_tip,"at+ZCDS? send fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZCDS_QRECV_ISSUE:
        sprintf(str_error_tip,"at+ZCDS? recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_ZCDS_QUERY_MAX:
        sprintf(str_error_tip,"at+ZCDS? send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_ZCDS_QRESEND_ERR:
        sprintf(str_error_tip,"at+ZCDS? re-send error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_ZCDS_QUERY_TERM:
        sprintf(str_error_tip,"at+ZCDS? send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    case E_ECM_ZCDS_QOTHER_ERR:
        sprintf(str_error_tip,"at+ZCDS? send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;

    case E_ECM_PERSON_AT_SEND_FAIL:
        sprintf(str_error_tip,"Personalization at send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_PERSON_AT_RECV_ISSUE:
        sprintf(str_error_tip,"Personalization at recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_PERSON_AT_SEND_MAX:
        sprintf(str_error_tip,"Personalization at send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_PERSON_AT_RESEND_ERR:
        sprintf(str_error_tip,"Personalization at re-send error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_PERSON_AT_OTHER_ERR:
        sprintf(str_error_tip,"Personalization at send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    case E_ECM_PERSON_AT_TERMINAL:
        sprintf(str_error_tip,"Personalization at send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;

        /*add liwei for fix gswerr id 0000 start */
#if (ECM_CALL_FIX_GSWERR_ID_0000    ==ECM_DEMO_ON)
    case E_ECM_GSWERR_QUERY_FAIL:
        sprintf(str_error_tip,"AT+GSWERR=0000 send failure");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_GSWERR_QRECV_ISSUE:
        sprintf(str_error_tip,"AT+GSWERR=0000 recv fail");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_RECV_AT_ERROR);
        break;
    case E_ECM_GSWERR_QUERY_MAX:
        sprintf(str_error_tip,"AT+GSWERR=0000 send over max");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_OVERMAX_ERR);
        break;
    case E_ECM_GSWERR_QRESEND_ERR:
        sprintf(str_error_tip,"AT+GSWERR=0000 re-send error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_SEND_AT_ERROR);
        break;
    case E_ECM_GSWERR_QOTHER_ERR:
        sprintf(str_error_tip,"AT+GSWERR=0000 send other error");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_ERROR_TERMINATED);
        break;
    case E_ECM_GSWERR_QTERMINAL:
        sprintf(str_error_tip,"AT+GSWERR=0000 send terminated");
        sprintf(str_error_ext_tip,ECM_AUTOCFG_E_OTHER_AT_ERROR);
        break;
#endif
        /*add liwei for fix gswerr id 0000 end */

    default:
        sprintf(str_error_tip,"Other Error");
        break;
    }

    str_error_tip[ECM_AUTOCFG_ERR_INFO_LEN-1] = '\0';
    
    if (NULL!=error_info)
    {
        sprintf(error_info,"%s", str_error_tip);
    }

    if (NULL!=error_ext_info)
    {
        sprintf(error_ext_info,"%s", str_error_ext_tip);
    }    

}



void ECM_delete_echo_at(    const char* echo_at)
{
    char *          cur_begin = NULL ;
    char *          cur_pos     = NULL ;
    char *          cur_end     = NULL ;
    unsigned int offset = 0 ;

    if (itc_tty_at_buff[TTYUSB_BUF_LEN-1] == '\0' )
    {
        ECM_log(ECM_LOG_L_3,"[info] recv %s",itc_tty_at_buff);
    }
    else
    {
        ECM_log(ECM_LOG_L_3,
            "[info] recv length:%d,data:%02X %02X %02X %02X %02X %02X %02X %02X "
            "%02X %02X %02X %02X %02X %02X %02X %02X",      itc_tty_at_len,
            itc_tty_at_buff[0],     itc_tty_at_buff[1],     itc_tty_at_buff[2],     itc_tty_at_buff[3],
            itc_tty_at_buff[4],     itc_tty_at_buff[5],     itc_tty_at_buff[6],     itc_tty_at_buff[7],
            itc_tty_at_buff[8],     itc_tty_at_buff[9],     itc_tty_at_buff[10],    itc_tty_at_buff[11],
            itc_tty_at_buff[12],    itc_tty_at_buff[13],    itc_tty_at_buff[14],    itc_tty_at_buff[15]   );
    }

    if ((NULL != echo_at) && (itc_tty_at_buff[TTYUSB_BUF_LEN-1] == '\0' ))
    {
        cur_pos = strstr(itc_tty_at_buff,echo_at) ;
        
        if (NULL != cur_pos)
        {
            /*remove echo_at*/

            cur_begin = cur_pos + strlen(echo_at);
        
            if (itc_tty_at_len<=TTYUSB_BUF_LEN-1)
            {
                cur_end = &(itc_tty_at_buff[itc_tty_at_len-1]) ;
            }
            else
            {
                cur_end = &(itc_tty_at_buff[TTYUSB_BUF_LEN-1]) ;
            }

            /*remove \r and \n */
            while( ((*cur_begin)=='\r') || ((*cur_begin)=='\n') )
            {
                if ((cur_begin <= cur_end) && (cur_begin > itc_tty_at_buff))
                {
                    cur_begin++;
                }
                else
                {
                    break;
                }
            }

            for (offset=0,cur_pos=cur_begin;(cur_pos <= cur_end) && (cur_pos > itc_tty_at_buff);
                offset++, cur_pos++)
            {
                itc_tty_at_buff[offset] = *cur_pos;
            }

            itc_tty_at_len = offset ;

            for (;offset<TTYUSB_BUF_LEN;offset++)
            {
                itc_tty_at_buff[offset] = 0 ;
            }

            if (0!=itc_tty_at_len)
            {
                //ECM_log(ECM_LOG_L_3,"[info] rm echo len:%d,at=%s",itc_tty_at_len,itc_tty_at_buff);
                itc_tty_at_buff[TTYUSB_BUF_LEN-1]='\0';
                ECM_log(ECM_LOG_L_4,"[info] rm echo:%s",itc_tty_at_buff);
            }

        }

    }

}

unsigned int ECM_is_at_with_content(void)
{
    unsigned int error = 0 ;
    unsigned int offset = 0 ;
    unsigned int has_none_echo_flg = 0 ;

    for (offset=0;(offset<itc_tty_at_len)&&(offset<TTYUSB_BUF_LEN);offset++)
    {
        if ((itc_tty_at_buff[offset] != '\r') && (itc_tty_at_buff[offset] != '\n') && (itc_tty_at_buff[offset] != '\0'))
        {
            has_none_echo_flg = 1 ;
            break;
        }
    }

    return has_none_echo_flg ;
}


void ECM_usr_cfg_init(user_cfg_item_t* cfg)
{
    if (cfg)
    {
        bzero((void*)cfg,sizeof(user_cfg_item_t));
    }
}


void ECM_usr_cfg_set(user_cfg_item_t* cfg, char* ipv4v6, char* apn)
{

    if ((cfg)&&(ipv4v6))
    {
        if ((strlen(ipv4v6)<ECM_V4V6_MAX_LEN)&&(0<strlen(ipv4v6)))
        {           
            cfg->para_ipv4v6_flag = 1 ;
            memcpy((void*)cfg->para_ipv4v6,(void*)ipv4v6,strlen(ipv4v6));
        }
    }

    if ((cfg)&&(apn))
    {
        if ((strlen(apn)<ECM_APN_MAX_LEN)&&(0<strlen(apn)))
        {           
            cfg->para_apn_flag = 1 ;
            memcpy((void*)cfg->para_apn,(void*)apn,strlen(apn));
        }
        else
        {
            cfg->para_apn_flag = 0 ;
        }
    }

}


/*add liwei for fix gswerr id 0000 start */
#if (ECM_CALL_FIX_GSWERR_ID_0000    ==ECM_DEMO_ON)
unsigned int  ECM_get_gswerr_id_000_status(void)
{
    return g_gswerr_id_000_find;
}
void  ECM_clr_gswerr_id_000_status(void)
{
    g_gswerr_id_000_find = 0 ;
}

ECM_SM_RSLT_T ECM_query_tty_gswerr_id_000(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*     cmd_str = "AT+GSWERR=0000\r\n";
    const char*     cmd_str_rsp="OK\r\n";
    static unsigned    int  cmd_retry_cnt = 0 ;

    const char*      echo_at     = "AT+GSWERR=0000";
    ECM_SM_RSLT_T    ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_GSWERR_QUERY_FAIL ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
            /* add liwei for bug fix start */
            ITC_clear_timer(&ECM_call_timer);
            /* add liwei for bug fix end */
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {        
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else  if( (NULL!=strstr(itc_tty_at_buff,"+"))
            &&     (NULL==strstr(itc_tty_at_buff,"+GSWERR:"))
            &&     (NULL==strstr(itc_tty_at_buff,"+gswerr:")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ( (NULL != strstr(itc_tty_at_buff,"+GSWERR:0000000A"))
            || (NULL != strstr(itc_tty_at_buff,"+gswerr:0000000a")) )
        {
            *error = 0 ;
            ITC_clear_timer(&ECM_call_timer);
            g_gswerr_id_000_find = 0 ;
            ECM_log(ECM_LOG_L_3,"[info] egress format is 0000000A");
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else if ( (NULL != strstr(itc_tty_at_buff,"+GSWERR:00000000"))
            || (NULL != strstr(itc_tty_at_buff,"+gswerr:00000000")) )
        {
            *error = 0 ;
            ITC_clear_timer(&ECM_call_timer);
            g_gswerr_id_000_find = 1 ;
            ECM_log(ECM_LOG_L_3,"[info] egress format error 00000000");
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            *error = 0 ;
            ITC_clear_timer(&ECM_call_timer);
            g_gswerr_id_000_find = 0 ;
            ECM_log(ECM_LOG_L_3,"[info] egress format Not Support");
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_GSWERR_QUERY_MAX ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_GSWERR_QRESEND_ERR ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_GSWERR_QTERMINAL ;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_GSWERR_QOTHER_ERR ;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}


#endif
/*add liwei for fix gswerr id 0000 end */


ECM_SM_RSLT_T ECM_query_tty_ati(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*     cmd_str = "ati\r\n";
    const char*     cmd_str_rsp="OK\r\n";
    static unsigned    int  cmd_retry_cnt = 0 ;

    const char*     echo_at     = "ati";
    
    ECM_SM_RSLT_T    ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_ATI_SEND_FAIL ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
            /* add liwei for bug fix start */
            ITC_clear_timer(&ECM_call_timer);
            /* add liwei for bug fix end */

        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {        
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else  if (NULL!=strstr(itc_tty_at_buff,"+"))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_ATI_RECV_ISSUE ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ATI_SEND_MAX ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ATI_RESEND_ERR ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ATI_TERMINAL ;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ATI_OTHER_ERR ;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}









ECM_SM_RSLT_T ECM_query_tty_zswitch(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*     cmd_str="at+zswitch?\r\n";
    const char*     cmd_str_rsp1="+ZSWITCH: l";
    const char*     cmd_str_rsp2="+ZSWITCH: L";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zswitch?";
    
    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_ZSWITCH_QUERY_FAIL ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {

        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else  if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZSWITCH")))            
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp1))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp2))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_ZSWITCH_QRECV_ISSUE ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZSWITCH_QUERY_MAX ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            
            *error = E_ECM_ZSWITCH_QRESEND_ERR ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZSWITCH_QUERY_TERM ;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZSWITCH_QOTHER_ERR ;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}

ECM_SM_RSLT_T ECM_send_tty_zswitch(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*     cmd_str="at+zswitch=l\r\n";
    const char*     cmd_str_rsp="OK\r\n";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zswitch=l";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);

            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_ZSWITCH_SEND_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else  if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZSWITCH")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            *error = E_ECM_ZSWITCH_RECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZSWITCH_SEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZSWITCH_RESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZSWITCH_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZSWITCH_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}



ECM_SM_RSLT_T ECM_query_tty_zadset(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*     cmd_str="at+zadset?\r\n";
    const char*     cmd_str_rsp="OK\r\n"; /*if online*/

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zadset?";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_ZADSET_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if(  (NULL!=strstr(itc_tty_at_buff,"+")) 
               && (NULL==strstr(itc_tty_at_buff,"+ZADSET"))
               && (NULL==strstr(itc_tty_at_buff,"+zadset")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_ZADSET_QRECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZADSET_QUERY_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZADSET_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZADSET_QUERY_TERM;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;        
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZADSET_QOTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}

ECM_SM_RSLT_T ECM_send_tty_zadset(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*     cmd_str="at+zadset=e\r\n";
    const char*     cmd_str_rsp="OK\r\n"; /*if online*/

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zadset=e";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_ZADSET_SEND_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {        

        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else  if( (NULL!=strstr(itc_tty_at_buff,"+"))
            &&     (NULL==strstr(itc_tty_at_buff,"+ZADSET"))
            &&     (NULL==strstr(itc_tty_at_buff,"+zadset")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            *error = E_ECM_ZADSET_RECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZADSET_SEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZADSET_RESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZADSET_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;        
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZADSET_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}





ECM_SM_RSLT_T ECM_query_tty_cfun(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*     cmd_str="at+cfun?\r\n";
    const char*     cmd_str_rsp="+CFUN: 1"; /*if online*/
    const char*     cmd_str_rsp1="+cfun: 1"; /*if online*/

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+cfun?";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_CFUN_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {

        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+CFUN:"))
            && (NULL==strstr(itc_tty_at_buff,"+cfun:")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp1))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_CFUN_QRECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_CFUN_QUERY_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_CFUN_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_CFUN_QUERY_TERM;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_CFUN_QOTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}



ECM_SM_RSLT_T ECM_query_tty_cpin(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*        cmd_str="at+cpin?\r\n";

    const char*        cmd_str_rsp="+CPIN: READY";

    const char*        cmd_str_not_insert = "SIM not inserted";//"+CME ERROR: SIM not inserted"

    const char*        cmd_str_sim_busy = "SIM busy";    //+CME ERROR: SIM busy

    const char*        cmd_str_sim_pin = "SIM PIN";    //+CPIN: SIM PIN

    const char*        cmd_str_sim_puk = "SIM PUK";     //+CPIN: SIM PUK

    const char*        cmd_str_sim_failure = "SIM failure";  //+CME ERROR: SIM failure

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+cpin?";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        //strcpy(s_sim_status,"SIM_NOT_INSERT");
        //pthread_mutex_lock(&usr_monitor_mux);
        //sprintf(usr_monitor_info.sim_status,"%s",s_sim_status);
        //pthread_mutex_unlock(&usr_monitor_mux);
    
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_CPIN_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {

        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+CPIN:"))
            && (NULL==strstr(itc_tty_at_buff,"+cpin:")) && (NULL==strstr(itc_tty_at_buff,"+CME ERROR")) )
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            strcpy(s_sim_status,ECM_AUTOCFG_SIM_READY);
            pthread_mutex_lock(&usr_monitor_mux);
            sprintf(usr_monitor_info.sim_status,"%s",s_sim_status);
            pthread_mutex_unlock(&usr_monitor_mux);

            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_not_insert))
        {
            strcpy(s_sim_status,ECM_AUTOCFG_SIM_NOT_INSERT);
            pthread_mutex_lock(&usr_monitor_mux);
            sprintf(usr_monitor_info.sim_status,"%s",s_sim_status);
            bzero((void*)(usr_monitor_info.iccid),ECM_AUTOCFG_ICCID_LEN);
            pthread_mutex_unlock(&usr_monitor_mux);

            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
            *error = E_ECM_CPIN_QNOT_INSERT;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_off();
            }
            #endif

            /*add liwei for ali project signal led operation*/
            #if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_sig_level_led_func_start)
            {
                ECM_aux_set_siglevel_led_off();
            }
            #endif


        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_sim_busy))
        {
            strcpy(s_sim_status,ECM_AUTOCFG_SIM_BUSY);
            pthread_mutex_lock(&usr_monitor_mux);
            sprintf(usr_monitor_info.sim_status,"%s",s_sim_status);
            bzero((void*)(usr_monitor_info.iccid),ECM_AUTOCFG_ICCID_LEN);
            pthread_mutex_unlock(&usr_monitor_mux);

            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
            *error = E_ECM_CPIN_Q_SIM_BUSY;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_off();
            }
            #endif

            /*add liwei for ali project signal led operation*/
            #if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_sig_level_led_func_start)
            {
                ECM_aux_set_siglevel_led_off();
            }
            #endif


        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_sim_pin))
        {
            strcpy(s_sim_status,ECM_AUTOCFG_SIM_PIN);
            pthread_mutex_lock(&usr_monitor_mux);
            sprintf(usr_monitor_info.sim_status,"%s",s_sim_status);
            bzero((void*)(usr_monitor_info.iccid),ECM_AUTOCFG_ICCID_LEN);
            pthread_mutex_unlock(&usr_monitor_mux);

            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
            *error = E_ECM_CPIN_Q_SIM_PIN;            

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_off();
            }
            #endif

            /*add liwei for ali project signal led operation*/
            #if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_sig_level_led_func_start)
            {
                ECM_aux_set_siglevel_led_off();
            }
            #endif


        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_sim_puk))
        {
            strcpy(s_sim_status,ECM_AUTOCFG_SIM_PUK);
            pthread_mutex_lock(&usr_monitor_mux);
            sprintf(usr_monitor_info.sim_status,"%s",s_sim_status);
            bzero((void*)(usr_monitor_info.iccid),ECM_AUTOCFG_ICCID_LEN);
            pthread_mutex_unlock(&usr_monitor_mux);

            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
            *error = E_ECM_CPIN_Q_SIM_PUK;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_off();
            }
            #endif

            /*add liwei for ali project signal led operation*/
            #if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_sig_level_led_func_start)
            {
                ECM_aux_set_siglevel_led_off();
            }
            #endif


        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_sim_failure))
        {
            strcpy(s_sim_status,ECM_AUTOCFG_SIM_FAILURE);
            pthread_mutex_lock(&usr_monitor_mux);
            sprintf(usr_monitor_info.sim_status,"%s",s_sim_status);
            bzero((void*)(usr_monitor_info.iccid),ECM_AUTOCFG_ICCID_LEN);
            pthread_mutex_unlock(&usr_monitor_mux);

            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
            *error = E_ECM_CPIN_Q_SIM_FAILURE;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_off();
            }
            #endif

            /*add liwei for ali project signal led operation*/
            #if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_sig_level_led_func_start)
            {
                ECM_aux_set_siglevel_led_off();
            }
            #endif


        }
        else
        {
            strcpy(s_sim_status,ECM_AUTOCFG_SIM_ERROR);
            pthread_mutex_lock(&usr_monitor_mux);
            sprintf(usr_monitor_info.sim_status,"%s",s_sim_status);
            bzero((void*)(usr_monitor_info.iccid),ECM_AUTOCFG_ICCID_LEN);
            pthread_mutex_unlock(&usr_monitor_mux);

            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_CPIN_QRECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_off();
            }
            #endif

            /*add liwei for ali project signal led operation*/
            #if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_sig_level_led_func_start)
            {
                ECM_aux_set_siglevel_led_off();
            }
            #endif

        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_CPIN_QUERY_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_CPIN_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_CPIN_QUERY_TERM;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_CPIN_QOTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}


ECM_SM_RSLT_T ECM_query_tty_zband(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*        cmd_str="at+zband?\r\n";//at+zband=all,all,all,all
    const char*        cmd_str_rsp="OK";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zband?";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_ZBAND_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {

        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ( (NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZBAND"))
            && (NULL==strstr(itc_tty_at_buff,"+zband")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_ZBAND_QRECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_off();
            }
            #endif

            /*add liwei for ali project signal led operation*/
            #if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_sig_level_led_func_start)
            {
                ECM_aux_set_siglevel_led_off();
            }
            #endif

        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZBAND_QUERY_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZBAND_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZBAND_QUERY_TERM;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZBAND_QOTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}

ECM_SM_RSLT_T ECM_query_tty_csq(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*        cmd_str="at+csq\r\n";
    const char*        cmd_str_rsp="OK";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+csq";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_CSQ_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(   echo_at);

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+CSQ"))
            && (NULL==strstr(itc_tty_at_buff,"+csq")))            
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {

            ECM_auto_save_csq_strength();
            /*add liwei for ali project signal led operation*/
            #if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_sig_level_led_func_start)
            {
                unsigned int var_rf_recv_strength=0;
                /*get signal strength level */
                pthread_mutex_lock(&usr_monitor_mux);
                var_rf_recv_strength = usr_monitor_info.rf_recv_strength;
                pthread_mutex_unlock(&usr_monitor_mux);

                if ((0 <= var_rf_recv_strength) && (var_rf_recv_strength < 10)) {
                    ECM_aux_set_siglevel_led_low();
                } else if ((10 <= var_rf_recv_strength) && (var_rf_recv_strength < 20)) {
                    ECM_aux_set_siglevel_led_middle();
                } else if ((20 <= var_rf_recv_strength)&&(var_rf_recv_strength <= 31)) {
                    ECM_aux_set_siglevel_led_high();
                } else {
                    ECM_aux_set_siglevel_led_low();
                }
            }
            #endif

            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_CSQ_QRECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_off();
            }
            #endif

            /*add liwei for ali project signal led operation*/
            #if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_sig_level_led_func_start)
            {
                ECM_aux_set_siglevel_led_off();
            }
            #endif

        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_CSQ_QUERY_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_CSQ_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_CSQ_QUERY_TERM;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_CSQ_QOTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}


ECM_SM_RSLT_T ECM_query_tty_creg(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*        cmd_str="at+creg?\r\n";
    const char*        reg_str_rsp1="+CREG: 0,1";
    const char*        reg_str_rsp2="+CREG: 1,1";
    const char*        reg_str_rsp3="+CREG: 2,1";
    const char*        reg_str_rsp4="+CREG: 3,1";
    const char*        reg_str_rsp5="+CREG: 0,5";
    const char*        reg_str_rsp6="+CREG: 1,5";
    const char*        reg_str_rsp7="+CREG: 2,5";
    const char*        reg_str_rsp8="+CREG: 3,5";
    
    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+creg?";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_CREG_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else  if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+CREG"))
            && (NULL==strstr(itc_tty_at_buff,"+creg")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,reg_str_rsp1))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_on();
            }
            #endif

        }
        else if (NULL!=strstr(itc_tty_at_buff,reg_str_rsp2))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_on();
            }
            #endif

        }
        else if (NULL!=strstr(itc_tty_at_buff,reg_str_rsp3))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_on();
            }
            #endif

        }
        else if (NULL!=strstr(itc_tty_at_buff,reg_str_rsp4))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_on();
            }
            #endif

        }
        else if (NULL!=strstr(itc_tty_at_buff,reg_str_rsp5))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_on();
            }
            #endif

        }
        else if (NULL!=strstr(itc_tty_at_buff,reg_str_rsp6))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_on();
            }
            #endif

        }
        else if (NULL!=strstr(itc_tty_at_buff,reg_str_rsp7))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_on();
            }
            #endif

        }
        else if (NULL!=strstr(itc_tty_at_buff,reg_str_rsp8))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_on();
            }
            #endif

        }
        else
        {
            /*register fail, display signal strength 199*/
            pthread_mutex_lock(&usr_monitor_mux);
            usr_monitor_info.net_recv_strength = ECM_DFLT_SIG_STRENGTH ;            
            bzero((void*)(usr_monitor_info.net_type_str),ECM_NET_TYPE_STR_LEN);
            strcpy(usr_monitor_info.net_type_str,s_network_noserver);
            pthread_mutex_unlock(&usr_monitor_mux);

            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_CREG_QRECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_off();
            }
            #endif

            /*add liwei for ali project signal led operation*/
            //#if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            //if (1==ECM_auto_sig_level_led_func_start)
            //{
            //    ECM_aux_set_siglevel_led_off();
            //}
            //#endif

        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_CREG_QUERY_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_CREG_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_CREG_QUERY_TERM;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_CREG_QOTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}



ECM_SM_RSLT_T ECM_send_tty_cgdcont(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt,
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    static char        cmd_str_usr[128] = {0} ;
    const char*        cmd_str="at+cgdcont=1,\"IP\",\"CTNET\"\r\n";
    const char*        cmd_str_T="at+cgdcont=1,\"IP\",\"";
    const char*        cmd_str_E=   "\"\r\n";
    const char*        cmd_str_rsp="OK\r\n";

    static unsigned    int     cmd_retry_cnt = 0 ;

    static char         echo_at[128] = {0} ;

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;

        if (cfg)
        {
            if (1==cfg->para_apn_flag)
            {
                strcpy(cmd_str_usr,cmd_str_T);
                strcat(cmd_str_usr,cfg->para_apn);
                strcat(cmd_str_usr,cmd_str_E);

                strcpy(echo_at,cmd_str_T);
                strcat(echo_at,cfg->para_apn);
                strcat(echo_at,"\"");
            }
            else
            {
                return ECM_SM_EXEC_OK;
                //ecm_sm_rslt = ECM_SM_EXEC_OK;
                //strcpy(cmd_str_usr,cmd_str);
            }
        }
        else
        {
            strcpy(cmd_str_usr,cmd_str);
        }
        
        if (0==ttyusb_send(p_serial,cmd_str_usr,strlen(cmd_str_usr)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str_usr);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_CGDCONT_SEND_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {

        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+CGDCONT"))
            && (NULL==strstr(itc_tty_at_buff,"+cgdcont")))            
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_CGDCONT_RECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_CGDCONT_SEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str_usr,strlen(cmd_str_usr)))
        {
            --cmd_retry_cnt;
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str_usr);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            --cmd_retry_cnt;
            *error = E_ECM_CGDCONT_RESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        
        *error = E_ECM_CGDCONT_TERMINAL;
         ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_CGDCONT_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}




ECM_SM_RSLT_T ECM_send_tty_zecmcallup(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*        cmd_str="at+zecmcall=1\r\n";
    const char*        cmd_str_rsp="OK\r\n";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zecmcall=1";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,9,0);
        }
        else
        {
            *error = E_ECM_ZECMCALL1_SEND_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZECMCALL"))
            && (NULL==strstr(itc_tty_at_buff,"+zecmcall")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_ZECMCALL1_RECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }

    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZECMCALL1_SEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,9,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZECMCALL1_RESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {

        *error = E_ECM_ZECMCALL1_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZECMCALL1_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}


ECM_SM_RSLT_T ECM_send_tty_zecmcalldown(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*        cmd_str="at+zecmcall=0\r\n";
    const char*        cmd_str_rsp="OK\r\n";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zecmcall=0";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else
        {
            *error = E_ECM_ZECMCALL0_SEND_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZECMCALL"))
            && (NULL==strstr(itc_tty_at_buff,"+zecmcall")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_ZECMCALL0_RECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }

    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZECMCALL0_SEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZECMCALL0_RESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZECMCALL0_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZECMCALL0_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}

ECM_SM_RSLT_T ECM_query_tty_zecmcallstatus(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*        cmd_str="at+zecmcall?\r\n";
    const char*        cmd_str_rsp="OK\r\n";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zecmcall?";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else
        {
            *error = E_ECM_ZECMCALL_C_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZECMCALL"))
            && (NULL==strstr(itc_tty_at_buff,"+zecmcall")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            *error = E_ECM_ZECMCALL_C_QRECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }

    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZECMCALL_C_QSEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZECMCALL_C_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZECMCALL_C_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZECMCALL_C_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}





ECM_SM_RSLT_T ECM_query_tty_zpdpcall(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*        cmd_str="at+zpdpcall?\r\n";
    const char*        cmd_str_rsp="+ZPDPCALL: 1";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zpdpcall?";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else
        {
            *error = E_ECM_ZPDPTYPE_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZPDPCALL"))
            && (NULL==strstr(itc_tty_at_buff,"+zpdpcall")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_ZPDPTYPE_QRECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }

    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZPDPTYPE_QSEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZPDPTYPE_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZPDPTYPE_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZPDPTYPE_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}

ECM_SM_RSLT_T ECM_query_tty_cgsn_status(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*        cmd_str="at+cgsn\r\n";
    const char*        cmd_str_rsp="OK\r\n";
    const char*        cmd_str_rsp_error="+CME ERROR:";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+cgsn";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else
        {
            *error = E_ECM_CGSN_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {

        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+CGSN"))
            && (NULL==strstr(itc_tty_at_buff,"+cgsn"))&& (NULL==strstr(itc_tty_at_buff,"+CME ERROR:")))            
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ECM_auto_save_imei();
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp_error))
        {            
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_CGSN_QRECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }

    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_CGSN_QSEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_CGSN_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_CGSN_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_CGSN_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}



ECM_SM_RSLT_T ECM_query_tty_zpas_status(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*                cmd_str="at+zpas?\r\n";
    const char*                cmd_str_rsp="OK\r\n";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zpas?";

    ECM_SM_RSLT_T              ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    char                        var_tmp_str[ECM_NET_TYPE_STR_LEN] = {0} ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else
        {
            *error = E_ECM_ZPAS_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZPAS"))
            && (NULL==strstr(itc_tty_at_buff,"+zpas")))            
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if ((NULL!=strstr(itc_tty_at_buff,cmd_str_rsp)) && ((NULL!=strstr(itc_tty_at_buff,"+ZPAS"))
            ||(NULL!=strstr(itc_tty_at_buff,"+zpas"))))
        {
            if (NULL!=strstr(itc_tty_at_buff,s_network_noserver))
            {
                strcpy(var_tmp_str,s_network_noserver) ;
                s_net_type_val = ECM_NET_NO_SERVICE;
                *error = E_ECM_ZPAS_QNO_SERVICE;
                ecm_sm_rslt = ECM_SM_EXEC_FAIL;
                ITC_clear_timer(&ECM_call_timer);

                /*set network signal strength to default*/
                s_net_signal_strength = ECM_DFLT_SIG_STRENGTH ;
                pthread_mutex_lock(&usr_monitor_mux);
                usr_monitor_info.net_recv_strength = s_net_signal_strength ;
                pthread_mutex_unlock(&usr_monitor_mux);                
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_limited_server))
            {
                strcpy(var_tmp_str,s_network_limited_server) ;
                s_net_type_val = ECM_NET_LIMIT_SERVICE;
                *error = E_ECM_ZPAS_QLIMIT_SERVICE;
                ecm_sm_rslt = ECM_SM_EXEC_FAIL;
                ITC_clear_timer(&ECM_call_timer);

                /*set network signal strength to default*/
                s_net_signal_strength = ECM_DFLT_SIG_STRENGTH ;
                pthread_mutex_lock(&usr_monitor_mux);
                usr_monitor_info.net_recv_strength = s_net_signal_strength ;
                pthread_mutex_unlock(&usr_monitor_mux);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_gsm))
            {
                strcpy(var_tmp_str,s_network_gsm) ;
                s_net_type_val = ECM_NET_GSM;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_gprs))
            {
                strcpy(var_tmp_str,s_network_gprs) ;
                s_net_type_val = ECM_NET_GPRS;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_cdma))
            {
                strcpy(var_tmp_str,s_network_cdma) ;
                s_net_type_val = ECM_NET_CDMA;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_evdo))
            {
                strcpy(var_tmp_str,s_network_evdo) ;
                s_net_type_val = ECM_NET_EVDO;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_ehrpd))
            {
                strcpy(var_tmp_str,s_network_ehrpd) ;
                s_net_type_val = ECM_NET_EHRPD;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_umts))
            {
                strcpy(var_tmp_str,s_network_umts) ;
                s_net_type_val = ECM_NET_UMTS;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_hsdpa))
            {
                strcpy(var_tmp_str,s_network_hsdpa) ;
                s_net_type_val = ECM_NET_HSDPA;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_hsupa))
            {
                strcpy(var_tmp_str,s_network_hsupa) ;
                s_net_type_val = ECM_NET_HSUPA;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_hspa))
            {
                strcpy(var_tmp_str,s_network_hspa) ;
                s_net_type_val = ECM_NET_HSPA;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_hspa_plus))
            {
                strcpy(var_tmp_str,s_network_hspa_plus) ;
                s_net_type_val = ECM_NET_HSPA_P;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_lte))
            {
                strcpy(var_tmp_str,s_network_lte) ;
                s_net_type_val = ECM_NET_LTE;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else if (NULL!=strstr(itc_tty_at_buff,s_network_tdscdma))
            {
                strcpy(var_tmp_str,s_network_tdscdma) ;
                s_net_type_val = ECM_NET_TDSCDMA;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }
            else
            {   //s_network_other
                strcpy(var_tmp_str,s_network_other) ;
                s_net_type_val = ECM_NET_OTHER;
                ecm_sm_rslt = ECM_SM_EXEC_OK;
                ITC_clear_timer(&ECM_call_timer);
            }

            pthread_mutex_lock(&usr_monitor_mux);
            memcpy((void*)(usr_monitor_info.net_type_str),(void*)var_tmp_str, ECM_NET_TYPE_STR_LEN-1);
            pthread_mutex_unlock(&usr_monitor_mux);
        
        }
        else
        {
            ecm_sm_rslt = ECM_SM_EXEC_OK;
            //ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }

    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZPAS_QSEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZPAS_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZPAS_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZPAS_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}



ECM_SM_RSLT_T ECM_query_tty_zgeticcid_status(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*        cmd_str="at+zgeticcid\r\n";
    const char*        cmd_str_rsp="OK\r\n";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zgeticcid";
    
    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else
        {
            *error = E_ECM_ZGETICCID_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZGETICCID"))
            && (NULL==strstr(itc_tty_at_buff,"+zgeticcid")))            
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ECM_auto_save_iccid();
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_ZGETICCID_QRECV_ISSUE;
            //ecm_sm_rslt = ECM_SM_EXEC_OK;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;

            /*add liwei for ali project led operation*/
            #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
            if (1==ECM_auto_led_function_start)
            {
                ECM_aux_led_off();
            }
            #endif

            /*add liwei for ali project signal led operation*/
            //#if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
            //if (1==ECM_auto_sig_level_led_func_start)
            //{
            //    ECM_aux_set_siglevel_led_off();
            //}
            //#endif


        }

    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZGETICCID_QSEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZGETICCID_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZGETICCID_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZGETICCID_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}



ECM_SM_RSLT_T ECM_send_tty_reboot(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*     cmd_str="at+cfun=1,1\r\n";
    const char*     cmd_str_rsp="OK\r\n";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+cfun=1,1";

    ECM_SM_RSLT_T           ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_CFUN_1_1_SEND_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+CFUN"))
            && (NULL==strstr(itc_tty_at_buff,"+cfun")))            
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            *error = E_ECM_CFUN_1_1_RECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_CFUN_1_1_SEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_CFUN_1_1_RESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_CFUN_1_1_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_CFUN_1_1_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}

ECM_SM_RSLT_T ECM_send_tty_zsdt(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    static char     cmd_str[24] = {0};
    const char*     cmd_str_rsp="OK\r\n"; /*if online*/

    static unsigned    int      cmd_retry_cnt = 0 ;

    static   char  echo_at[24]     = {0};

    ECM_SM_RSLT_T               ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        sprintf(cmd_str,"at+zsdt=%01d,%01d,%01d\r\n",1,
            ECM_CALL_HOST_PLUG_POLARITY,ECM_CALL_HOST_PLUG_PULL);

        sprintf(echo_at,"at+zsdt=%01d,%01d,%01d",1,
            ECM_CALL_HOST_PLUG_POLARITY,ECM_CALL_HOST_PLUG_PULL);
        
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_ZSDT_SEND_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZSDT"))
            && (NULL==strstr(itc_tty_at_buff,"+zsdt")))            
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else   if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            *error = E_ECM_ZSDT_RECV_ISSUE;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZSDT_SEND_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZSDT_RESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZSDT_OTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;        
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZSDT_TERMINAL;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}


ECM_SM_RSLT_T ECM_query_tty_zsdt(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*     cmd_str="at+zsdt?\r\n";
    const char*     cmd_str_rsp="OK\r\n";

    static unsigned    int      cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zsdt?";

    ECM_SM_RSLT_T                ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else
        {
            *error = E_ECM_ZSDT_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZSDT"))
            && (NULL==strstr(itc_tty_at_buff,"+zsdt")))            
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if (NULL!=strstr(itc_tty_at_buff,cmd_str_rsp))
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            //*error = E_ECM_ZSDT_QRECV_ISSUE;
            //ecm_sm_rslt = ECM_SM_EXEC_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZSDT_QUERY_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,2,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZSDT_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZSDT_QUERY_TERM;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;        
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZSDT_QOTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}


/*network signal strength */
ECM_SM_RSLT_T ECM_query_tty_zcds(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    const char*                cmd_str="at+zcds?\r\n";
    const char*                cmd_str_rsp="OK\r\n";

    static unsigned    int     cmd_retry_cnt = 0 ;

    const char*     echo_at     = "at+zcds?";

    ECM_SM_RSLT_T              ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    char                       var_tmp_str[ECM_NET_TYPE_STR_LEN] = {0} ;

    *error = 0 ;

    if (ECM_SM_ENTRY_EVT==evt)
    {
        cmd_retry_cnt = 3 ;
        if (0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            ECM_log(ECM_LOG_L_3,"[info] send at_cmd:%s",cmd_str);
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else
        {
            *error = E_ECM_ZCDS_QUERY_FAIL;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {

        ECM_delete_echo_at(    echo_at );

        if (1 != ECM_is_at_with_content())
        {
            /*ignore echo at */
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;
        }
        else if ((NULL!=strstr(itc_tty_at_buff,"+")) && (NULL==strstr(itc_tty_at_buff,"+ZCDS"))
            && (NULL==strstr(itc_tty_at_buff,"+zcds")))
        {
            ecm_sm_rslt = ECM_SM_EXEC_KEEP ;//ignore unsolicited request
        }
        else  if ((NULL!=strstr(itc_tty_at_buff,cmd_str_rsp)) && ((NULL!=strstr(itc_tty_at_buff,"+ZCDS:"))
            ||(NULL!=strstr(itc_tty_at_buff,"+zcds:"))))
        {
            if (ECM_NET_NO_SERVICE==s_net_type_val)
            {
                s_net_signal_strength = ECM_DFLT_SIG_STRENGTH ;
            }
            else if (ECM_NET_LIMIT_SERVICE==s_net_type_val)
            {
                s_net_signal_strength = ECM_DFLT_SIG_STRENGTH ;
            }
            else if (ECM_NET_GSM==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(5);
            }
            else if (ECM_NET_GPRS==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(5);
            }
            else if (ECM_NET_CDMA==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(6);
            }
            else if (ECM_NET_EVDO==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(6);
            }
            else if (ECM_NET_EHRPD==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(6);
            }
            else if (ECM_NET_UMTS==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(5);
            }
            else if (ECM_NET_HSDPA==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(5);
            }
            else if (ECM_NET_HSUPA==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(5);
            }
            else if (ECM_NET_HSPA==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(5);
            }
            else if (ECM_NET_HSPA_P==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(5);
            }
            else if (ECM_NET_LTE==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(5);
            }
            else if (ECM_NET_TDSCDMA==s_net_type_val)
            {
                s_net_signal_strength = ECM_auto_save_signal_strength(5);
            }
            else if (ECM_NET_OTHER==s_net_type_val)
            {
                s_net_signal_strength = ECM_DFLT_SIG_STRENGTH ;
            }
            else
            {
                s_net_signal_strength = ECM_DFLT_SIG_STRENGTH ;
            }
            
            pthread_mutex_lock(&usr_monitor_mux);
            usr_monitor_info.net_recv_strength = s_net_signal_strength ;
            pthread_mutex_unlock(&usr_monitor_mux);

            ITC_clear_timer(&ECM_call_timer);

            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }
        else
        {
            ITC_clear_timer(&ECM_call_timer);
            ecm_sm_rslt = ECM_SM_EXEC_OK;
        }

    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        if(0==cmd_retry_cnt)
        {
            *error = E_ECM_ZCDS_QUERY_MAX;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
        else if(0==ttyusb_send(p_serial,cmd_str,strlen(cmd_str)))
        {
            --cmd_retry_cnt;
            ITC_set_timer(&ECM_call_timer,3,0);
        }
        else 
        {
            --cmd_retry_cnt;
            *error = E_ECM_ZCDS_QRESEND_ERR;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_ZCDS_QUERY_TERM;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_ZCDS_QOTHER_ERR;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}




ECM_SM_RSLT_T ECM_personalization_at(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg, unsigned int* error, void* in, void* out)
{   
    ECM_SM_RSLT_T    ecm_sm_rslt = ECM_SM_EXEC_KEEP ;

    *error = 0 ;
    if (ECM_SM_ENTRY_EVT==evt)
    {
        if (0==ttyusb_send(p_serial, ECM_personal_at, strlen(ECM_personal_at)))
        {
            ECM_log(ECM_LOG_L_1, "[info] send at_cmd:%s", ECM_personal_at);
            ITC_set_timer(&ECM_call_timer,5,0);
        }
        else
        {
            *error = E_ECM_PERSON_AT_SEND_FAIL ;
            ecm_sm_rslt = ECM_SM_EXEC_FAIL;
        }
    }
    else if (ECM_SM_QUERY_EVT==evt)
    {

    }
    else if (ECM_SM_AT_RSP_EVT==evt)
    {
        ECM_log(ECM_LOG_L_1,"[info] recv %s",itc_tty_at_buff);
        
        ITC_clear_timer(&ECM_call_timer);
        ecm_sm_rslt = ECM_SM_EXEC_OK;
    }
    else if (ECM_SM_TIMEOUT_EVT==evt)
    {
        *error = E_ECM_PERSON_AT_RECV_ISSUE ;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }
    else if (ECM_SM_KILL_EVT==evt)
    {
        *error = E_ECM_PERSON_AT_TERMINAL ;
        ecm_sm_rslt = ECM_SM_EXEC_TERM;
    }
    else if (ECM_SM_EXIT_EVT==evt)
    {
        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else
    {
        *error = E_ECM_PERSON_AT_OTHER_ERR ;
        ecm_sm_rslt = ECM_SM_EXEC_FAIL;
    }

    return ecm_sm_rslt ;
}


ECM_SM_RSLT_T ECM_terminal_state(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    *error = 0 ;
    ttyusb_cancel_thxd(p_serial);

    return ECM_SM_EXEC_SUCC ;
}

ECM_SM_RSLT_T ECM_idle_state(ttyusb_dev_t* p_serial, ECM_SM_EVENT_T evt, 
    user_cfg_item_t* cfg,unsigned int* error, void* in, void* out)
{
    *error = 0 ;
    return ECM_SM_EXEC_SUCC ;
}


/*call up flow*/
ecm_sm_t ECM_callup_flow[] = {
    ECM_query_tty_ati,
    ECM_query_tty_zswitch,
    ECM_query_tty_zadset,
    ECM_query_tty_cpin,
    ECM_query_tty_cfun,
    ECM_query_tty_zband,
    ECM_query_tty_csq,
    ECM_query_tty_creg,
    ECM_send_tty_cgdcont,
    ECM_query_tty_zsdt,
    ECM_send_tty_zecmcallup,
    ECM_terminal_state,
};

ecm_sm_t ECM_calldown_flow[] = {
    ECM_query_tty_ati,
    ECM_query_tty_zswitch,
    ECM_query_tty_zadset,
    ECM_query_tty_cpin,
    ECM_query_tty_cfun,
    ECM_query_tty_zband,
    ECM_query_tty_csq,
    ECM_query_tty_creg,
    ECM_send_tty_zecmcalldown,
    ECM_terminal_state,
};

ecm_sm_t ECM_config_flow[] = {
    ECM_query_tty_ati,
    ECM_send_tty_zswitch,
    ECM_send_tty_zadset,
    ECM_send_tty_reboot,
    ECM_terminal_state,
};

ecm_sm_t ECM_query_flow[] = {
    ECM_query_tty_ati,
    ECM_query_tty_zecmcallstatus,
    ECM_terminal_state,
};

ecm_sm_t ECM_cfg_hostplug_sim_flow[] = {
    ECM_query_tty_ati,
    ECM_send_tty_zsdt,/*configure host-plug sim*/
    ECM_send_tty_reboot,
    ECM_terminal_state,
};



//..s_sim_status

ecm_sm_t ECM_query_sim_flow[] = {
    ECM_query_tty_cpin,
    ECM_terminal_state,
};



ecm_sm_t ECM_personalization_flow[] = {
    ECM_personalization_at,
    ECM_terminal_state,
};


unsigned int ECM_exec_at_flow(ttyusb_dev_t* p_serial, ecm_sm_t* flow,user_cfg_item_t* cfg)
{
    unsigned int at_err_id = 0 ;
    unsigned int error = 0 ;
    unsigned int pflow = 0 ;
    ECM_SM_RSLT_T rflow = ECM_SM_EXEC_KEEP;


    do {
        if ((!flow) || (!p_serial))
        {
            error = E_ECM_EXEC_AT_FLOW_PARA_NULL;
            break;
        }

        ITC_signal_init();
        
        rflow = (flow[pflow])(p_serial,ECM_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);

        if (ECM_SM_EXEC_FAIL==rflow)
        {
            error = at_err_id;
            break;
        }

        itc_msg_timer      = 0 ;
        itc_msg_atcmd      = 0 ;
        itc_msg_terminal = 0 ;

        while(1)
        {

            pthread_mutex_lock(&itc_msg_mux);

            while ((0==itc_msg_timer)&&(0==itc_msg_atcmd)&&(0==itc_msg_terminal))
            {                    
                    pthread_cond_wait(&itc_msg_cond, &itc_msg_mux);
            }
            if (1==itc_msg_timer)
            {
                itc_msg_timer =   0;
                rflow = (flow[pflow])(p_serial,ECM_SM_TIMEOUT_EVT,cfg,&at_err_id,NULL,NULL);
            }
            if (1==itc_msg_atcmd)
            {
                itc_msg_atcmd = 0;                
                rflow = (flow[pflow])(p_serial,ECM_SM_AT_RSP_EVT,cfg,&at_err_id,NULL,NULL);
            }
            if (1==itc_msg_terminal)
            {                
                itc_msg_terminal = 0;
                rflow = (flow[pflow])(p_serial,ECM_SM_KILL_EVT,cfg,&at_err_id,NULL,NULL);
            }

            while ((ECM_SM_EXEC_OK==rflow) && ((flow[pflow]) != ECM_terminal_state))
            {
                rflow = (flow[pflow])(p_serial,ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
                ++ pflow ;
                rflow = (flow[pflow])(p_serial,ECM_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);
            }

            if (ECM_SM_EXEC_FAIL==rflow)
            {
                if ((flow[pflow]) != ECM_terminal_state)//need kill thread
                {
                    ttyusb_cancel_thxd(p_serial);
                }
                error = at_err_id;
                break ;
            }
            else if (ECM_SM_EXEC_TERM==rflow)
            {
                if ((flow[pflow]) != ECM_terminal_state)//need kill thread
                {
                    ttyusb_cancel_thxd(p_serial);
                }
                error = at_err_id;
                break ;
            }
            else if (ECM_SM_EXEC_SUCC==rflow)
            {
                /* exec successfully */
                error = 0;
                break ;
            }
            else 
            {                
            }

            pthread_mutex_unlock(&itc_msg_mux);
        }

       
    }while(0);

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_3,"[info] ECM_exec_at_flow error_code(%d)",error);
    }

    return error ;
}


unsigned int ECM_call(char* tty_path, ECM_USR_OPS_T op, char* v4v6para, char* apn)
{
    unsigned int      error = 0;
    ttyusb_dev_t      serial_demo;
    user_cfg_item_t usr_cfg;

#if (ECM_CALL_AUTO_PORT==ECM_DEMO_ON)
    char              ECM_auto_port_mdm_path[256] = {0};
#endif

    do {
        ttyusb_init(&serial_demo);

        ECM_usr_cfg_init(&usr_cfg);

        ECM_usr_cfg_set(&usr_cfg,v4v6para,apn);

        if (NULL==tty_path)
        {
#if (ECM_CALL_AUTO_PORT==ECM_DEMO_ON)
            if (0 != ttyusb_search_port(1,ECM_auto_port_mdm_path,256))
            {
                error=E_ECM_PORT_SEARCH_FAIL ;
                break;
            }
            else
            {
                ECM_auto_port_mdm_path[255]='\0';
                error = ttyusb_config(&serial_demo,
                    ECM_auto_port_mdm_path,strlen(ECM_auto_port_mdm_path));
            }
#else
            error = ttyusb_config(&serial_demo,tty_path,0);
#endif
        }
        else
        {
            error = ttyusb_config(&serial_demo,tty_path, strlen(tty_path));
        }

        if (0 != error)
        {
            break;
        }

        error = ttyusb_open(&serial_demo);

        if (0 != error)
        {
            break;
        }

        error = ttyusb_reg_cb(&serial_demo,ITC_send_atcmd_msg);

        if (0 != error)
        {
            (void)ttyusb_close(&serial_demo);
            break;
        }

        error = ttyusb_start_recv(&serial_demo);

        if (0 != error)
        {
            (void)ttyusb_close(&serial_demo);
            break;
        }

        switch(op)
        {
            case ECM_OP_UP:
                error = ECM_exec_at_flow(&serial_demo,ECM_callup_flow,&usr_cfg);
                break;
            case ECM_OP_DOWN:
                error = ECM_exec_at_flow(&serial_demo,ECM_calldown_flow,&usr_cfg);
                break;
            case ECM_OP_CONFIG:
                error = ECM_exec_at_flow(&serial_demo,ECM_config_flow,&usr_cfg);
                break;
            case ECM_OP_QSTATUS:
                error = ECM_exec_at_flow(&serial_demo,ECM_query_flow,&usr_cfg);
                break;
            case ECM_OP_CFG_HOT_SIM:
                error = ECM_exec_at_flow(&serial_demo,ECM_cfg_hostplug_sim_flow,&usr_cfg);
                break;
            case ECM_OP_Q_SIM_STATUS:
                error = ECM_exec_at_flow(&serial_demo,ECM_query_sim_flow,&usr_cfg);
                break;
            case ECM_OP_PERSON_AT:
                error = ECM_exec_at_flow(&serial_demo,ECM_personalization_flow,&usr_cfg);                
                break;
            default:
                error = ECM_exec_at_flow(&serial_demo,ECM_query_flow,&usr_cfg);
                break;
        }
       

        if (0 != error)
        {
            (void)ttyusb_close(&serial_demo);
            break;
        }

        (void)ttyusb_close(&serial_demo);

    } while(0);

    if (op==ECM_OP_Q_SIM_STATUS)
    {
        ECM_log(ECM_LOG_L_1,"[RET_SIM_STATUS] %s",s_sim_status);
    }
    else
    {
        if (0!=error)
        {
            ECM_log(ECM_LOG_L_1,"[ERROR CODE] ECM_call err%04d",error);
        }
    }

    return error ;

}


unsigned int ECM_call_ext(int fd, ECM_USR_OPS_T op, char* v4v6para, char* apn)
{
    unsigned int error=0;
    ttyusb_dev_t    serial_demo;
    user_cfg_item_t usr_cfg;

    do {
        ttyusb_init(&serial_demo);
        
        ECM_usr_cfg_init(&usr_cfg);

        ECM_usr_cfg_set(&usr_cfg,v4v6para,apn);

        if (fd < 0)
        {
            error = E_ECM_CALL_START_EXT_FD_ERR;
            break;
        }

        serial_demo.tty_type = TTY_O_USR_SPC_FD ;

        error = ttyusb_reg_cb(&serial_demo,ITC_send_atcmd_msg);

        if (0 != error)
        {
            (void)ttyusb_close(&serial_demo);
            break;
        }

        error = ttyusb_start_recv(&serial_demo);

        if (0 != error)
        {
            (void)ttyusb_close(&serial_demo);
            break;
        }
        switch(op)
        {
            case ECM_OP_UP:
                error = ECM_exec_at_flow(&serial_demo,ECM_callup_flow,&usr_cfg);
                break;
            case ECM_OP_DOWN:
                error = ECM_exec_at_flow(&serial_demo,ECM_calldown_flow,&usr_cfg);
                break;
            case ECM_OP_CONFIG:
                error = ECM_exec_at_flow(&serial_demo,ECM_config_flow,&usr_cfg);
                break;
            case ECM_OP_QSTATUS:
                error = ECM_exec_at_flow(&serial_demo,ECM_query_flow,&usr_cfg);
                break;
            case ECM_OP_CFG_HOT_SIM:
                error = ECM_exec_at_flow(&serial_demo,ECM_cfg_hostplug_sim_flow,&usr_cfg);
                break;
            default:
                error = ECM_exec_at_flow(&serial_demo,ECM_query_flow,&usr_cfg);
                break;
        }        

        if (0 != error)
        {
            (void)ttyusb_close(&serial_demo);
            break;
        }

        (void)ttyusb_close(&serial_demo);

    } while(0);

    if (0 != error)
    {
        ECM_log(ECM_LOG_L_1, "[ERROR CODE] ECM_call_start_ext err%04d", error);
    }

    return error ;

}


/* add requirement of auto connect flow start */


ecm_sm_t ECM_auto_under_callup_flow[] = {
    ECM_query_tty_ati,
    ECM_query_tty_zswitch,
    ECM_query_tty_zadset,
    ECM_query_tty_zsdt,
    ECM_query_tty_cgsn_status,
    ECM_query_tty_cfun,
    ECM_query_tty_cpin,
    ECM_query_tty_zgeticcid_status,
    ECM_query_tty_zband,
    ECM_query_tty_csq,
    ECM_query_tty_creg,
    ECM_query_tty_zpas_status,
    ECM_query_tty_zcds,
    ECM_send_tty_cgdcont,
    ECM_send_tty_zecmcallup,
    ECM_query_tty_zecmcallstatus,
    ECM_query_tty_zpdpcall,
    ECM_idle_state,
};

ecm_sm_t ECM_auto_under_connect_flow[] = {
    ECM_query_tty_csq,
/*add liwei for fix gswerr id 0000 start */
#if (ECM_CALL_FIX_GSWERR_ID_0000    ==ECM_DEMO_ON)
    ECM_query_tty_gswerr_id_000,
#endif
    /*add liwei for fix gswerr id 0000 end */
    ECM_query_tty_zpdpcall,
    ECM_query_tty_zpas_status,
    ECM_query_tty_zcds,
    ECM_idle_state,
};

ecm_sm_t ECM_auto_under_calldown_flow[] = {
    ECM_send_tty_zecmcalldown,
    ECM_terminal_state,
};


ecm_sm_t ECM_auto_under_reboot_flow[] = {
    /*add liwei for start resolve fd issue start*/
//  ECM_send_tty_zecmcalldown,
    /*add liwei for start resolve fd issue end*/
    ECM_send_tty_reboot,
    ECM_terminal_state,
};


static ECM_AUTO_SM_STATE_T     ECM_auto_callup_state = ECM_AUTO_SM_STATE_UNINIT ;

static ECM_AUTO_SM_STATE_T     ECM_auto_callup_state_last = ECM_AUTO_SM_STATE_UNINIT ;


static unsigned int     pos_auto_callup_substate = 0 ;


void ECM_auto_sm_init(void)
{
    ECM_auto_callup_state = ECM_AUTO_SM_STATE_UNINIT ;   
    ECM_auto_callup_state_last = ECM_AUTO_SM_STATE_UNINIT ;
    pos_auto_callup_substate = 0 ;

    /*add liwei for port error start*/
    ttyusb_get_clear_failed_flg();
    /*add liwei for port error end*/
}

ECM_AUTO_SM_JUMP_T ECM_auto_sm_callup(ttyusb_dev_t* p_serial, ECM_AUTO_SM_EVENT_T evt,     
    user_cfg_item_t* cfg,   unsigned int* error, void* in, void* out)
{
    /*auto sm*/
    ECM_AUTO_SM_JUMP_T sm_rslt = ECM_AUTO_SM_NO_SWITCH;

    /*atflow sm*/
    ECM_SM_RSLT_T rflow = ECM_SM_EXEC_KEEP;
    unsigned int at_err_id = 0 ;

    *error = 0 ;

    if (ECM_AUTO_SM_ENTRY_EVT==evt)
    {
        pos_auto_callup_substate = 0;
        rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup ENTRY");
    }
    else if (ECM_AUTO_SM_QUERY_EVT==evt)
    {
        rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_QUERY_EVT,cfg,&at_err_id,NULL,NULL);
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup QUERY");
    }
    else if (ECM_AUTO_SM_AT_RSP_EVT==evt)
    {
        rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_AT_RSP_EVT,cfg,&at_err_id,NULL,NULL);
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup AT_RSP");
    }
    else if (ECM_AUTO_SM_TIMEOUT_EVT==evt)
    {
        rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_TIMEOUT_EVT,cfg,&at_err_id,NULL,NULL);
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup TIMEOUT");
    }
    else if (ECM_AUTO_SM_KILL_EVT==evt)
    {
        rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_KILL_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_EXIT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup EXIT");
        rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        pos_auto_callup_substate = 0;        
    }
    else if (ECM_AUTO_SM_REQ_QUIT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup QUIT");

        rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        
        ECM_auto_callup_state_last = ECM_auto_callup_state;
        ECM_auto_callup_state = ECM_AUTO_SM_STATE_CALLDOWN;
        sm_rslt = ECM_AUTO_SM_JUMP_CALLDOWN;
    }
    else if (ECM_AUTO_SM_REQ_REBOOT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup REBOOT");
        rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);

        ECM_auto_callup_state_last = ECM_auto_callup_state;
        ECM_auto_callup_state = ECM_AUTO_SM_STATE_REBOOT;
        sm_rslt = ECM_AUTO_SM_JUMP_REBOOT;
    }
    else
    {
        rflow = ECM_SM_EXEC_KEEP;
        sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }


    *error = at_err_id;

    ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup rflow=(%d)",rflow);
    ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup error_code(%d)",at_err_id);


    while( (ECM_SM_EXEC_OK==rflow) && ((ECM_auto_under_callup_flow[pos_auto_callup_substate]) != ECM_idle_state) )
    {
        rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        ++ pos_auto_callup_substate ;
        rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,ECM_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);
    }

    if (ECM_SM_EXEC_FAIL==rflow)
    {
        /* jump to ECM_AUTO_SM_STATE_DISCON */
        if ((ECM_auto_under_callup_flow[pos_auto_callup_substate]) != ECM_idle_state)
        {
             rflow = (ECM_auto_under_callup_flow[pos_auto_callup_substate])(p_serial,ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        }
        ECM_auto_callup_state_last = ECM_auto_callup_state;
        ECM_auto_callup_state = ECM_AUTO_SM_STATE_DISCON;
        sm_rslt = ECM_AUTO_SM_JUMP_DISCON;
        if (0 != at_err_id)
        {
            *error = at_err_id;
        }
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup ECM_SM_EXEC_FAIL err%d",at_err_id);
    }
    else if (ECM_SM_EXEC_TERM==rflow)
    {
        sm_rslt = ECM_AUTO_SM_TERMINAL;
    }
    else if (ECM_SM_EXEC_SUCC==rflow)
    {
        /* jump to ECM_AUTO_SM_STATE_CONNECT */
        ECM_auto_callup_state_last = ECM_auto_callup_state;
        ECM_auto_callup_state = ECM_AUTO_SM_STATE_CONNECT;
        sm_rslt = ECM_AUTO_SM_JUMP_CONNECT;
        *error = 0;
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_callup ECM_SM_EXEC_SUCC");
    }
    else if ( ECM_SM_EXEC_KEEP==rflow )
    {
        //sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }
    else
    {
        //sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }

    return sm_rslt ;

}


ECM_AUTO_SM_JUMP_T ECM_auto_sm_calldown(ttyusb_dev_t* p_serial, ECM_AUTO_SM_EVENT_T evt,     
    user_cfg_item_t* cfg,   unsigned int* error, void* in, void* out)
{
    /*auto sm*/
    ECM_AUTO_SM_JUMP_T sm_rslt = ECM_AUTO_SM_NO_SWITCH;

    /*atflow sm*/
    ECM_SM_RSLT_T rflow = ECM_SM_EXEC_KEEP;
    unsigned int at_err_id = 0 ;

    *error = 0 ;

    if (ECM_AUTO_SM_ENTRY_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_calldown Entry");
        
        pos_auto_callup_substate = 0;
        rflow = (ECM_auto_under_calldown_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_QUERY_EVT==evt)
    {
        rflow = (ECM_auto_under_calldown_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_QUERY_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_AT_RSP_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_calldown AT_RSP");
        rflow = (ECM_auto_under_calldown_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_AT_RSP_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_TIMEOUT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_calldown TIMEOUT");
        rflow = (ECM_auto_under_calldown_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_TIMEOUT_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_KILL_EVT==evt)
    {
        rflow = (ECM_auto_under_calldown_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_KILL_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_EXIT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_calldown Exit");

        rflow = (ECM_auto_under_calldown_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        pos_auto_callup_substate = 0;
        *error = at_err_id;
    }
    else if (ECM_AUTO_SM_REQ_QUIT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_calldown QUIT");
        rflow = ECM_SM_EXEC_KEEP;
        sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }

    else if (ECM_AUTO_SM_REQ_REBOOT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_calldown reboot");
        rflow = ECM_SM_EXEC_KEEP;
        sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }
    else
    {
        rflow = ECM_SM_EXEC_KEEP;
        sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }

    ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_calldown rflow=%d",rflow);

    while( (ECM_SM_EXEC_OK==rflow) && ((ECM_auto_under_calldown_flow[pos_auto_callup_substate]) != ECM_terminal_state) )
    {
        rflow = (ECM_auto_under_calldown_flow[pos_auto_callup_substate])(p_serial,ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        ++ pos_auto_callup_substate ;
        rflow = (ECM_auto_under_calldown_flow[pos_auto_callup_substate])(p_serial,ECM_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);
    }

    if (ECM_SM_EXEC_FAIL==rflow)
    {
        sm_rslt = ECM_AUTO_SM_NO_SWITCH;
        *error = at_err_id;
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_calldown ECM_SM_EXEC_FAIL");
    }
    else if (ECM_SM_EXEC_TERM==rflow)
    {
        sm_rslt = ECM_AUTO_SM_TERMINAL;
    }
    else if (ECM_SM_EXEC_SUCC==rflow)
    {
        sm_rslt = ECM_AUTO_SM_TERMINAL;
        *error = 0;
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_calldown ECM_SM_EXEC_SUCC");
    }
    else if (ECM_SM_EXEC_KEEP==rflow )
    {
        //sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }
    else
    {
        //sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }

    return sm_rslt ;

}


ECM_AUTO_SM_JUMP_T ECM_auto_sm_reboot(ttyusb_dev_t* p_serial, ECM_AUTO_SM_EVENT_T evt,     
    user_cfg_item_t* cfg,   unsigned int* error, void* in, void* out)
{
    /*auto sm*/
    ECM_AUTO_SM_JUMP_T sm_rslt = ECM_AUTO_SM_NO_SWITCH;

    /*atflow sm*/
    ECM_SM_RSLT_T rflow = ECM_SM_EXEC_KEEP;
    unsigned int at_err_id = 0 ;

    *error = 0 ;

    if (ECM_AUTO_SM_ENTRY_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_reboot Entry");
        
        pos_auto_callup_substate = 0;
        rflow = (ECM_auto_under_reboot_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_QUERY_EVT==evt)
    {
        rflow = (ECM_auto_under_reboot_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_QUERY_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_AT_RSP_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_reboot AT_RSP");
        rflow = (ECM_auto_under_reboot_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_AT_RSP_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_TIMEOUT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_reboot TIMEOUT");
        rflow = (ECM_auto_under_reboot_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_TIMEOUT_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_KILL_EVT==evt)
    {
        rflow = (ECM_auto_under_reboot_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_KILL_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_EXIT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_reboot Exit");

        rflow = (ECM_auto_under_reboot_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        pos_auto_callup_substate = 0;
        *error = at_err_id;
    }
    else if (ECM_AUTO_SM_REQ_QUIT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_reboot QUIT");
        rflow = ECM_SM_EXEC_KEEP;
        sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }
    else if (ECM_AUTO_SM_REQ_REBOOT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_reboot REBOOT");
        rflow = ECM_SM_EXEC_KEEP;
        sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }

    else
    {
        sm_rslt = ECM_AUTO_SM_NO_SWITCH;
        rflow = ECM_SM_EXEC_KEEP;
    }

    ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_reboot rflow=%d",rflow);

    while( (ECM_SM_EXEC_OK==rflow) && ((ECM_auto_under_reboot_flow[pos_auto_callup_substate]) != ECM_terminal_state) )
    {
        rflow = (ECM_auto_under_reboot_flow[pos_auto_callup_substate])(p_serial,ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        ++ pos_auto_callup_substate ;
        rflow = (ECM_auto_under_reboot_flow[pos_auto_callup_substate])(p_serial,ECM_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);
    }

    if (ECM_SM_EXEC_FAIL==rflow)
    {

        /*add liwei for start resolve fd issue start*/
        //sm_rslt = ECM_AUTO_SM_NO_SWITCH;
        sm_rslt = ECM_AUTO_SM_TERMINAL;
        pthread_mutex_lock(&usr_monitor_mux);
        bzero((void*)&usr_monitor_info,sizeof(ECM_auto_monitor_t));
        pthread_mutex_unlock(&usr_monitor_mux);
        ttyusb_cancel_thxd(p_serial);
        /*add liwei for start resolve fd issue end*/

        ECM_log(ECM_LOG_L_3,"[info] ECM_auto_sm_reboot ECM_SM_EXEC_FAIL");
        *error = at_err_id;
    }
    else if (ECM_SM_EXEC_TERM==rflow)
    {
        sm_rslt = ECM_AUTO_SM_TERMINAL;
        /*add liwei for start resolve fd issue start*/
        ttyusb_cancel_thxd(p_serial);
        ECM_log(ECM_LOG_L_3,"[info] ECM_auto_sm_reboot ECM_SM_EXEC_TERM");
        /*add liwei for start resolve fd issue end*/
    }
    else if (ECM_SM_EXEC_SUCC==rflow)
    {
        pthread_mutex_lock(&usr_monitor_mux);
        bzero((void*)&usr_monitor_info,sizeof(ECM_auto_monitor_t));
        pthread_mutex_unlock(&usr_monitor_mux);

        sm_rslt = ECM_AUTO_SM_TERMINAL;
        ECM_log(ECM_LOG_L_3,"[info] ECM_auto_sm_reboot ECM_SM_EXEC_SUCC");
        *error = 0;
    }
    else if (ECM_SM_EXEC_KEEP==rflow )
    {
        //sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }
    else
    {
        //sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }

    return sm_rslt ;

}



ECM_AUTO_SM_JUMP_T ECM_auto_sm_under_connect(ttyusb_dev_t* p_serial, ECM_AUTO_SM_EVENT_T evt,     
    user_cfg_item_t* cfg,   unsigned int* error, void* in, void* out)
{
    /*auto sm*/
    ECM_AUTO_SM_JUMP_T sm_rslt = ECM_AUTO_SM_NO_SWITCH;

    /*atflow sm*/
    ECM_SM_RSLT_T rflow = ECM_SM_EXEC_KEEP;
    unsigned int at_err_id = 0 ;


    static unsigned    int     cmd_retry_cnt = 0 ;
    static unsigned int        flag_delay_run_flow = 0 ;


    *error = 0 ;

    if (ECM_AUTO_SM_ENTRY_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_connect Entry");
        /*after 15 seconds we will bootup the flow:ECM_auto_under_connect_flow*/
        ITC_set_timer(&ECM_call_timer,ECM_AUTO_RETRY_INTV,0);
        flag_delay_run_flow = 1 ;
        pos_auto_callup_substate = 0;

        /*add liwei for ali project led operation*/
        #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
        if (1==ECM_auto_led_function_start)
        {
            ECM_aux_led_fls();
        }
        #endif

    }
    else if (ECM_AUTO_SM_QUERY_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_connect Query");
        rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_QUERY_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_AT_RSP_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_connect ATRESP");
        if (0==flag_delay_run_flow)
        {
            rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,
                ECM_SM_AT_RSP_EVT,cfg,&at_err_id,NULL,NULL);
        }
    }
    else if (ECM_AUTO_SM_TIMEOUT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm delay query begin");
        if (1==flag_delay_run_flow)
        {
            flag_delay_run_flow = 0 ;
            rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,
                ECM_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);
        }
        else
        {
            rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,
                ECM_SM_TIMEOUT_EVT,cfg,&at_err_id,NULL,NULL);
        }    
    }
    else if (ECM_AUTO_SM_KILL_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_connect KILL_EVT");
        rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_KILL_EVT,cfg,&at_err_id,NULL,NULL);
    }
    else if (ECM_AUTO_SM_EXIT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_connect Exit");

        rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        pos_auto_callup_substate = 0;

        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);

        /*add liwei for ali project led operation*/
        #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
        if (1==ECM_auto_led_function_start)
        {
            ECM_aux_led_off();
        }
        #endif

    }
    else if (ECM_AUTO_SM_REQ_QUIT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_connect QUIT");

        rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);

        ECM_auto_callup_state_last = ECM_auto_callup_state;
        ECM_auto_callup_state = ECM_AUTO_SM_STATE_CALLDOWN;
        sm_rslt = ECM_AUTO_SM_JUMP_CALLDOWN;
        rflow = ECM_SM_EXEC_KEEP;
    }
    else if (ECM_AUTO_SM_REQ_REBOOT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_connect REBOOT");
        rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,
            ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);

        ECM_auto_callup_state_last = ECM_auto_callup_state;
        ECM_auto_callup_state = ECM_AUTO_SM_STATE_REBOOT;        
        sm_rslt = ECM_AUTO_SM_JUMP_REBOOT;
    }
    else
    {
        rflow = ECM_SM_EXEC_KEEP;
        sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }


    *error = at_err_id;

    ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_connect rflow=%d",rflow);
    ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_connect error_code(%d)",at_err_id);

    while( (ECM_SM_EXEC_OK==rflow) && ((ECM_auto_under_connect_flow[pos_auto_callup_substate]) != ECM_idle_state) )
    {
        rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        ++ pos_auto_callup_substate ;
        rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,ECM_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);
    }

    if (ECM_SM_EXEC_FAIL==rflow)
    {
        /* disconnect, then need to jump to ECM_AUTO_SM_STATE_DISCON */
        if ((ECM_auto_under_connect_flow[pos_auto_callup_substate]) != ECM_idle_state)
        {
             rflow = (ECM_auto_under_connect_flow[pos_auto_callup_substate])(p_serial,ECM_SM_EXIT_EVT,cfg,&at_err_id,NULL,NULL);
        }
        ECM_auto_callup_state_last = ECM_auto_callup_state;
        ECM_auto_callup_state = ECM_AUTO_SM_STATE_DISCON;
        sm_rslt = ECM_AUTO_SM_JUMP_DISCON;
        if (0!=at_err_id)
        {
            *error = at_err_id;
        }
        ECM_log(ECM_LOG_L_3,"[WARNING] CONNECT FAIL, Try to connect againg...........");
    }
    else if (ECM_SM_EXEC_TERM==rflow)
    {
        sm_rslt = ECM_AUTO_SM_TERMINAL;
    }
    else if (ECM_SM_EXEC_SUCC==rflow)
    {
        /* no need to switch state */
        sm_rslt = ECM_AUTO_SM_NO_SWITCH;
        *error = 0;

        ITC_set_timer(&ECM_call_timer,ECM_AUTO_RETRY_INTV,0);  /*re-query network status atfter next 30 seconds*/
        flag_delay_run_flow = 1 ;
        pos_auto_callup_substate = 0;

        ECM_log(ECM_LOG_L_3,"[INFO] Under CONNECT Status..............");
    }
    else if (ECM_SM_EXEC_KEEP==rflow )
    {
        //sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }
    else
    {
        //sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    }

    return sm_rslt ;

}


ECM_AUTO_SM_JUMP_T ECM_auto_sm_under_disconn(ttyusb_dev_t* p_serial, ECM_AUTO_SM_EVENT_T evt,
    user_cfg_item_t* cfg,   unsigned int* error, void* in, void* out)
{
    ECM_AUTO_SM_JUMP_T sm_rslt = ECM_AUTO_SM_NO_SWITCH;
    static unsigned    int     cmd_retry_cnt = 0 ;
    static unsigned int        flag_delay_run_flow = 0 ;

    *error = 0 ;

    if (ECM_AUTO_SM_ENTRY_EVT==evt)
    {

#if (ECM_CALL_AUTO_PORT==ECM_DEMO_ON)

        if (0 != ECM_auto_check_port(p_serial))
        {
            pos_auto_callup_substate = 0;
            ITC_clear_timer(&ECM_call_timer);
            ttyusb_cancel_thxd(p_serial);

            /*add liwei for fix bugs start*/
            flag_delay_run_flow = 0 ;
            /*add liwei for fix bugs end*/
            
            sm_rslt = ECM_AUTO_SM_TERMINAL;
            ECM_log(ECM_LOG_L_3,"[INFO] Set ECM_AUTO_SM_TERMINAL01...........");
        }
        else if (0 != ttyusb_get_thxd_failed_flg())
        {
            /*add liwei for port error start*/
            pos_auto_callup_substate = 0;
            ITC_clear_timer(&ECM_call_timer);
            ttyusb_cancel_thxd(p_serial);
            
            /*add liwei for fix bugs start*/
            flag_delay_run_flow = 0 ;
            /*add liwei for fix bugs end*/
            ECM_log(ECM_LOG_L_4,"[info] Port Error,Will reboot~");

            sm_rslt = ECM_AUTO_SM_TERMINAL;
            ECM_log(ECM_LOG_L_3,"[INFO] Set ECM_AUTO_SM_TERMINAL02...........");
            /*add liwei for port error end*/
        }
        else
        {
            /*after 30 seconds we will bootup the flow:ECM_AUTO_SM_STATE_CALLUP*/
            ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_disconn Entry");
            
            ITC_set_timer(&ECM_call_timer,ECM_AUTO_RETRY_INTV,0);
            flag_delay_run_flow = 1 ;
            pos_auto_callup_substate = 0;
        }
#else
        /*after 30 seconds we will bootup the flow:ECM_auto_under_connect_flow*/
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_disconn Entry");

        ITC_set_timer(&ECM_call_timer,ECM_AUTO_RETRY_INTV,0);
        flag_delay_run_flow = 1 ;
        pos_auto_callup_substate = 0;

#endif   

    }
    else if (ECM_AUTO_SM_TIMEOUT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_disconn TIMEOUT");
        if (1==flag_delay_run_flow)
        {
            flag_delay_run_flow = 0 ;
            /* disconnect, then need to jump to ECM_AUTO_SM_STATE_DISCON */
            ECM_auto_callup_state_last = ECM_auto_callup_state;
            ECM_auto_callup_state = ECM_AUTO_SM_STATE_CALLUP;
            sm_rslt = ECM_AUTO_SM_JUMP_CALLUP;
            ITC_clear_timer(&ECM_call_timer);
        }
        /*add liwei for fix bugs start*/
        else
        {
            ECM_log(ECM_LOG_L_3,"[info] ECM_auto_sm_under_disconn Error");
        }
        /*add liwei for fix bugs end*/

    }
    else if (ECM_AUTO_SM_EXIT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_disconn Exit");

        ITC_clear_timer(&ECM_call_timer);
        usleep(ECM_AT_FLOW_INTV);
    }
    else if (ECM_AUTO_SM_REQ_QUIT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_disconn QUIT");
        ECM_auto_callup_state_last = ECM_auto_callup_state;
        ECM_auto_callup_state = ECM_AUTO_SM_STATE_CALLDOWN;
        sm_rslt = ECM_AUTO_SM_JUMP_CALLDOWN;
    }
    else if (ECM_AUTO_SM_REQ_REBOOT_EVT==evt)
    {
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm_under_disconn REBOOT");
        ECM_auto_callup_state_last = ECM_auto_callup_state;
        ECM_auto_callup_state = ECM_AUTO_SM_STATE_REBOOT;
        sm_rslt = ECM_AUTO_SM_JUMP_REBOOT;
    }

    return sm_rslt ;

}



ECM_AUTO_SM_JUMP_T ECM_auto_sm    (ttyusb_dev_t* p_serial, ECM_AUTO_SM_EVENT_T evt,     
    user_cfg_item_t* cfg,   unsigned int* error, void* in, void* out)
{
    /*auto sm*/
    ECM_AUTO_SM_JUMP_T sm_rslt = ECM_AUTO_SM_NO_SWITCH;

    if (ECM_AUTO_SM_STATE_UNINIT==ECM_auto_callup_state)
    {
        ECM_auto_callup_state_last= ECM_AUTO_SM_STATE_UNINIT;
        ECM_auto_callup_state = ECM_AUTO_SM_STATE_CALLUP;
        sm_rslt = ECM_AUTO_SM_JUMP_CALLUP;
    }
    else if (ECM_AUTO_SM_STATE_CALLUP==ECM_auto_callup_state)    
    {
        sm_rslt = ECM_auto_sm_callup(p_serial,evt,cfg,error,in,out);
    }
    else if (ECM_AUTO_SM_STATE_DISCON==ECM_auto_callup_state)    
    {
        sm_rslt = ECM_auto_sm_under_disconn(p_serial,evt,cfg,error,in,out);
    }
    else if (ECM_AUTO_SM_STATE_CONNECT==ECM_auto_callup_state)
    {
        sm_rslt = ECM_auto_sm_under_connect(p_serial,evt,cfg,error,in,out);
    }
    else if (ECM_AUTO_SM_STATE_CALLDOWN==ECM_auto_callup_state)    
    {
        sm_rslt = ECM_auto_sm_calldown(p_serial,evt,cfg,error,in,out);
    }
    else if (ECM_AUTO_SM_STATE_REBOOT==ECM_auto_callup_state)
    {
        sm_rslt = ECM_auto_sm_reboot(p_serial,evt,cfg,error,in,out);
    }

    /* record error status */
    if (error)
    {
        if (0 != *error)
        {
            /* lock and write error information */
            pthread_mutex_lock(&usr_monitor_mux);
            usr_monitor_info.error_code = *error ;
            ECM_error_map(usr_monitor_info.error_code,
                                  usr_monitor_info.error_info,
                                       usr_monitor_info.error_ext_info);
            usr_monitor_info.error_info[ECM_AUTOCFG_ERR_INFO_LEN-1]='\0';
            usr_monitor_info.error_ext_info[ECM_AUTOCFG_ERR_INFO_LEN-1]='\0';
            pthread_mutex_unlock(&usr_monitor_mux);

            ECM_log(ECM_LOG_L_3,"[ERROR] ECM_auto_sm prev:error=%d",*error);
        }
    }

    ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm prev ECM_auto_callup_state =%d",ECM_auto_callup_state );
    ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm prev ECM_auto_callup_state_last=%d",ECM_auto_callup_state_last);
    ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm prev sm_rslt =%d",sm_rslt );

    while( (ECM_AUTO_SM_JUMP_CALLUP==sm_rslt) || (ECM_AUTO_SM_JUMP_CONNECT==sm_rslt)
        ||(ECM_AUTO_SM_JUMP_DISCON==sm_rslt) || (ECM_AUTO_SM_JUMP_CALLDOWN==sm_rslt)
        || (ECM_AUTO_SM_JUMP_REBOOT==sm_rslt) )
    {
        if (ECM_AUTO_SM_JUMP_CALLUP==sm_rslt)
        {
            if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_UNINIT) {

            } else  if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CALLUP)        {
                sm_rslt = ECM_auto_sm_callup(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            } else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_DISCON)        {
                sm_rslt = ECM_auto_sm_under_disconn(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            } else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CONNECT)        {
                sm_rslt = ECM_auto_sm_under_connect(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            } else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CALLDOWN)        {
                sm_rslt = ECM_auto_sm_calldown(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            sm_rslt = ECM_auto_sm_callup(p_serial,ECM_AUTO_SM_ENTRY_EVT,cfg,error,in,out);
        }
        else if (ECM_AUTO_SM_JUMP_CONNECT==sm_rslt)
        {
            if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CALLUP)
            {

                /* set to connect and clear retry_times */
                pthread_mutex_lock(&usr_monitor_mux);
                usr_monitor_info.connet_status = 1 ;
                usr_monitor_info.retry_times = 0;
                usr_monitor_info.error_code = 0;
                bzero((void*)(usr_monitor_info.error_info),ECM_AUTOCFG_ERR_INFO_LEN);
                bzero((void*)(usr_monitor_info.error_ext_info),ECM_AUTOCFG_ERR_INFO_LEN);
                pthread_mutex_unlock(&usr_monitor_mux);

                /*set to connect, notify user */
                sm_rslt = ECM_auto_sm_callup(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_DISCON)       
            {

                /* set to connect and clear retry_times */
                pthread_mutex_lock(&usr_monitor_mux);
                usr_monitor_info.connet_status = 1 ;
                usr_monitor_info.retry_times = 0;
                usr_monitor_info.error_code = 0;
                bzero((void*)(usr_monitor_info.error_info),ECM_AUTOCFG_ERR_INFO_LEN);
                bzero((void*)(usr_monitor_info.error_ext_info),ECM_AUTOCFG_ERR_INFO_LEN);
                pthread_mutex_unlock(&usr_monitor_mux);

                sm_rslt = ECM_auto_sm_under_disconn(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CONNECT) 
            {
                sm_rslt = ECM_auto_sm_under_connect(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CALLDOWN)       
            {
                sm_rslt = ECM_auto_sm_calldown(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }

            sm_rslt = ECM_auto_sm_under_connect(p_serial,ECM_AUTO_SM_ENTRY_EVT,cfg,error,in,out);

        }
        else if (ECM_AUTO_SM_JUMP_DISCON==sm_rslt)
        {
            if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CALLUP)
            {
                /* set to disconnect and plus retry_times */
                pthread_mutex_lock(&usr_monitor_mux);
                usr_monitor_info.connet_status = 0 ;

                /* only when sim is ready*/
                if (0==strcmp(usr_monitor_info.sim_status,"SIM_READY"))
                {
                    usr_monitor_info.retry_times ++ ;
                }
                else
                {
                    usr_monitor_info.retry_times = 0 ;
                }
                //usr_monitor_info.net_recv_strength = ECM_DFLT_SIG_STRENGTH;
                //usr_monitor_info.net_type_str[ECM_NET_TYPE_STR_LEN-1] = '\0';
                //bzero((void*)(usr_monitor_info.net_type_str),ECM_NET_TYPE_STR_LEN);
                pthread_mutex_unlock(&usr_monitor_mux);

                sm_rslt = ECM_auto_sm_callup(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_DISCON)
            {
                sm_rslt = ECM_auto_sm_under_disconn(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CONNECT)
            {
                /* set to disconnect and plus retry_times */
                pthread_mutex_lock(&usr_monitor_mux);
                usr_monitor_info.connet_status = 0 ;
                usr_monitor_info.retry_times = 0 ;

                usr_monitor_info.net_recv_strength = ECM_DFLT_SIG_STRENGTH;
                usr_monitor_info.net_type_str[ECM_NET_TYPE_STR_LEN-1] = '\0';
                bzero((void*)(usr_monitor_info.net_type_str),ECM_NET_TYPE_STR_LEN);

                pthread_mutex_unlock(&usr_monitor_mux);

                sm_rslt = ECM_auto_sm_under_connect(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CALLDOWN)       
            {
                sm_rslt = ECM_auto_sm_calldown(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }

            sm_rslt = ECM_auto_sm_under_disconn(p_serial,ECM_AUTO_SM_ENTRY_EVT,cfg,error,in,out);

        }
        else if (ECM_AUTO_SM_JUMP_CALLDOWN==sm_rslt)
        {
            if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CALLUP)
            {
                sm_rslt = ECM_auto_sm_callup(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_DISCON)       
            {
                sm_rslt = ECM_auto_sm_under_disconn(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CONNECT)       
            {
                sm_rslt = ECM_auto_sm_under_connect(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CALLDOWN)       
            {
                sm_rslt = ECM_auto_sm_calldown(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }

            sm_rslt = ECM_auto_sm_calldown(p_serial,ECM_AUTO_SM_ENTRY_EVT,cfg,error,in,out);

        }
        else if (ECM_AUTO_SM_JUMP_REBOOT==sm_rslt)
        {        
            if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CALLUP)
            {
                sm_rslt = ECM_auto_sm_callup(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_DISCON)       
            {
                sm_rslt = ECM_auto_sm_under_disconn(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CONNECT)       
            {
                sm_rslt = ECM_auto_sm_under_connect(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            else if (ECM_auto_callup_state_last==ECM_AUTO_SM_STATE_CALLDOWN)       
            {
                sm_rslt = ECM_auto_sm_calldown(p_serial,ECM_AUTO_SM_EXIT_EVT,cfg,error,in,out);
            }
            sm_rslt = ECM_auto_sm_reboot(p_serial,ECM_AUTO_SM_ENTRY_EVT,cfg,error,in,out);
        }

        /* record error status */
        if (error)
        {
            if (0 != *error)
            {
                pthread_mutex_lock(&usr_monitor_mux);
                usr_monitor_info.error_code = *error ;
                pthread_mutex_unlock(&usr_monitor_mux);
        
                ECM_log(ECM_LOG_L_3,"[info] ECM_auto_sm after error=%d",*error);
            }
        }

        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm after ECM_auto_callup_state =%d",ECM_auto_callup_state );
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm after ECM_auto_callup_state_last=%d",ECM_auto_callup_state_last);
        ECM_log(ECM_LOG_L_4,"[info] ECM_auto_sm after sm_rslt =%d",sm_rslt );

    } while(0);

    if (0!=*error)
    {
        ECM_log(ECM_LOG_L_3,"[info] ECM_auto_sm error_code(%d)",*error);
    }

    return sm_rslt;

}


unsigned int ECM_auto_exec_at_flow(ttyusb_dev_t* p_serial, user_cfg_item_t* cfg)
{
    unsigned int at_err_id = 0 ;
    unsigned int error = 0 ;
    unsigned int pflow = 0 ;
    ECM_AUTO_SM_JUMP_T rflow = ECM_SM_EXEC_KEEP;


    do {
        if (!p_serial)
        {
            error = E_ECM_EXEC_AT_FLOW_PARA_NULL;
            break;
        }

        ITC_signal_init();

        /*add liwei for bug fix start*/
        itc_msg_timer      = 0 ;
        itc_msg_atcmd      = 0 ;
        itc_msg_terminal = 0 ;
        
        rflow = ECM_auto_sm(p_serial,ECM_AUTO_SM_ENTRY_EVT,cfg,&at_err_id,NULL,NULL);
        /*add liwei for bug fix end*/        

        
        /*add liwei for bug fix start*/
        if (rflow==ECM_AUTO_SM_TERMINAL)
        {
            ECM_log(ECM_LOG_L_3,"[info] ECM_auto_exec_at_flow before exit~");
        }
        /*add liwei for bug fix end*/

        while(1)
        {
            /*add liwei for bug fix start*/
            if (rflow==ECM_AUTO_SM_TERMINAL)
            {
                //signal(SIGINT,SIG_IGN);
                //signal(SIGALRM,SIG_IGN);
                /*add liwei for bug fix start */
                ITC_clear_timer(&ECM_call_timer);
                ECM_log(ECM_LOG_L_3,"[info] ECM_auto_exec_at_flow after exit~");
                //ECM_auto_call_status==ECM_AUTO_CALL_CLOSE;
                /*add liwei for bug fix end*/
                break;
            }
            /*add liwei for bug fix end*/

            pthread_mutex_lock(&itc_msg_mux);

            while ((0==itc_msg_timer)&&(0==itc_msg_atcmd)&&(0==itc_msg_terminal)      &&(0==itc_msg_quit)     &&(0==itc_msg_reboot))
            {                    
                    pthread_cond_wait(&itc_msg_cond, &itc_msg_mux);
            }

            /*add liwei for start resolve fd issue start*/
            if (1==itc_msg_timer)
            {
                itc_msg_timer =   0;
                at_err_id = 0 ;
                rflow = ECM_auto_sm(p_serial,ECM_AUTO_SM_TIMEOUT_EVT,cfg,&at_err_id,NULL,NULL);
            }
            else if (1==itc_msg_atcmd)
            {
                itc_msg_atcmd = 0;
                at_err_id = 0 ;
                rflow = ECM_auto_sm(p_serial,ECM_AUTO_SM_AT_RSP_EVT,cfg,&at_err_id,NULL,NULL);
            }
            else if (1==itc_msg_terminal)
            {                
                itc_msg_terminal = 0;
                at_err_id = 0 ;
                rflow = ECM_auto_sm(p_serial,ECM_AUTO_SM_KILL_EVT,cfg,&at_err_id,NULL,NULL);
            }
            else if (1==itc_msg_quit)
            {                
                itc_msg_quit = 0;
                at_err_id = 0 ;
                rflow = ECM_auto_sm(p_serial,ECM_AUTO_SM_REQ_QUIT_EVT,cfg,&at_err_id,NULL,NULL);
            }
            else if (1==itc_msg_reboot)
            {                
                itc_msg_reboot = 0;
                at_err_id = 0 ;
                rflow = ECM_auto_sm(p_serial,ECM_AUTO_SM_REQ_REBOOT_EVT,cfg,&at_err_id,NULL,NULL);
            }
            /*add liwei for start resolve fd issue end*/

            pthread_mutex_unlock(&itc_msg_mux);

            /*add liwei for bug fix start*/
            //if (rflow==ECM_AUTO_SM_TERMINAL)
            //{
            //    //signal(SIGINT,SIG_IGN);
            //    //signal(SIGALRM,SIG_IGN);
            //    /*add liwei for bug fix start */
            //    ITC_clear_timer(&ECM_call_timer);
            //    ECM_log(ECM_LOG_L_3,"[info] ECM_auto_exec_at_flow after exit~");
            //    //ECM_auto_call_status==ECM_AUTO_CALL_CLOSE;
            //    /*add liwei for bug fix end*/
            //    break;
            //}
            /*add liwei for bug fix end*/

        }
    }while(0);

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_3,"[info] ECM_exec_at_flow error_code(%d)",error);
    }

    return error ;
}


unsigned int ECM_auto_call(char* tty_path, char* v4v6para, char* apn)
{
    unsigned int      error = 0;
    /*add liwei for issue start*/
    //ttyusb_dev_t      serial_demo;
    //ttyusb_dev_t g_ecm_auto_serial_demo;
    /*add liwei for issue end*/
    user_cfg_item_t usr_cfg;
    unsigned int      tty_path_len = 0 ;

    do {

        ECM_auto_call_status = ECM_AUTO_CALL_INIT ;

        ECM_auto_clear_usr_monitor();

        /*add liwei for issue start*/
        //ttyusb_init(&serial_demo);
        ttyusb_init(&g_ecm_auto_serial_demo);
        /*add liwei for issue end*/

        ECM_usr_cfg_init(&usr_cfg);

        ECM_usr_cfg_set(&usr_cfg,v4v6para,apn);

        tty_path_len = (!tty_path)?0:strlen(tty_path);

        /*add liwei for issue start*/
        //error = ttyusb_config(&serial_demo,tty_path,tty_path_len);
        error = ttyusb_config(&g_ecm_auto_serial_demo,tty_path,tty_path_len);
        /*add liwei for issue end*/

        if (0 != error)
        {
            ECM_auto_call_status = ECM_AUTO_CALL_CFG_ERR;
            break;
        }

        /*add liwei for issue start*/
        //error = ttyusb_open(&serial_demo);
        error = ttyusb_open(&g_ecm_auto_serial_demo);
        /*add liwei for issue end*/

        if (0 != error)
        {
            ECM_auto_call_status = ECM_AUTO_CALL_OPEN_FAIL;
            break;
        }

        /*add liwei for issue start*/
        //error = ttyusb_reg_cb(&serial_demo,ITC_send_atcmd_msg);
        error = ttyusb_reg_cb(&g_ecm_auto_serial_demo,ITC_send_atcmd_msg);
        /*add liwei for issue end*/

        if (0 != error)
        {
            ECM_auto_call_status = ECM_AUTO_CALL_REG_FAIL;
            /*add liwei for issue start*/
            //(void)ttyusb_close(&serial_demo);
            (void)ttyusb_close(&g_ecm_auto_serial_demo);
            /*add liwei for issue end*/
            break;
        }
        
        /*add liwei for issue start*/
        //error = ttyusb_start_recv(&serial_demo);
        error = ttyusb_start_recv(&g_ecm_auto_serial_demo);
        /*add liwei for issue end*/


        if (0 != error)
        {
            ECM_auto_call_status = ECM_AUTO_CALL_RCV_FAIL;
            /*add liwei for issue start*/
            //(void)ttyusb_close(&serial_demo);
            (void)ttyusb_close(&g_ecm_auto_serial_demo);
            /*add liwei for issue end*/

            break;
        }

        ECM_auto_sm_init();

        ECM_auto_call_status = ECM_AUTO_CALL_ONLINE;
        /*add liwei for issue start*/
        //error = ECM_auto_exec_at_flow(&serial_demo,&usr_cfg);
        error = ECM_auto_exec_at_flow(&g_ecm_auto_serial_demo,&usr_cfg);
        /*add liwei for issue end*/


        if (0 != error)
        {
            /*add liwei for issue start*/
            //(void)ttyusb_close(&serial_demo);
            (void)ttyusb_close(&g_ecm_auto_serial_demo);
            /*add liwei for issue end*/

            ECM_auto_call_status = ECM_AUTO_CALL_EXIT;
            break;
        }

        /*add liwei for issue start*/
        //(void)ttyusb_close(&serial_demo);
        (void)ttyusb_close(&g_ecm_auto_serial_demo);
        /*add liwei for issue end*/

        ECM_auto_call_status = ECM_AUTO_CALL_OFFLINE;

    } while(0);

    if (0!=error)
    {
        ECM_log(ECM_LOG_L_1,"[info] ERROR CODE:%04d,ECM_auto_call_status:%01d",error,ECM_auto_call_status);
    }


    ECM_auto_call_status = ECM_AUTO_CALL_CLOSE ;

    return error ;

}



void* ECM_auto_call_thxd(void* para)
{
    unsigned int result = 0 ;

    ECM_auto_call_txd_para_t*     txd_para=(ECM_auto_call_txd_para_t*)para;
    
#if (ECM_CALL_EXIT_METHOD==    ECM_EXIT_METHOD_DETACH)
    pthread_detach(pthread_self());
    ECM_log(ECM_LOG_L_2,"[INFO] ECM_auto_call_thxd set detach method");
#endif

    result=ECM_auto_call(txd_para->ttyusb_dev, NULL, txd_para->apn);

    if (0!=result)
    {
        ECM_log(ECM_LOG_L_1,"[INFO] ECM_auto_call_thxd result%01d",result);
    }
    else
    {
        ECM_log(ECM_LOG_L_1,"[INFO] ECM_auto_call_thxd OFFLINE");
    }

#if (ECM_CALL_EXIT_METHOD==    ECM_EXIT_METHOD_DETACH)
    pthread_exit((void*)0);
#endif

    return NULL ;
}


unsigned int ECM_auto_demo_start(char* tty_path, char* v4v6para, char* apn)
{
    unsigned int                error_code = 0;
    int                         var_thxd_user = 0;
    

    unsigned int                var_thxd_wait=0;

    do {

        /*copy parameter */
        bzero((void*)&ECM_auto_call_configure,sizeof(ECM_auto_call_txd_para_t)) ;

        if (apn)            
        {
            if (strlen(apn)<64)
            {
                memcpy((void*)(ECM_auto_call_configure.apn),apn,strlen(apn));
            }
        }

#if (ECM_CALL_AUTO_PORT   ==ECM_DEMO_ON)
        if (0 != ttyusb_search_port(0,ECM_auto_port_at_path,256))
        {
            error_code = 1 ;
            break;
        }

        if (strlen(ECM_auto_port_at_path) >= TTYUSB_PATH_LEN)
        {
            error_code = 2 ;
            break;
        }

        /*set auto port flag*/
        ECM_auto_port_flag = 1 ;
        memcpy((void*)ECM_auto_call_configure.ttyusb_dev,ECM_auto_port_at_path,strlen(ECM_auto_port_at_path));
#else
        if (tty_path==NULL)
        {
            memcpy((void*)ECM_auto_call_configure.ttyusb_dev,TTYUSB_DEV_PATH,strlen(TTYUSB_DEV_PATH));
        }
        else
        {
            memcpy((void*)ECM_auto_call_configure.ttyusb_dev,tty_path,strlen(tty_path));
        }
#endif
        /*create ECM auto call thread */
        ECM_auto_call_status = ECM_AUTO_CALL_INIT ;

        var_thxd_user=pthread_create(&ECM_auto_call_txd_id,      NULL, ECM_auto_call_thxd,(void*)&ECM_auto_call_configure);

        if (0!=var_thxd_user)
        {
            error_code = 3 ;
            break;
        }

        /* wait thread up */
        for (var_thxd_wait=0;var_thxd_wait<5;var_thxd_wait++)
        {
            if (   (ECM_auto_call_status!=ECM_AUTO_CALL_CFG_ERR        )
                && (ECM_auto_call_status!=ECM_AUTO_CALL_OPEN_FAIL)
                && (ECM_auto_call_status!=ECM_AUTO_CALL_REG_FAIL       )
                && (ECM_auto_call_status!=ECM_AUTO_CALL_RCV_FAIL       )
                && (ECM_auto_call_status!=ECM_AUTO_CALL_ONLINE         )
                && (ECM_auto_call_status!=ECM_AUTO_CALL_OFFLINE        )
                && (ECM_auto_call_status!=ECM_AUTO_CALL_EXIT           )
                && (ECM_auto_call_status!=ECM_AUTO_CALL_CLOSE          ))
            {
                sleep(1);
            }
        }


        if (ECM_AUTO_CALL_ONLINE!=ECM_auto_call_status)
        {
            error_code = 4 ;
            break;
        }

    } while(0);

    return error_code ;

}



unsigned int    ECM_auto_demo_online(void)
{
    if (ECM_auto_call_status==ECM_AUTO_CALL_ONLINE)
    {
        return 0 ;
    }
    else
    {
        return 1 ;
    }
}


void ECM_auto_demo_quit_msg(void)
{
#if (ECM_CALL_EXIT_METHOD==    ECM_EXIT_METHOD_SLEEP)
    unsigned int time_limit = 0;
#endif
    if (ECM_auto_call_status==ECM_AUTO_CALL_ONLINE)
    {
        ITC_send_quit_msg();

#if (ECM_CALL_EXIT_METHOD==    ECM_EXIT_METHOD_PJOIN)        
        pthread_join(ECM_auto_call_txd_id,NULL);
#endif

#if (ECM_CALL_EXIT_METHOD==    ECM_EXIT_METHOD_SLEEP)
        while(ECM_auto_call_status!=ECM_AUTO_CALL_CLOSE       )
        {
            sleep(1);
            if (time_limit<ECM_AUTO_SAFE_EXIT_TIME)
            {
                time_limit++ ;
            }
            else
            {
                break;
            }
        }
#endif
    }
}


void ECM_auto_demo_reboot_msg(void)
{
#if (ECM_CALL_EXIT_METHOD==    ECM_EXIT_METHOD_SLEEP)
        unsigned int time_limit = 0;
#endif


    if (ECM_auto_call_status==ECM_AUTO_CALL_ONLINE)
    {
        ITC_send_reboot_msg();
#if (ECM_CALL_EXIT_METHOD==    ECM_EXIT_METHOD_PJOIN)  
        pthread_join(ECM_auto_call_txd_id,NULL);
#endif

#if (ECM_CALL_EXIT_METHOD==    ECM_EXIT_METHOD_SLEEP)  
        while(ECM_auto_call_status!=ECM_AUTO_CALL_CLOSE       )
        {
            sleep(1);
            if (time_limit<ECM_AUTO_SAFE_EXIT_TIME)
            {
                time_limit++ ;
            }
            else
            {
                break;
            }
        }
#endif
    }
}


unsigned int ECM_auto_check_port(ttyusb_dev_t* p_serial)
{
    unsigned int error_code = 0;
    char at_port[256] = {0};

    do {
        if (0 != ttyusb_search_port(0,at_port,256))
        {
            error_code = 1 ;
            break;
        }
        if (strlen(at_port) >= TTYUSB_PATH_LEN)
        {
            error_code = 2 ;
            break;
        }

        if (0 != strcmp(at_port,ECM_auto_port_at_path))
        {
            error_code = 3 ;
            break;
        }
    } while(0);

    if (0 != error_code)
    {
        ECM_log(ECM_LOG_L_3,"[info] ECM_auto_check_port changed......");
    }

    return error_code ;
}



