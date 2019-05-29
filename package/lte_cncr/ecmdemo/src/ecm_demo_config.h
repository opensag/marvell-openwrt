/* ============================================================================
 * @file: ecm_demo_config.h
 * @author: Wei,LI
 * @Copyright:                      GoSUNCN
* @Website:                         www.ztewelink.com
* @Email:                           ztewelink@zte.com.cn
* @version:                         "ECM_CALLV1.0.1B02"
* @date:                            "2019-02-11"

* History:

1. No IMEI number, could continue to connect network.
2. Optimize memory of variable ttyusb_drv_map_tab[] from RAM area to CODE area, reduce memory using.
3. Add Print Log Level function.
4. Remove unnecessary Log Print
5. Modify AT command echo issue

......................................................................................For Alibaba Linux OpenWRT system.
......................................................................................ECM_CALLV1.0.0B07
......................................................................................Wei,LI,2018_09_10

1. Add ECM_DEMO -t autoconfig function
2. Add ECM_DEMO query simstatus function
3. Add ECM_DEMO query imei funcion
4. Add ECM_DEMO query iccid funcion
5. Add ECM_DEMO query net type funcion
6. Add ECM_DEMO query error info funcion
7. ECM_DEMO & ECM_DEMO_AUTO proc&proc communication

......................................................................................For Alibaba Linux OpenWRT system.
......................................................................................ECM_CALLV1.0.0B08
......................................................................................Wei,LI,2018_09_19


1. Sync/Imediately release RECV_THXD Memory, please refer ECM_auto_sm_under_disconn()
2. Adjust some log information 

......................................................................................For Alibaba Linux OpenWRT system.
......................................................................................ECM_CALLV1.0.0B09
......................................................................................Wei,LI,2018_10_09

1. Add led control logical function

......................................................................................For Alibaba Linux OpenWRT system.
......................................................................................ECM_CALLV1.0.0B10
......................................................................................Wei,LI,2018_11_19

1. If LTE Module switch port, fix the switch port bugs...
2. Adjust some log information 

......................................................................................For Alibaba Linux OpenWRT system.
......................................................................................ECM_CALLV1.0.0B10
......................................................................................Wei,LI,2019_01_11


1. Change local varible to global variable: g_ecm_auto_serial_demo
2. Adjust some log information 

......................................................................................For Alibaba Linux OpenWRT system.
......................................................................................ECM_CALLV1.0.1B01
......................................................................................Wei,LI,2019_01_11


1. Fix bugs if AT+ZECMCALL=0 return ERROR
2. Reboot Flow don't exec AT+ZECMCALL=0

......................................................................................For Alibaba Linux OpenWRT system.
......................................................................................ECM_CALLV1.0.1B02
......................................................................................Wei,LI,2019_01_31



1. Add new requirement of signal strength LED
2. on optimizing Log print
......................................................................................For Alibaba Linux OpenWRT system.
......................................................................................ECM_CALLV1.0.1B03
......................................................................................Wei,LI,2019_02_27

1. Add 305A,1406
......................................................................................For Alibaba Linux OpenWRT system.
......................................................................................ECM_CALLV1.0.1B04
......................................................................................Wei,LI,2019_03_11


* ============================================================================*/

#ifndef __ECM_DEMO_CONFIG__H

#define __ECM_DEMO_CONFIG__H

#include <stdarg.h>

#define ECM_DEMO_ON                           1

#define ECM_DEMO_OFF                          0

#define   TTYUSB_PATH_LEN                     (64)

#define TTYUSB_BUF_LEN                        (256)

#define ITC_DBG_BUF_LEN                       (256)

#define ECM_APN_MAX_LEN                        64

#define ECM_PERSONAL_AT_LEN                    128

#define ECM_V4V6_MAX_LEN                       16

#define ECM_NET_TYPE_STR_LEN                   32

#define ECM_IMEI_NUMBERS                       15

#define ECM_SIM_STATUS_LEN                     64

#define ECM_ICCID_NUMBERS                      20


#define TTYUSB_BAUDRATE                        B115200

#define TTYUSB_DEV_PATH                        "/dev/ttyUSB1"

#define ECM_AT_FLOW_INTV                       300000/*300ms*/

