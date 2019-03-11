
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "ecm_demo_config.h"
#include "ecm_demo_msg.h"
#include "ecm_demo_ttydev.h"
#include "ecm_demo_atctl.h"
#include "ecm_demo_autocfg.h"

#define ECM_DEMO_DBG_FLAG                      ECM_DEMO_ON

void ECM_print(const char* msg, ...)
{
#if (ECM_DEMO_DBG_FLAG==ECM_DEMO_ON)
    char buf[TTYUSB_BUF_LEN+64];
    va_list ap;
    struct timespec time;
    struct tm time_current;
    
    bzero((void*)buf,sizeof(buf));
    bzero((void*)&time,sizeof(time));
    bzero((void*)&time_current,sizeof(time_current));

    va_start(ap,msg);
    vsprintf(buf,msg,ap);
    va_end(ap);

    clock_gettime(CLOCK_REALTIME, &time);
    localtime_r(&time.tv_sec, &time_current);

    fprintf(stdout,"[%02d_%02d_%02d:%02d:%02d] %s\n",
        //time_current.tm_year + 1900,
        time_current.tm_mon+1,
        time_current.tm_mday,
        time_current.tm_hour,
        time_current.tm_min,
        time_current.tm_sec,
        buf);/*User could define print function himself*/
#endif
}


void ECM_print_ext(const char* msg, ...)
{
#if (ECM_DEMO_DBG_FLAG==ECM_DEMO_ON)
    char buf[TTYUSB_BUF_LEN+64];
    va_list ap;    
    bzero((void*)buf,sizeof(buf));
    va_start(ap,msg);
    vsprintf(buf,msg,ap);
    va_end(ap);
    fprintf(stdout,"%s\n", buf);
#endif
}

unsigned int ECM_usage(void)
{
     ECM_print_ext("\n\n====================================================");
     ECM_print_ext("============================USAGE====================");
     ECM_print_ext("=====================================================");
     ECM_print_ext("ECM_DEMO -t (up|down|set|status|hotplug|simstatus|autoconfig) [-a APN] [-p DEV_NODE] [-L LOG_LEVEL]");

     ECM_print_ext(" @-t (up|down|set|status); call up, or call down, or set");
     ECM_print_ext(" @-t up: Module call up operation");
     ECM_print_ext(" @-t down: Module call down operation");
     ECM_print_ext(" @-t set: Module set L+E mode");
     ECM_print_ext(" @-t status: Query Module net status");
     ECM_print_ext(" @-t hotplug: set Module hotplug function");
     ECM_print_ext(" @-t simstatus: get Module sim card status");
     ECM_print_ext(" @-t personal: Send Personalization AT");
     ECM_print_ext(" @-t autoconfig: get ECM_DEMO_AUTO run status");

     ECM_print_ext(" @-a APN: user can set apn, no more than 63 characters");

     ECM_print_ext(" @-p DEV_NODE:user select AT port or Modem port, path no more than 63 characters");

     ECM_print_ext(" @-L LOG_LEVEL:user can set log level, from \'1\' to \'4\'. ");

     ECM_print_ext("Example:");
     ECM_print_ext("   Example: ECM_DEMO -t up -a 3gnet -p /dev/ttyUSB2 -L 3");
     ECM_print_ext("   Example: ECM_DEMO -t down -p /dev/ttyUSB0 -L 3");
     ECM_print_ext("   Example: ECM_DEMO -t personal -D AT+CPIN?");
     ECM_print_ext("   Example: ECM_DEMO -t autoconfig");
     ECM_print_ext("=====================================================\n\n");

     return 0;
}


