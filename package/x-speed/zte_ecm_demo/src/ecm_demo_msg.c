
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>

#include "ecm_demo_config.h"
#include "ecm_demo_msg.h"
#include "ecm_demo_ttydev.h"

pthread_mutex_t itc_msg_mux         =  PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  itc_msg_cond = PTHREAD_COND_INITIALIZER;

unsigned int itc_msg_timer                 = 0;
unsigned int itc_msg_terminal = 0;
unsigned int itc_msg_quit                  = 0;
unsigned int itc_msg_reboot                = 0;
unsigned int itc_msg_atcmd                 = 0;


extern  char    itc_tty_at_buff[TTYUSB_BUF_LEN];
extern  int     itc_tty_at_len;


void ITC_send_atcmd_msg(char* data, int data_len)
{
    if ((data)&&(0<data_len)&&(data_len<TTYUSB_BUF_LEN))
    {
        pthread_mutex_lock(&itc_msg_mux);
        pthread_cond_signal(&itc_msg_cond);
        bzero(itc_tty_at_buff,TTYUSB_BUF_LEN);
        memcpy((void*)itc_tty_at_buff,(void*)data,data_len);
        itc_tty_at_len = data_len ;
        itc_msg_atcmd = 1 ;
        pthread_mutex_unlock(&itc_msg_mux);
    }
}

void ITC_send_terminal_msg(void)
{
    pthread_mutex_lock(&itc_msg_mux);
    itc_msg_terminal = 1 ;
    pthread_cond_signal(&itc_msg_cond);    
    pthread_mutex_unlock(&itc_msg_mux); 
}

void ITC_send_quit_msg(void)
{
    pthread_mutex_lock(&itc_msg_mux);
    itc_msg_quit = 1 ;
    pthread_cond_signal(&itc_msg_cond);
    pthread_mutex_unlock(&itc_msg_mux); 
}


void ITC_send_reboot_msg(void)
{
    pthread_mutex_lock(&itc_msg_mux);
    itc_msg_reboot = 1 ;
    pthread_cond_signal(&itc_msg_cond);
    pthread_mutex_unlock(&itc_msg_mux); 
}


void ITC_send_timeout_msg(void)
{
    pthread_mutex_lock(&itc_msg_mux);
    itc_msg_timer = 1 ;
    pthread_cond_signal(&itc_msg_cond);    
    pthread_mutex_unlock(&itc_msg_mux); 
}



void ITC_set_timer(struct itimerval* timer, int triger_secs,int interv_secs)
{
    timer->it_value.tv_sec = triger_secs;/*first triger*/
    timer->it_value.tv_usec = 0;
    timer->it_interval.tv_sec = interv_secs;/*every triger*/
    timer->it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, timer, NULL);    
}

void  ITC_clear_timer(struct itimerval* timer)
{
    timer->it_value.tv_sec = 0;/*first triger*/
    timer->it_value.tv_usec = 0;
    timer->it_interval.tv_sec = 0;/*every triger*/
    timer->it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, timer, NULL);
}

void ITC_signal_routine(int signo)
{   
    if (signo==SIGALRM)
    {
        ITC_send_timeout_msg();
    }
    else if (signo==SIGTERM)
    {
        ITC_send_terminal_msg();
    }    
}

void ITC_signal_init(void)
{
    //signal(SIGINT,SIG_IGN);
    signal(SIGALRM,ITC_signal_routine);
}