#define ECM_AT_RECV_INTV                       500000/*500ms*/

#define ECM_RECOVERY_TIME                      30

#define ECM_AUTO_RETRY_INTV                    15    /*ECM auto connnect interv time*/


#define ECM_CALL_VERSION                       "ECM_CALLV1.0.1B04"

#define ECM_CALL_DATE                          "2019-03-11"

#define ECM_CALL_AUTO_PORT                     ECM_DEMO_ON

/*add liwei for fix gswerr id 0000 start */
#define ECM_CALL_FIX_GSWERR_ID_0000                          ECM_DEMO_OFF
/*add liwei for fix gswerr id 0000 end */
#if 0
#define   ECM_CALL_HOST_PLUG_POLARITY                        1
#define ECM_CALL_HOST_PLUG_PULL                              3
#else
#define   ECM_CALL_HOST_PLUG_POLARITY                        0
#define ECM_CALL_HOST_PLUG_PULL                              0
#endif

/*add liwei for ali project led operation*/
#define ECM_AUTO_LED_ON                        ECM_DEMO_ON

/*add liwei for ali project signal led operation*/
#define ECM_AUTO_SIG_LEVEL_LED_ON                          ECM_DEMO_ON


#if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
#define ECM_ALED_NODE_ON                           "/sys/class/leds/lte/delay_on"
#define ECM_ALED_NODE_OFF                          "/sys/class/leds/lte/delay_off"
#define ECM_ALED_CMD_STR_LEN                       64
#endif

#if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
#define ECM_ASLLED_NODE_0                            "/sys/class/leds/lte_0/brightness"
#define ECM_ASLLED_NODE_1                            "/sys/class/leds/lte_1/brightness"
#define ECM_ASLLED_NODE_2                            "/sys/class/leds/lte_2/brightness"
#define ECM_ASLLED_CMD_STR_LEN                       64
#endif


typedef enum
{
    ECM_OP_UP=0,
    ECM_OP_DOWN=1,
    ECM_OP_CONFIG=2,
    ECM_OP_QSTATUS=3,
    ECM_OP_CFG_HOT_SIM=4,/*configure host plugin sim card, if board is support */
    ECM_OP_Q_SIM_STATUS=5,
    ECM_OP_PERSON_AT=6,
    ECM_OP_Q_AUTOCFG=7,
    ECM_OP_OTHER=8,

} ECM_USR_OPS_T;


typedef enum
{
    ECM_LOG_L_1=0,/* output error information */
    ECM_LOG_L_2,/* output error & important information */
    ECM_LOG_L_3,/* output error,important & run path info. */
    ECM_LOG_L_4,/* output all log info. */

} ECM_LOG_LEVEL_T;


