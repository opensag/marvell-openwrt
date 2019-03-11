
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


static unsigned int     ECM_dbg_level = ECM_LOG_L_1 ;

static Ecm_log_t        ECM_dbg_entry = NULL ;

void ECM_log_init(Ecm_log_t ptr_debug_entry, unsigned int log_level)
{
    if (NULL!=ptr_debug_entry)
    {
        ECM_dbg_entry = ptr_debug_entry;
        ECM_dbg_level = log_level;
    }
}

void ECM_log(unsigned int log_level, const char* msg, ...)
{
    char    buf[ITC_DBG_BUF_LEN+64] = {0};
    va_list ap;
    
    va_start(ap,msg);
    vsprintf(buf,msg,ap);
    va_end(ap);

    if ((NULL != ECM_dbg_entry) && (log_level<=ECM_dbg_level))
    {
        (ECM_dbg_entry)(buf);
    }
}

/*call sample */

/*
ECM_log(ECM_LOG_L_1, ... );  output error information
ECM_log(ECM_LOG_L_2, ... );  output error & important information
ECM_log(ECM_LOG_L_3, ... );  output error,important & run path info.
ECM_log(ECM_LOG_L_4, ... );  output all log info, detailed info.
*/

#if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
void ECM_aux_led_on(void)
{
    char led_cmd_str[ECM_ALED_CMD_STR_LEN] ;

    bzero((void*)led_cmd_str,ECM_ALED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 200 > %s",ECM_ALED_NODE_ON);
    if(0==access(ECM_ALED_NODE_ON, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ALED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 0 > %s",ECM_ALED_NODE_OFF);
    if(0==access(ECM_ALED_NODE_OFF, F_OK))
    {
        system(led_cmd_str);
    }

    ECM_log(ECM_LOG_L_3,"[info] LED on");
}

void ECM_aux_led_off(void)
{
    char led_cmd_str[ECM_ALED_CMD_STR_LEN] ;

    bzero((void*)led_cmd_str,ECM_ALED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 0 > %s",ECM_ALED_NODE_ON);
    if(0==access(ECM_ALED_NODE_ON, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ALED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 200 > %s",ECM_ALED_NODE_OFF);
    if(0==access(ECM_ALED_NODE_OFF, F_OK))
    {
        system(led_cmd_str);
    }
    ECM_log(ECM_LOG_L_3,"[info] LED off");
}

void ECM_aux_led_fls(void)
{
    char led_cmd_str[ECM_ALED_CMD_STR_LEN] ;

    bzero((void*)led_cmd_str,ECM_ALED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 500 > %s",ECM_ALED_NODE_ON);
    if(0==access(ECM_ALED_NODE_ON, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ALED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 500 > %s",ECM_ALED_NODE_OFF);
    if(0==access(ECM_ALED_NODE_OFF, F_OK))
    {
        system(led_cmd_str);
    }
    ECM_log(ECM_LOG_L_3,"[info] LED flash");
}

#endif


#if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)

void ECM_aux_set_siglevel_led_off(void)
{
    char led_cmd_str[ECM_ASLLED_CMD_STR_LEN] ;

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 0 > %s",ECM_ASLLED_NODE_0);
    if(0==access(ECM_ASLLED_NODE_0, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 0 > %s",ECM_ASLLED_NODE_1);
    if(0==access(ECM_ASLLED_NODE_1, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 0 > %s",ECM_ASLLED_NODE_2);
    if(0==access(ECM_ASLLED_NODE_2, F_OK))
    {
        system(led_cmd_str);
    }

    ECM_log(ECM_LOG_L_3,"[info] Set LED Level Low");
}



void ECM_aux_set_siglevel_led_low(void)
{
    char led_cmd_str[ECM_ASLLED_CMD_STR_LEN] ;

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 1 > %s",ECM_ASLLED_NODE_0);
    if(0==access(ECM_ASLLED_NODE_0, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 0 > %s",ECM_ASLLED_NODE_1);
    if(0==access(ECM_ASLLED_NODE_1, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 0 > %s",ECM_ASLLED_NODE_2);
    if(0==access(ECM_ASLLED_NODE_2, F_OK))
    {
        system(led_cmd_str);
    }

    ECM_log(ECM_LOG_L_3,"[info] Set LED Level Low");
}

void ECM_aux_set_siglevel_led_middle(void)
{
    char led_cmd_str[ECM_ASLLED_CMD_STR_LEN] ;

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 1 > %s",ECM_ASLLED_NODE_0);
    if(0==access(ECM_ASLLED_NODE_0, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 1 > %s",ECM_ASLLED_NODE_1);
    if(0==access(ECM_ASLLED_NODE_1, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 0 > %s",ECM_ASLLED_NODE_2);
    if(0==access(ECM_ASLLED_NODE_2, F_OK))
    {
        system(led_cmd_str);
    }

    ECM_log(ECM_LOG_L_3,"[info] Set LED Level Middle");
}

void ECM_aux_set_siglevel_led_high(void)
{
    char led_cmd_str[ECM_ASLLED_CMD_STR_LEN] ;

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 1 > %s",ECM_ASLLED_NODE_0);
    if(0==access(ECM_ASLLED_NODE_0, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 1 > %s",ECM_ASLLED_NODE_1);
    if(0==access(ECM_ASLLED_NODE_1, F_OK))
    {
        system(led_cmd_str);
    }

    bzero((void*)led_cmd_str,ECM_ASLLED_CMD_STR_LEN);
    sprintf(led_cmd_str,"echo 1 > %s",ECM_ASLLED_NODE_2);
    if(0==access(ECM_ASLLED_NODE_2, F_OK))
    {
        system(led_cmd_str);
    }

    ECM_log(ECM_LOG_L_3,"[info] Set LED Level High");
}



#endif