unsigned int ECM_parse_parameter(int argc, char *argv[], 
    ECM_USR_OPS_T* op, char** apn,char** path,unsigned int* log_level, char** personal_at)
{
    unsigned int          error = 0 ;
    ECM_USR_OPS_T         var_usr_op = ECM_OP_QSTATUS;
    char*                 var_type=NULL;
    char*                 var_apn=NULL;
    char*                 var_path=NULL;
    char*                 var_log_level=NULL;
    char*                 var_person_at = NULL;
    int                   var_opt = 1;

    if (!strcmp(argv[argc-1], "&"))
    {
        argc--;
    }

    var_opt = 1;

    while  (var_opt < argc)
    {
        if (argv[var_opt][0] != '-')
        {
            error = 1;
            break;
        }
        if ('t' == argv[var_opt][1])
        {
            var_opt++;
            if (var_opt >= argc)
            {
                error = 2;
                break;
            }
            if (argv[var_opt][0] == '-')
            {
                error = 3;
                break;
            }
            var_type = argv[var_opt++];
            
            if (!strcmp(var_type,"up"))
            {
                var_usr_op = ECM_OP_UP;
            }
            else if (!strcmp(var_type,"down"))
            {
                var_usr_op = ECM_OP_DOWN;
            }
            else if (!strcmp(var_type,"set"))
            {
                var_usr_op = ECM_OP_CONFIG;
            }
            else if (!strcmp(var_type,"status"))
            {
                var_usr_op = ECM_OP_QSTATUS;
            }
            else if (!strcmp(var_type,"hotplug"))
            {
                var_usr_op = ECM_OP_CFG_HOT_SIM;
            }
            else if (!strcmp(var_type,"simstatus"))
            {
                var_usr_op = ECM_OP_Q_SIM_STATUS;
            }
            else if (!strcmp(var_type,"personal"))
            {
                var_usr_op = ECM_OP_PERSON_AT;
            }
            else if (!strcmp(var_type,"autoconfig"))
            {
                var_usr_op = ECM_OP_Q_AUTOCFG;
            }
            else
            {
                error = 4;
                break;
            }
        }
        else if ('a'==argv[var_opt][1])
        {
            var_opt++;
            if (var_opt >= argc)
            {
                error = 5;
                break;
            }

            if (argv[var_opt][0] == '-')
            {
                error = 6;
                break;
            }

            var_apn = argv[var_opt++];
            if (ECM_APN_MAX_LEN<=strlen(var_apn))
            {
                error = 7;
                break;
            }            
        }
        else if ('p'==argv[var_opt][1])
        {
            var_opt++;
            if (var_opt >= argc)
            {
                error = 8;
                break;
            }

            if (argv[var_opt][0] == '-')
            {
                error = 9;
                break;
            }

            var_path = argv[var_opt++];
            if (TTYUSB_PATH_LEN<=strlen(var_path))
            {
                error = 10;
                break;
            }
            //memcpy((void*)path_save,(void*)path,strlen(path));
        }
        else if (('L'==argv[var_opt][1])||('l'==argv[var_opt][1]))
        {
            var_opt++;
            if (var_opt >= argc)
            {
                error = 11;
                break;
            }

            if (argv[var_opt][0] == '-')
            {
                error = 12;
                break;
            }

            var_log_level = argv[var_opt++];
            if (1 != strlen(var_log_level))
            {
                error = 13;
                break;
            }

            if (('1' != *var_log_level) && ('2' != *var_log_level) 
                && ('3' != *var_log_level) && ('4' != *var_log_level))
            {
                error = 14;
                break;
            }            
        }
        else if (('D'==argv[var_opt][1])||('d'==argv[var_opt][1]))
        {
            var_opt++;
            if (var_opt >= argc)
            {
                error = 15;
                break;
            }

            if (argv[var_opt][0] == '-')
            {
                error = 16;
                break;
            }

            var_person_at = argv[var_opt++];
            if (strlen(var_person_at)>(ECM_PERSONAL_AT_LEN-8))
            {
                error = 17;
                break;
            }   
        }

        else
        {
            error = 19;
            break;
        }
    }


    if (0==error)
    {
        if (NULL != op)
        {
            *op = var_usr_op;
        }

        if (NULL != apn)
        {
            if (NULL != var_apn)
            {
                *apn = var_apn;
            }
        }

        if (NULL != path)
        {
            if (NULL != var_path)
            {
                *path = var_path;
            }
        }

        if (NULL != log_level)
        {
            if (NULL==var_log_level)
            {
                *log_level = ECM_LOG_L_1 ;
            }
            else if ('1' == *var_log_level)
            {
                *log_level = ECM_LOG_L_1 ;
            }
            else if ('2' == *var_log_level)
            {
                *log_level = ECM_LOG_L_2 ;
            }
            else if ('3' == *var_log_level)
            {
                *log_level = ECM_LOG_L_3 ;
            }
            else if ('4' == *var_log_level)
            {
                *log_level = ECM_LOG_L_4 ;
            }
            else
            {
                *log_level = ECM_LOG_L_1 ;
            }
        }   
       
        if (NULL != personal_at)
        {
            if (NULL != var_person_at)
            {
                *personal_at = var_person_at;
            }
        }

    }

    return error ;
}