typedef enum
{
    E_ECM_CALL_SUCCESS                        =0,/* ECM call success */
    E_TTYUSB_CONFIG_OK                        =0,

    E_TTYUSB_CONFIG_TTY_S_NULL=1,                 /* tty struct is empty*/
    E_TTYUSB_CONFIG_PATH_NULL=2,/*                    undefined path*/
    E_TTYUSB_CONFIG_PATH_LENG=3,/* device path length error*/
    E_TTYUSB_CONFIG_OVER_MAX=4,/* device path over max 63 bytes*/

    E_TTYUSB_OPEN_TTY_S_NULL=5,                /* tty struct is empty*/
    E_TTYUSB_OPEN_TTY_DEV_FAIL=6,                /* open device ttyUSBX fail*/
    E_TTYUSB_OPEN_TTY_BAK_CFG=7,                /* back device configure fail*/
    E_TTYUSB_OPEN_TTY_FCNTL_ERR=8,                /* back device configure fail*/

    E_TTYUSB_CLOSE_TTY_S_NULL=9,                /* tty struct is empty*/
    E_TTYUSB_CLOSE_TTY_FD_ERR=10,                /* tty fd error */


    E_TTYUSB_SEND_PARA_ERR=11,                 /* tty send para error */
    E_TTYUSB_SEND_LENTH_ERR=12,                /* tty send at cmd   length error */
    E_TTYUSB_SEND_FD_ERR=13,                   /* tty fd error */
    E_TTYUSB_SEND_ABNORMAL=14,                 /* tty send abnormal error */


    E_TTYUSB_REG_CB_PARA_NULL=15,                 /* tty   receive callback   null */


    E_TTYUSB_START_RECV_TTY_S_NULL=16,                /* tty struct is empty*/
    E_TTYUSB_START_RECV_NEW_TXD_FAIL=17,                /* create thread fail*/
    E_TTYUSB_START_RECV_TXD_BOOT_FAIL=18,                /* recv thread run abnormal       */


    E_TTYUSB_RECV_THXD_PARA_NULL=19,                /* recv thread data fail        */
    E_TTYUSB_RECV_THXD_BE_TERMINAL=20,                /* recv thread killed         */


    E_ECM_CALL_START_EXT_FD_ERR=21,/*user input error fd */


    E_ECM_EXEC_AT_FLOW_PARA_NULL=22,/*                       parameter empty */
    E_ECM_EXEC_AT_FLOW_SM0_RET_FAIL=23,/* state machine return fail */
    E_ECM_EXEC_AT_FLOW_SM_RET_FAIL=24,/* state machine return fail */
    E_ECM_EXEC_AT_FLOW_SM_BE_TERM=25,/* state machine be terminal */
    E_ECM_EXEC_AT_FLOW_SM_SWITCH_FAIL=26,/* state machine switch fail */

    E_ECM_CALL_PARA_UNPACK_FAIL=27,/* user input error para */

    E_ECM_CALL_USR_TERMINAL=28,
    
    E_ECM_CALL_TTY_USR_OPEN_FAIL=29,

    E_ECM_PORT_JUMP=30,
    E_ECM_PORT_SEARCH_FAIL=31,

    E_ECM_ATI_SEND_FAIL=90,/* ATI: send fail */
    E_ECM_ATI_RECV_ISSUE=91,/* ATI: recv fail */
    E_ECM_ATI_SEND_MAX=92,/* ATI: send                over max*/
    E_ECM_ATI_RESEND_ERR=93,/* ATI: send                over max*/
    E_ECM_ATI_OTHER_ERR=94,/* ATI: send                 other error*/
    E_ECM_ATI_TERMINAL=95,/* ATI: send terminal */    


    E_ECM_ZSWITCH_QUERY_FAIL=100,/* zswitchq: send fail */
    E_ECM_ZSWITCH_QRECV_ISSUE,/* zswitchq: recv fail */
    E_ECM_ZSWITCH_QUERY_MAX,/* zswitchq: send                over max*/
    E_ECM_ZSWITCH_QRESEND_ERR,/* zswitchq: send                over max*/
    E_ECM_ZSWITCH_QUERY_TERM,/* zswitchq: send                 other error*/
    E_ECM_ZSWITCH_QOTHER_ERR,          /* zswitchq: send terminal */

    E_ECM_ZSWITCH_SEND_FAIL=110,/* zswitch: send fail */
    E_ECM_ZSWITCH_RECV_ISSUE,/* zswitch: recv fail */
    E_ECM_ZSWITCH_SEND_MAX,/* zswitch: send                over max*/
    E_ECM_ZSWITCH_RESEND_ERR,/* zswitch: send                over max*/
    E_ECM_ZSWITCH_OTHER_ERR,/* zswitch: send                 other error*/
    E_ECM_ZSWITCH_TERMINAL,/* zswitch: send terminal */

    E_ECM_ZADSET_QUERY_FAIL=120,/* ZADSETq: send fail */
    E_ECM_ZADSET_QRECV_ISSUE,/* ZADSETq: recv fail */
    E_ECM_ZADSET_QUERY_MAX,/* ZADSETq: send                over max*/
    E_ECM_ZADSET_QRESEND_ERR,/* ZADSETq: send                over max*/
    E_ECM_ZADSET_QUERY_TERM,/* ZADSETq: send                 other error*/
    E_ECM_ZADSET_QOTHER_ERR,          /* ZADSETq: send terminal */

    E_ECM_ZADSET_SEND_FAIL=130,/* ZADSET: send fail */
    E_ECM_ZADSET_RECV_ISSUE,/* ZADSET: recv fail */
    E_ECM_ZADSET_SEND_MAX,/* ZADSET: send                over max*/
    E_ECM_ZADSET_RESEND_ERR,/* ZADSET: send                over max*/
    E_ECM_ZADSET_OTHER_ERR,/* ZADSET: send                 other error*/
    E_ECM_ZADSET_TERMINAL,/* ZADSET: send terminal */


    E_ECM_CFUN_QUERY_FAIL=140,/* cfun: send fail */
    E_ECM_CFUN_QRECV_ISSUE,/* cfun: recv fail */
    E_ECM_CFUN_QUERY_MAX,/* cfun: send                    over max*/
    E_ECM_CFUN_QRESEND_ERR,/* cfun: send                over max*/
    E_ECM_CFUN_QUERY_TERM,/* cfun: send                 other error*/
    E_ECM_CFUN_QOTHER_ERR,          /* cfun: send terminal */

    E_ECM_CPIN_QUERY_FAIL=150,/* cpin: send fail */
    E_ECM_CPIN_QNOT_INSERT,/* cpin: sin not insert */
    E_ECM_CPIN_QRECV_ISSUE,/* cpin: recv fail */
    E_ECM_CPIN_QUERY_MAX,/* cpin: send                    over max*/
    E_ECM_CPIN_QRESEND_ERR,/* cpin: send                over max*/
    E_ECM_CPIN_QUERY_TERM,/* cpin: send                 other error*/
    E_ECM_CPIN_QOTHER_ERR,          /* cpin: send terminal */
    E_ECM_CPIN_Q_SIM_BUSY,          /* cpin: SIM Busy */    
    E_ECM_CPIN_Q_SIM_PIN,          /* cpin: SIM Pin */
    E_ECM_CPIN_Q_SIM_PUK,          /* cpin: SIM      PUK*/
    E_ECM_CPIN_Q_SIM_FAILURE,          /* cpin: SIM failure */

    E_ECM_ZBAND_QUERY_FAIL=170,/* zband: send fail */
    E_ECM_ZBAND_QRECV_ISSUE,/* zband: recv fail */
    E_ECM_ZBAND_QUERY_MAX,/* zband: send                    over max*/
    E_ECM_ZBAND_QRESEND_ERR,/* zband: send                over max*/
    E_ECM_ZBAND_QUERY_TERM,/* zband: send                 other error*/
    E_ECM_ZBAND_QOTHER_ERR,          /* zband: send terminal */


    E_ECM_CSQ_QUERY_FAIL=180,/* csq: send fail */
    E_ECM_CSQ_QRECV_ISSUE,/* csq: recv fail */
    E_ECM_CSQ_QUERY_MAX,/* csq: send                    over max*/
    E_ECM_CSQ_QRESEND_ERR,/* csq: send                over max*/
    E_ECM_CSQ_QUERY_TERM,/* csq: send                 other error*/
    E_ECM_CSQ_QOTHER_ERR,          /* csq: send terminal */


    E_ECM_CREG_QUERY_FAIL=190,/* creg: send fail */
    E_ECM_CREG_QRECV_ISSUE,/* creg: recv fail */
    E_ECM_CREG_QUERY_MAX,/* creg: send                    over max*/
    E_ECM_CREG_QRESEND_ERR,/* creg: send                over max*/
    E_ECM_CREG_QUERY_TERM,/* creg: send                 other error*/
    E_ECM_CREG_QOTHER_ERR,          /* creg: send terminal */

    E_ECM_CGDCONT_SEND_FAIL=200,/* cgdcont: send fail */
    E_ECM_CGDCONT_RECV_ISSUE,/* cgdcont: recv fail */
    E_ECM_CGDCONT_SEND_MAX,/* cgdcont: send                over max*/
    E_ECM_CGDCONT_RESEND_ERR,/* cgdcont: send                over max*/
    E_ECM_CGDCONT_OTHER_ERR,/* cgdcont: send                 other error*/
    E_ECM_CGDCONT_TERMINAL,/* cgdcont: send terminal */

    E_ECM_ZECMCALL1_SEND_FAIL=210,/* at+zecmcall=1: send fail */
    E_ECM_ZECMCALL1_RECV_ISSUE,/* at+zecmcall=1: recv fail */
    E_ECM_ZECMCALL1_SEND_MAX,/* at+zecmcall=1: send                over max*/
    E_ECM_ZECMCALL1_RESEND_ERR,/* at+zecmcall=1: send                over max*/
    E_ECM_ZECMCALL1_OTHER_ERR,/* at+zecmcall=1: send                 other error*/
    E_ECM_ZECMCALL1_TERMINAL,/* at+zecmcall=1: send terminal */

    E_ECM_ZECMCALL0_SEND_FAIL=220,/* at+zecmcall=0: send fail */
    E_ECM_ZECMCALL0_RECV_ISSUE,/* at+zecmcall=0: recv fail */
    E_ECM_ZECMCALL0_SEND_MAX,/* at+zecmcall=0: send                over max*/
    E_ECM_ZECMCALL0_RESEND_ERR,/* at+zecmcall=0: send                over max*/
    E_ECM_ZECMCALL0_OTHER_ERR,/* at+zecmcall=0: send                 other error*/
    E_ECM_ZECMCALL0_TERMINAL,/* at+zecmcall=0: send terminal */

    E_ECM_ZECMCALL_C_QUERY_FAIL=230,/* at+zecmcall?: send fail */
    E_ECM_ZECMCALL_C_QRECV_ISSUE,/* at+zecmcall?: recv fail */
    E_ECM_ZECMCALL_C_QSEND_MAX,/* at+zecmcall?: send                over max*/
    E_ECM_ZECMCALL_C_QRESEND_ERR,/* at+zecmcall?: send                over max*/
    E_ECM_ZECMCALL_C_OTHER_ERR,/* at+zecmcall?: send                 other error*/
    E_ECM_ZECMCALL_C_TERMINAL,/* at+zecmcall?: send terminal */


    E_ECM_CFUN_1_1_SEND_FAIL=240,/* at+cfun=1,1: send fail */
    E_ECM_CFUN_1_1_RECV_ISSUE,/* at+cfun=1,1: recv fail */
    E_ECM_CFUN_1_1_SEND_MAX,/* at+cfun=1,1: send                over max*/
    E_ECM_CFUN_1_1_RESEND_ERR,/* at+cfun=1,1: send                over max*/
    E_ECM_CFUN_1_1_OTHER_ERR,/* at+cfun=1,1: send                 other error*/
    E_ECM_CFUN_1_1_TERMINAL,/* at+cfun=1,1: send terminal */

    E_ECM_ZPDPTYPE_QUERY_FAIL=250,/* at+zpdptype?: send fail */
    E_ECM_ZPDPTYPE_QRECV_ISSUE,/* at+zpdptype?: recv fail */
    E_ECM_ZPDPTYPE_QSEND_MAX,/* at+zpdptype?: send                over max*/
    E_ECM_ZPDPTYPE_QRESEND_ERR,/* at+zpdptype?: send                over max*/
    E_ECM_ZPDPTYPE_OTHER_ERR,/* at+zpdptype?: send                 other error*/
    E_ECM_ZPDPTYPE_TERMINAL,/* at+zpdptype?: send terminal */

    E_ECM_CGSN_QUERY_FAIL=260,/* at+cgsn: send fail */
    E_ECM_CGSN_QRECV_ISSUE,/* at+cgsn: recv fail */
    E_ECM_CGSN_QSEND_MAX,/* at+cgsn: send                over max*/
    E_ECM_CGSN_QRESEND_ERR,/* at+cgsn: send                over max*/
    E_ECM_CGSN_OTHER_ERR,/* at+cgsn: send                 other error*/
    E_ECM_CGSN_TERMINAL,/* at+cgsn: send terminal */

    E_ECM_ZGETICCID_QUERY_FAIL=270,/* at+zgeticcid: send fail */
    E_ECM_ZGETICCID_QRECV_ISSUE,/* at+zgeticcid: recv fail */
    E_ECM_ZGETICCID_QSEND_MAX,/* at+zgeticcid: send                over max*/
    E_ECM_ZGETICCID_QRESEND_ERR,/* at+zgeticcid: send                over max*/
    E_ECM_ZGETICCID_OTHER_ERR,/* at+zgeticcid: send                 other error*/
    E_ECM_ZGETICCID_TERMINAL,/* at+zgeticcid: send terminal */

    E_ECM_ZPAS_QUERY_FAIL=280,/* at+zpas?: send fail */
    E_ECM_ZPAS_QNO_SERVICE,/*at+zpas?: recv fail */
    E_ECM_ZPAS_QLIMIT_SERVICE,/*at+zpas?: recv fail */

    E_ECM_ZPAS_QRECV_ISSUE,/*at+zpas?: recv fail */
    E_ECM_ZPAS_QSEND_MAX,/* at+zpas?: send                over max*/
    E_ECM_ZPAS_QRESEND_ERR,/* at+zpas?: send                over max*/
    E_ECM_ZPAS_OTHER_ERR,/* at+zpas?: send                 other error*/
    E_ECM_ZPAS_TERMINAL,/* at+zpas?: send terminal */


    E_ECM_ZSDT_SEND_FAIL=290,/* AT+ZSDT: send fail */
    E_ECM_ZSDT_RECV_ISSUE,/* AT+ZSDT: recv fail */
    E_ECM_ZSDT_SEND_MAX,/* AT+ZSDT: send                over max*/
    E_ECM_ZSDT_RESEND_ERR,/* AT+ZSDT: send                over max*/
    E_ECM_ZSDT_OTHER_ERR,/* AT+ZSDT: send                 other error*/
    E_ECM_ZSDT_TERMINAL,/* AT+ZSDT: send terminal */


    E_ECM_ZSDT_QUERY_FAIL=300,/* AT+ZSDT? send fail */
    E_ECM_ZSDT_QRECV_ISSUE,/* AT+ZSDT? recv fail */
    E_ECM_ZSDT_QUERY_MAX,/* AT+ZSDT? send                over max*/
    E_ECM_ZSDT_QRESEND_ERR,/* AT+ZSDT? send                over max*/
    E_ECM_ZSDT_QUERY_TERM,/* AT+ZSDT? send                 other error*/
    E_ECM_ZSDT_QOTHER_ERR,          /* AT+ZSDT? send terminal */


    E_ECM_ZCDS_QUERY_FAIL=310,/* AT+ZCDS? send fail */
    E_ECM_ZCDS_QRECV_ISSUE,/* AT+ZCDS? recv fail */
    E_ECM_ZCDS_QUERY_MAX,/* AT+ZCDS? send                over max*/
    E_ECM_ZCDS_QRESEND_ERR,/* AT+ZCDS? send                over max*/
    E_ECM_ZCDS_QUERY_TERM,/* AT+ZCDS? send                 other error*/
    E_ECM_ZCDS_QOTHER_ERR,          /* AT+ZCDS? send terminal */

    E_ECM_PERSON_AT_SEND_FAIL=320,/* Personalization at send fail */
    E_ECM_PERSON_AT_RECV_ISSUE=321,/* Personalization at                      recv fail */
    E_ECM_PERSON_AT_SEND_MAX=322,/* Personalization at send                over max*/
    E_ECM_PERSON_AT_RESEND_ERR=323,/* Personalization at send                over max*/
    E_ECM_PERSON_AT_OTHER_ERR=324,/* Personalization at send                 other error*/
    E_ECM_PERSON_AT_TERMINAL=325,/* Personalization at send terminal */

/*add liwei for fix gswerr id 0000 start */
#if (ECM_CALL_FIX_GSWERR_ID_0000    ==ECM_DEMO_ON)
    E_ECM_GSWERR_QUERY_FAIL=330,/* AT+GSWERR=0000: send fail */
    E_ECM_GSWERR_QRECV_ISSUE=331,/* AT+GSWERR=0000: recv fail */
    E_ECM_GSWERR_QUERY_MAX=332,/* AT+GSWERR=0000: send                over max*/
    E_ECM_GSWERR_QRESEND_ERR=333,/* AT+GSWERR=0000: send                over max*/
    E_ECM_GSWERR_QOTHER_ERR=334,/* AT+GSWERR=0000: send                 other error*/
    E_ECM_GSWERR_QTERMINAL=335,            /* AT+GSWERR=0000: send terminal */
#endif
/*add liwei for fix gswerr id 0000 end */

} ECM_ERROR_CODE_TABLE_T;


