
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ecm_demo_atctl.h"
#include "ecm_demo_autocfg.h"

#define ECM_DEMO_DBG_FLAG                      ECM_DEMO_ON

typedef enum
{
    ECM_DEMO_AUTO_CALL_INIT=0,
    ECM_DEMO_AUTO_WAIT_CALL_UP,
    ECM_DEMO_AUTO_STATUS_CONNECT,
    ECM_DEMO_AUTO_STATUS_DISCONN,

} ECM_AUTO_RECORED_T;


void ECM_print(const char* msg, ...)
{
#if (ECM_DEMO_DBG_FLAG==ECM_DEMO_ON)
    char    buf[TTYUSB_BUF_LEN+64];
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

void ECM_usage(void)
{
     ECM_print_ext("\n\n====================================================");
     ECM_print_ext("============================USAGE====================");
     ECM_print_ext("=====================================================");
     
     ECM_print_ext("ECM_DEMO_AUTO [-a APN] [-p DEV_NODE] [-L DBG_LEVEL]");

     ECM_print_ext("@-a APN; user can set apn,no more than 63 characters");
     ECM_print_ext("@APN: User could configure the APN when  call up...");

     ECM_print_ext("@-p DEV_NODE; user can set path,no more than 63 characters");
     ECM_print_ext("  Example: ECM_DEMO_AUTO -a 3gnet");

     ECM_print_ext("@-p DEV_NODE: User configure of the ttyUSB device path,");
     ECM_print_ext("  By default, we use/dev/ttyUSB1");
     
     ECM_print_ext("@-L DBG_LEVEL: User configure log level, from \'1\' to \'4\'.");
     ECM_print_ext("  By default, we use log level 3");

     ECM_print_ext("  Example: ECM_DEMO_AUTO  ");

     ECM_print_ext("  Example: ECM_DEMO_AUTO   -a 3gnet -p /dev/ttyUSB0");
     
     ECM_print_ext("  Example: ECM_DEMO_AUTO   -a 3gnet -p /dev/ttyUSB0 -L 1");

     ECM_print_ext("  Example: ECM_DEMO_AUTO   -L 2");

     ECM_print_ext("=====================================================\n\n");

}


unsigned int ECM_auto_parse_parameter(int argc, char *argv[], 
    char** apn, char** path, unsigned int* log_level)
{

    char*            var_apn=NULL;
    char*            var_path=NULL;
    char*            var_log_level=NULL;
    int              var_opt = 1;
    unsigned int     var_error = 0 ;


    /*para input parameters start */
    if (!strcmp(argv[argc-1], "&"))
    {
        argc--;
    }

    var_opt = 1;
    while  (var_opt < argc)
    {
        if (!argv[var_opt])
        {
            var_error = 1;
            break;
        }
        if (argv[var_opt][0] != '-')
        {
            var_error = 2;
            break;
        }
        if ('a'==argv[var_opt][1])
        {
            var_opt++;
            if (var_opt >= argc)
            {
                var_error = 3;
                break;
            }

            if (argv[var_opt][0] == '-')
            {
                var_error = 4;
                break;
            }

            var_apn = argv[var_opt++];
            if (ECM_APN_MAX_LEN<=strlen(var_apn))
            {
                var_error = 5;
                break;
            }
        }
        else if ('p'==argv[var_opt][1])
        {
            var_opt++;
            if (var_opt >= argc)
            {
                var_error = 6;
                break;
            }

            if (argv[var_opt][0] == '-')
            {
                var_error = 7;
                break;
            }

            var_path = argv[var_opt++];
            if (TTYUSB_PATH_LEN<=strlen(var_path))
            {
                var_error = 8;
                break;
            }

        }
        else if (('L'==argv[var_opt][1])||('l'==argv[var_opt][1]))
        {
            var_opt++;
            if (var_opt >= argc)
            {
                var_error = 9;
                break;
            }

            if (argv[var_opt][0] == '-')
            {
                var_error = 10;
                break;
            }

            var_log_level = argv[var_opt++];
            if (1 != strlen(var_log_level))
            {
                var_error = 11;
                break;
            }

            if (('1' != *var_log_level) && ('2' != *var_log_level) 
                && ('3' != *var_log_level) && ('4' != *var_log_level))
            {
                var_error = 12;
                break;
            }
        }
        else
        {
            var_error = 13;
            break;
        }

    }

    if (0 == var_error)
    {

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
    }

    return var_error ;
}


int ECM_auto_shm_id = -1 ;
int ECM_auto_sem_id = -1 ;
ECM_auto_monitor_t* ECM_auto_moniter = NULL ;


void ECM_sig_delete(int signum)
{
    ECM_print("ECM_sig_delete");
    exit(0);
}

void ECM_delete_on_exit()
{
    ECM_print("ECM_delete_on_exit");
    ECM_autocfg_srv_rm_map(ECM_auto_moniter);
    ECM_autocfg_srv_rm_shm(ECM_auto_shm_id);
    ECM_autocfg_srv_rm_sem(ECM_auto_sem_id);
}



int main(int argc, char *argv[])
{
    unsigned int var_usr_exit = 0 ;
    int             var_error = 0;
    char*           var_apn=NULL;
    char*           var_path=NULL;
    unsigned int var_log_level = 0 ;

    unsigned int var_demo_err_id = 0 ;

    int var_shm_id = -1 ;
    int var_sem_id = -1 ;

    ECM_auto_monitor_t* cur_auto_cfg = NULL ;
    ECM_auto_monitor_t    prev_monitor ;
    ECM_auto_monitor_t    crnt_monitor ;

    unsigned int var_sim_is_ready = 0;

    unsigned int var_not_init_flg = 0;

    atexit(&ECM_delete_on_exit);
    signal(SIGINT, &ECM_sig_delete);

    do {

        if (0 != ECM_auto_parse_parameter(argc, argv, &var_apn, &var_path, &var_log_level))
        {
            ECM_usage();
            var_error = E_ECM_CALL_PARA_UNPACK_FAIL ;
            break;
        }

        bzero((void*)&prev_monitor,sizeof(ECM_auto_monitor_t));

        bzero((void*)&crnt_monitor,sizeof(ECM_auto_monitor_t));

        ECM_log_init(ECM_print,var_log_level);
        
        /*init share memory*/
        var_shm_id = ECM_autocfg_srv_new_shm();
        ECM_auto_shm_id = var_shm_id ;

        /*map share memory*/
        cur_auto_cfg = (ECM_auto_monitor_t*)ECM_autocfg_srv_map_shm(var_shm_id);
        ECM_auto_moniter = cur_auto_cfg;

        /*init semphore*/
        var_sem_id =  ECM_autocfg_srv_new_sem();
        ECM_auto_sem_id = var_sem_id ;

        /*config semphore*/
        ECM_autocfg_srv_sem_cfg(var_sem_id);

        ECM_print("ECM_DEMO_AUTO VERSION:%s start...",ECM_CALL_VERSION);

        /*add liwei for ali project led operation*/
        #if (ECM_AUTO_LED_ON==ECM_DEMO_ON)
        ECM_auto_led_func_start();
        #endif


        /*add liwei for ali project signal led operation*/
        #if (ECM_AUTO_SIG_LEVEL_LED_ON==ECM_DEMO_ON)
        ECM_auto_siglevl_led_func_start();
        #endif

        do {
            /*add liwei for fix gswerr id 0000 start */
            #if (ECM_CALL_FIX_GSWERR_ID_0000    ==ECM_DEMO_ON)
                //ECM_get_gswerr_id_000_status();
                ECM_clr_gswerr_id_000_status();
            #endif
            /*add liwei for fix gswerr id 0000 end */

            var_demo_err_id = ECM_auto_demo_start(var_path, NULL, var_apn);

            if ( 0 != var_demo_err_id )
            {
                if (0==var_not_init_flg)
                {
                    ECM_print("ECM_DEMO_AUTO start fail(%d), will restart for every 5 seconds!!!",var_demo_err_id);
                    var_not_init_flg = 1 ;
                }
                sleep(5);
                continue;
            }
            var_not_init_flg = 0 ;

             while (0==ECM_auto_demo_online())
             {               
                ECM_auto_demo_get_monitor(&crnt_monitor);
                ECM_autocfg_srv_sem_lock(var_sem_id);
                ECM_autocfg_srv_set_back(cur_auto_cfg,&crnt_monitor);
                ECM_autocfg_srv_sem_unlock(var_sem_id);

                /*clear*/
                var_sim_is_ready = 0 ;

                /* display IMEI number */
                crnt_monitor.imei[ECM_AUTOCFG_IMEI_LEN-1]='\0';
                prev_monitor.imei[ECM_AUTOCFG_IMEI_LEN-1]='\0';
                if (0 != strcmp(crnt_monitor.imei, prev_monitor.imei))
                {
                    if (0 != strcmp(crnt_monitor.imei, ""))
                    {
                        ECM_print("ECM_DEMO_AUTO GET IMEI=[%s]",crnt_monitor.imei);
                    }
                    strcpy(prev_monitor.imei,crnt_monitor.imei);
                }

                /* display ICCID number */
                crnt_monitor.iccid[ECM_AUTOCFG_ICCID_LEN-1]='\0';
                prev_monitor.iccid[ECM_AUTOCFG_ICCID_LEN-1]='\0';
                if (0 != strcmp(crnt_monitor.iccid, prev_monitor.iccid))
                {
                    if (0 != strcmp(crnt_monitor.iccid, ""))
                    {
                       ECM_print("ECM_DEMO_AUTO GET ICCID=[%s]",crnt_monitor.iccid);
                    }
                    strcpy(prev_monitor.iccid,crnt_monitor.iccid);
                }

                /* display simstatus */
                crnt_monitor.sim_status[ECM_AUTOCFG_SIM_STATUS_LEN-1]='\0';
                prev_monitor.sim_status[ECM_AUTOCFG_SIM_STATUS_LEN-1]='\0';
                if (0 != strcmp(crnt_monitor.sim_status, prev_monitor.sim_status))
                {
                    if (0 != strcmp(crnt_monitor.sim_status, ""))
                    {
                        ECM_print("ECM_DEMO_AUTO GET SIM_STATUS=[%s]",crnt_monitor.sim_status);
                    }
                    strcpy(prev_monitor.sim_status,crnt_monitor.sim_status);
                }

                if (0 == strcmp(crnt_monitor.sim_status,"SIM_READY"))
                {
                    var_sim_is_ready = 1 ;
                }

                /* get connect status */
                if (crnt_monitor.connet_status != prev_monitor.connet_status)
                {
                    if (1==crnt_monitor.connet_status)
                    {
                        crnt_monitor.net_type_str[ECM_NET_TYPE_STR_LEN-1]='\0';
                        ECM_print("ECM_DEMO_AUTO Network Connected,NetType[%s],SignalStrength[%01d]",
                            crnt_monitor.net_type_str,                     /* get NetworkType */
                            crnt_monitor.net_recv_strength);               /* get Signal Strength */

                        strcpy(prev_monitor.net_type_str,crnt_monitor.net_type_str);

                        prev_monitor.net_recv_strength = crnt_monitor.net_recv_strength;
                    }
                    else
                    {
                            ECM_print("ECM_DEMO_AUTO Network Disconnect,RetryTimes[%d],ErrCode[%01d]",
                                                    crnt_monitor.retry_times,
                                                    crnt_monitor.error_code);
                            prev_monitor.retry_times = crnt_monitor.retry_times;
                            prev_monitor.error_code = crnt_monitor.error_code;
                            //ECM_print("ECM_DEMO_AUTO Network Disconnect,Sim is not ready");
                    }
                    prev_monitor.connet_status = crnt_monitor.connet_status;                    
                }
                else if (1==crnt_monitor.connet_status)/*connected*/
                {
                    /* display NetworkType */
                    crnt_monitor.net_type_str[ECM_NET_TYPE_STR_LEN-1]='\0';
                    prev_monitor.net_type_str[ECM_NET_TYPE_STR_LEN-1]='\0';
                    if (0 != strcmp(crnt_monitor.net_type_str, prev_monitor.net_type_str))
                    {
                        ECM_print("ECM_DEMO_AUTO NetworkType[%s]",crnt_monitor.net_type_str);
                        strcpy(prev_monitor.net_type_str,crnt_monitor.net_type_str);
                    }
                    /* display NetSignalStrength*/
                    if ((ECM_LOG_L_3==var_log_level) || (ECM_LOG_L_4==var_log_level))
                    {
                        if (prev_monitor.net_recv_strength != crnt_monitor.net_recv_strength)
                        {
                            prev_monitor.net_recv_strength = crnt_monitor.net_recv_strength ;
                            ECM_print("ECM_DEMO_AUTO SigStrength[%01d]",crnt_monitor.net_recv_strength);
                        }
                    }
                    else
                    {
                        if (prev_monitor.net_recv_strength != crnt_monitor.net_recv_strength)
                        {
                            prev_monitor.net_recv_strength = crnt_monitor.net_recv_strength ;
                            if (crnt_monitor.net_recv_strength>125)
                            {
                               ECM_print("ECM_DEMO_AUTO SigStrength[%01d]",crnt_monitor.net_recv_strength);
                            }
                        }
                    }

                    /*add liwei for fix gswerr id 0000 start */
                    #if (ECM_CALL_FIX_GSWERR_ID_0000    ==ECM_DEMO_ON)
                        if (0 != ECM_get_gswerr_id_000_status() )
                        {
                            ECM_print("ECM_DEMO_AUTO Find Egress Format Error will reboot~");
                            ECM_auto_demo_reboot_msg();
                            break;
                        }
                    #endif
                    /*add liwei for fix gswerr id 0000 end */
                }
                else if (0==crnt_monitor.connet_status)
                {/*disconnect*/
                    if (prev_monitor.error_code != crnt_monitor.error_code)
                    {
                        ECM_print("ECM_DEMO_AUTO RetryConnectErrCode[%01d]",crnt_monitor.error_code);
                        prev_monitor.error_code = crnt_monitor.error_code;
                    }

                    if (prev_monitor.retry_times != crnt_monitor.retry_times)
                    {

                        if ((ECM_LOG_L_3==var_log_level) || (ECM_LOG_L_4==var_log_level))
                        {
                            if (prev_monitor.retry_times <= 3)
                            {
                                ECM_print("ECM_DEMO_AUTO RetryTimes[%01d]",crnt_monitor.retry_times);
                            }
                        }
                        else
                        {
                            if (prev_monitor.retry_times == 1)
                            {
                                ECM_print("ECM_DEMO_AUTO     RetryConnect...");
                            }
                        }

                        prev_monitor.retry_times = crnt_monitor.retry_times;
                    }

                    /*more than 15 times retry connect fail, reboot the device*/
                    if (crnt_monitor.retry_times >= 15)
                    {

                        ECM_auto_demo_get_monitor(&crnt_monitor);
                        ECM_autocfg_srv_sem_lock(var_sem_id);
                        ECM_autocfg_srv_set_back(cur_auto_cfg,&crnt_monitor);
                        ECM_autocfg_srv_sem_unlock(var_sem_id);

                        ECM_auto_demo_reboot_msg();

                        ECM_print("ECM_DEMO_AUTO Retry 15 Times Fail, Reboot");
                        break;
                    }

                }
                sleep(2);
                
            };

            /*wait system recover timer*/
            ECM_print("ECM_DEMO_AUTO Closed, Recovery after %d seconds.\n\n",ECM_RECOVERY_TIME);
            sleep(ECM_RECOVERY_TIME);

        } while(0 == var_usr_exit);

        ECM_autocfg_srv_rm_map(cur_auto_cfg);
        ECM_autocfg_srv_rm_shm(var_shm_id);
        ECM_autocfg_srv_rm_sem(var_sem_id);

        ECM_print("ECM_DEMO_AUTO user    exit ~");

    }while(0);

    return var_error ;

}