void  ECM_display_autocfg(ECM_auto_monitor_t* ptr_autocfg)
{
    if (NULL != ptr_autocfg)
    {
        ECM_print("ECM_MODULE_AUTOCFG:SUCCESS");

        ptr_autocfg->imei[ECM_AUTOCFG_IMEI_LEN-1]='\0';
        ECM_print_ext("IMEI=%s",                   ptr_autocfg->imei);

        ptr_autocfg->iccid[ECM_AUTOCFG_ICCID_LEN-1]='\0';
        ECM_print_ext("ICCID=%s",                  ptr_autocfg->iccid);

        ptr_autocfg->sim_status[ECM_AUTOCFG_SIM_STATUS_LEN-1]='\0';
        ECM_print_ext("SIM_STATUS=%s",             ptr_autocfg->sim_status);

        ECM_print_ext("CONNECT_STATUS=%01d",       ptr_autocfg->connet_status);

        ECM_print_ext("RETRY_TIMES=%01d",          ptr_autocfg->retry_times);

        ECM_print_ext("ERROR_CODE=%01d",           ptr_autocfg->error_code);

        ptr_autocfg->error_info[ECM_AUTOCFG_ERR_INFO_LEN-1] = '\0';
        ECM_print_ext("ERROR_INFO=%s",             ptr_autocfg->error_info);

        ptr_autocfg->error_ext_info[ECM_AUTOCFG_ERR_INFO_LEN-1] = '\0';
        ECM_print_ext("ERROR_EXT_INFO=%s",         ptr_autocfg->error_ext_info);

        ptr_autocfg->net_type_str[ECM_NET_TYPE_STR_LEN-1] = '\0';
        ECM_print_ext("NET_TYPE=%s",               ptr_autocfg->net_type_str);

        ECM_print_ext("RCV_STRENGTH=%01d",         ptr_autocfg->net_recv_strength);

        if ((0 <= ptr_autocfg->rf_recv_strength)&&(ptr_autocfg->rf_recv_strength < 10))
        {
            ECM_print_ext("RF_STRENGTH=LOW");
        }
        else if ((10 <= ptr_autocfg->rf_recv_strength)&&(ptr_autocfg->rf_recv_strength < 20))
        {
            ECM_print_ext("RF_STRENGTH=MIDDLE");
        }
        else if ((20 <= ptr_autocfg->rf_recv_strength)&&(ptr_autocfg->rf_recv_strength <= 31))
        {
            ECM_print_ext("RF_STRENGTH=HIGH");
        }
        else
        {
            ECM_print_ext("RF_STRENGTH=LOW");
        }
        ECM_print_ext("ECM_VERSION=%s,DATE=%s",      ECM_CALL_VERSION, ECM_CALL_DATE);

    }
}


void  ECM_display_dft_autocfg(ECM_auto_monitor_t* ptr_autocfg)
{
    if (NULL != ptr_autocfg)
    {
        ECM_print("ECM_MODULE_AUTOCFG:DEFAULT");
        ECM_print_ext("IMEI=");
        ECM_print_ext("ICCID=");
        ECM_print_ext("SIM_STATUS=");
        ECM_print_ext("CONNECT_STATUS=");
        ECM_print_ext("RETRY_TIMES=");
        ECM_print_ext("ERROR_CODE=");
        ECM_print_ext("ERROR_INFO=");
        ECM_print_ext("ERROR_EXT_INFO=");
        ECM_print_ext("NET_TYPE=");
        ECM_print_ext("RCV_STRENGTH=");
        ECM_print_ext("RF_STRENGTH=");
        ECM_print_ext("ECM_VERSION=%s,DATE=%s",      ECM_CALL_VERSION, ECM_CALL_DATE);
    }
}


int main(int argc, char *argv[])
{
    char*                  apn=NULL;
    char*                  path=NULL;
    char*                  personal_at=NULL;
    unsigned int           log_level;
    ECM_USR_OPS_T          usr_op ;
    char                   error_buffer[ECM_AUTOCFG_ERR_INFO_LEN] = {0};

    unsigned int           error = 0;
    ECM_auto_monitor_t     autocfg ;

    do {

        if (0 != ECM_parse_parameter(argc, argv, &usr_op, &apn, &path, &log_level, &personal_at))
        {
            (void)ECM_usage();
            error = E_ECM_CALL_PARA_UNPACK_FAIL ;
            break;
        }

        ECM_log_init(ECM_print,log_level);

        if (ECM_OP_Q_AUTOCFG == usr_op)
        {
            if (0 == ECM_autocfg_usr_read(&autocfg))
            {
                ECM_display_autocfg(&autocfg);
            }
            else
            {
                ECM_display_dft_autocfg(&autocfg);
            }
        }
        else if (ECM_OP_PERSON_AT == usr_op)
        {
            ECM_set_personalization_at(personal_at);
            error = ECM_call(path,usr_op,NULL,apn);
        }
        else
        {            
            error = ECM_call(path,usr_op,NULL,apn);
            if (0 != error)
            {
                ECM_error_map(error,error_buffer,NULL);
                error_buffer[ECM_AUTOCFG_ERR_INFO_LEN-1]='\0';
                ECM_print("ECM_DEMO %s",error_buffer);
            }
            else
            {
                ECM_print("ECM_DEMO success");
            }
        }
        
    }while(0);

    return error ;

}