#define ECM_AUTOCFG_IMEI_LEN                            16
#define ECM_AUTOCFG_ICCID_LEN                           24
#define ECM_AUTOCFG_SIM_STATUS_LEN                      64

#define ECM_AUTOCFG_ERR_INFO_LEN                        64


#define ECM_DFLT_SIG_STRENGTH                           199


typedef struct
{
    unsigned int connet_status ;
    unsigned int retry_times ;
    unsigned int error_code ;

    unsigned int rf_recv_strength;
    unsigned int net_recv_strength;
    unsigned int net_type_val ;
    char            net_type_str[ECM_NET_TYPE_STR_LEN] ;

    char            imei[ECM_AUTOCFG_IMEI_LEN] ;
    char            iccid[ECM_AUTOCFG_ICCID_LEN] ;
    char            sim_status[ECM_AUTOCFG_SIM_STATUS_LEN] ;
    char            error_info[ECM_AUTOCFG_ERR_INFO_LEN] ;
    char            error_ext_info[ECM_AUTOCFG_ERR_INFO_LEN] ;

} ECM_auto_monitor_t ;

#define ECM_AUTOCFG_SIM_NOT_INSERT "SIM_NOT_INSERT"

#define ECM_AUTOCFG_SIM_READY                     "SIM_READY"
#define ECM_AUTOCFG_SIM_BUSY                      "SIM_BUSY"
#define ECM_AUTOCFG_SIM_PIN                       "SIM_PIN"
#define ECM_AUTOCFG_SIM_PUK                       "SIM_PUK"
#define ECM_AUTOCFG_SIM_FAILURE                   "SIM_FAILURE"
#define ECM_AUTOCFG_SIM_ERROR                     "SIM_ERROR"

#define ECM_AUTOCFG_E_SUCCESS                                          "Success"
#define ECM_AUTOCFG_E_ERROR_OTHER                                      "Error_Other"
#define ECM_AUTOCFG_E_SEND_AT_ERROR                                    "Error_AT_Send"
#define ECM_AUTOCFG_E_RECV_AT_ERROR                                    "Error_AT_Analyz"
#define ECM_AUTOCFG_E_SEND_OVERMAX_ERR                                 "Error_AT_SendOverMax"
#define ECM_AUTOCFG_E_OTHER_AT_ERROR                                   "Error_AT_Other"
#define ECM_AUTOCFG_E_ERROR_TERMINATED                                 "Error_Terminated"

#define ECM_AUTOCFG_E_ERROR_SIM_NOT_INSERT                             "SIM_NOT_INSERT"
#define ECM_AUTOCFG_E_ERROR_SIM_BUSY                                   "SIM_BUSY"
#define ECM_AUTOCFG_E_ERROR_SIM_PIN                                    "SIM_PIN"
#define ECM_AUTOCFG_E_ERROR_SIM_PUK                                    "SIM_PUK"
#define ECM_AUTOCFG_E_ERROR_SIM_FAILURE                                "SIM_FAIL"

#define ECM_AUTOCFG_E_NET_REG_ERROR                                    "NET_REG_FAIL"
#define ECM_AUTOCFG_E_NET_DROP                                         "NET_DROP"

#define ECM_AUTOCFG_E_ERROR_NO_SERVICE                                 "NO_SERVICE"
#define ECM_AUTOCFG_E_ERROR_LIMIT_SERVICE                              "LIMIT_SERVICE"

#define ECM_AUTOCFG_E_DIAL_FAIL                                        "DIAL_FAIL"
#define ECM_AUTOCFG_E_DIAL_BUT_NO_RES                                  "DIAL_NO_RES"


typedef void (*Ecm_log_t)(const char* msg, ...);

typedef void (*ecm_recv_cb_t)(char* data, int data_len);

void ECM_error_map(unsigned int error, char* error_info, char* error_ext_info);


void ECM_log_init(Ecm_log_t ptr_debug_entry, unsigned int log_level);

void ECM_log(unsigned int log_level, const char* msg, ...);

#if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
void ECM_aux_led_on(void);
void ECM_aux_led_off(void);
void ECM_aux_led_fls(void);
#endif

#if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
void ECM_aux_set_siglevel_led_off(void);
void ECM_aux_set_siglevel_led_low(void);
void ECM_aux_set_siglevel_led_middle(void);
void ECM_aux_set_siglevel_led_high(void);
#endif

#endif

