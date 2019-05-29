
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
//#include <error.h>
#include <errno.h>

#include "ecm_demo_config.h"
#include "ecm_demo_atctl.h"
#include "ecm_demo_ttydev.h"
#include "ecm_demo_msg.h"
#include "ecm_demo_autocfg.h"

#define ECM_AUTOCFG_SHM_PATH "."
#define ECM_AUTOCFG_SHM_ID 0x213


#define ECM_AUTOCFG_SEM_PATH "."
#define ECM_AUTOCFG_SEM_ID 0x214
#define ECM_AUTOCFG_SEM_NUM 1


union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};


static int ECM_shm_get(int flags)
{
    int   size ;
	key_t key ;
    int   shmid = -5 ;

    size = sizeof(ECM_auto_monitor_t);
    key = ftok(ECM_AUTOCFG_SHM_PATH, ECM_AUTOCFG_SHM_ID);
    
	if (key < 0)
	{		
        ECM_log(ECM_LOG_L_1,"[ERROR] ECM_shm_get ftok:%d",key);
		return -3;
	}

	shmid = shmget(key,size,flags);

    if (shmid < 0)
	{
        ECM_log(ECM_LOG_L_1,"[ERROR] ECM_shm_get shmget");
		return -2;
	}

	return shmid;

}

static int ECM_shm_del(int shmid)
{
    if (shmid<0)
    {
        ECM_log(ECM_LOG_L_1,"[ERROR] ECM_shm_del shmid<0");
        return -1 ;
    }
	if(shmctl(shmid,IPC_RMID,NULL) < 0)
	{
        ECM_log(ECM_LOG_L_1,"[ERROR] ECM_shm_del rm shm fail");
		return -2;
	}
	return 0;
}

/*
    server share memory interface
*/
int ECM_autocfg_srv_new_shm(void)
{
    return ECM_shm_get(IPC_CREAT|0666);//|IPC_EXCL
}

void* ECM_autocfg_srv_map_shm(int shmid)
{
    if (shmid<0)
    {
        ECM_log(ECM_LOG_L_1,"[error] ECM_autocfg_srv_map_shm");
        return NULL;
    }
    else
    {
        return shmat(shmid,NULL,0);
    }
}

int ECM_autocfg_srv_rm_map(ECM_auto_monitor_t* addr)
{
    if (NULL!=addr)
    {
        return shmdt(addr);
    }
    else
    {
        ECM_log(ECM_LOG_L_1,"[error] ECM_autocfg_srv_rm_map error");
        return -1 ;
    }
}

int ECM_autocfg_srv_rm_shm(int shmid)
{
    return ECM_shm_del(shmid);
}


void ECM_autocfg_srv_read_from(ECM_auto_monitor_t* from, ECM_auto_monitor_t* to)
{
    if ((NULL!=from)&&(NULL!=to))
    {
        memcpy((void*)to,(void*)from,sizeof(ECM_auto_monitor_t));
    }
}

void ECM_autocfg_srv_set_back(ECM_auto_monitor_t* back, ECM_auto_monitor_t* from)
{
    if ((NULL!=from)&&(NULL!=back))
    {
        memcpy((void*)back,(void*)from,sizeof(ECM_auto_monitor_t));
    }
}


/*
    usr/client share memory interface
*/

int ECM_autocfg_usr_new_shm(void)
{
    //return ECM_shm_get(IPC_CREAT);
    //return ECM_shm_get(IPC_CREAT|0666);
    return ECM_shm_get(0666);
}

void* ECM_autocfg_usr_map_shm(int shmid)
{
    if (shmid<0)
    {
        ECM_log(ECM_LOG_L_1,"[error] ECM_autocfg_usr_map_shm");
        return NULL;
    }
    else
    {
        return shmat(shmid,NULL,0);
    }
}

int ECM_autocfg_usr_rm_map(ECM_auto_monitor_t* addr)
{
    if (NULL!=addr)
    {
        return shmdt(addr);
    }
    else
    {
        ECM_log(ECM_LOG_L_1,"[error] ECM_autocfg_usr_rm_map EMPTY");
        return -1 ;
    }
}

void ECM_autocfg_copy_to_usr(ECM_auto_monitor_t* from, ECM_auto_monitor_t* to)
{
    if ( (NULL!=from) && (NULL!=to) )
    {
        memcpy( (void*)to, (void*)from, sizeof(ECM_auto_monitor_t) );
    }
}

/*
*   Semphore
*
*/
static int ECM_sem_get(int semflg)
{
    key_t key ;
    int semid=-5;
      
    key = ftok(ECM_AUTOCFG_SEM_PATH, ECM_AUTOCFG_SEM_ID);

	if (key < 0)
	{		
        ECM_log(ECM_LOG_L_1,"[ERROR] ECM_sem_get ftok:%d",key);
		return -3;
	}

    semid = semget(key, ECM_AUTOCFG_SEM_NUM, semflg);

    if (semid<0)
    {
       //ECM_log(ECM_LOG_L_1,"ECM_sem_get error: %s. \n", sys_errlist[errno]);
       ECM_log(ECM_LOG_L_1,"ECM_sem_get error: %d. \n", errno);
       return -2;
    }

    return semid;
}


static int ECM_sem_del(int semid)
{
    if (semid<0)
    {
        ECM_log(ECM_LOG_L_1,"[ERROR] ECM_sem_del shmid<0");
        return -1 ;
    }

    if ((semctl(semid, 0, IPC_RMID, 0)) < 0)
	{
        ECM_log(ECM_LOG_L_1,"[ERROR] ECM_sem_del delete fail");
		return -2;
	}
	return 0;

    
}


static int ECM_sem_ctl(int semid, int semnum, int cmd, union semun arg)
{
    int retval;

    retval = semctl(semid, semnum, cmd, arg);

    if (retval<0)
    {
       //printf("ECM_sem_ctl error%01d",retval);
       //ECM_log(ECM_LOG_L_1,"semctl error: %s. \n", sys_errlist[errno]);
       ECM_log(ECM_LOG_L_1,"semctl error: %d. \n", errno);
    }
    return retval;
}



static int ECM_sem_op(int semid, struct sembuf *sops, unsigned nsops)
{
    int retval;

    if (semid<0)
    {
        retval = -2 ;
        ECM_log(ECM_LOG_L_1,"ECM_sem_op semid error");
    }
    else
    {
        retval = semop(semid, sops, nsops);

        if (-1 == retval)
        {       
           //ECM_log(ECM_LOG_L_1,"ECM_sem_op error%01d",retval);
           //ECM_log(ECM_LOG_L_1,"semop error: %s. \n", sys_errlist[errno]);
           ECM_log(ECM_LOG_L_1,"semop error: %s. \n", errno);
        }    
    }

    return retval;
}


int ECM_autocfg_srv_new_sem(void)
{
    return ECM_sem_get(IPC_CREAT|0666);//|IPC_EXCL
}


int ECM_autocfg_srv_sem_cfg(int semid)
{
    union semun sunion;
    sunion.val=1;
    return ECM_sem_ctl(semid, 0, SETVAL, sunion);
}


void ECM_autocfg_srv_sem_lock(int semid)
{
    struct sembuf sb;

    sb.sem_num = 0;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO;
    ECM_sem_op(semid, &sb, 1);
}

void ECM_autocfg_srv_sem_unlock(int semid)
{
    struct sembuf sb;

    sb.sem_num = 0;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;
    ECM_sem_op(semid, &sb, 1);
}




int ECM_autocfg_usr_new_sem(void)
{
    return ECM_sem_get(0666);//IPC_CREAT);
}



void ECM_autocfg_usr_sem_lock(int semid)
{
    struct sembuf sb;

    sb.sem_num = 0;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO;
    ECM_sem_op(semid, &sb, 1);
}

void ECM_autocfg_usr_sem_unlock(int semid)
{
    struct sembuf sb;

    sb.sem_num = 0;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;
    ECM_sem_op(semid, &sb, 1);
}

int ECM_autocfg_srv_rm_sem(int semid)
{
    return ECM_sem_del(semid);
}


unsigned int    ECM_autocfg_usr_read(ECM_auto_monitor_t* ptr_autocfg)
{
    unsigned int           error = 0 ;
    int                    sem_id = -1 ;
    int                    shm_id = -1 ;
    ECM_auto_monitor_t*    ptr_shm_autocfg      = NULL ;

    do {
        if (NULL == ptr_autocfg)
        {
            error = 1 ;
            break;
        }

        bzero( (void*)ptr_autocfg, sizeof(ECM_auto_monitor_t) );

        /*share memory init*/
        shm_id   = ECM_autocfg_usr_new_shm();
      

        if (shm_id<0)
        {
            error = 2 ;
            break;
        }

        /*share memory map*/
        ptr_shm_autocfg = (ECM_auto_monitor_t*)ECM_autocfg_usr_map_shm(shm_id);
        if (NULL == ptr_shm_autocfg)
        {
            error = 3 ;
            break;
        }

        /*semphore init*/
        sem_id = ECM_autocfg_usr_new_sem();
        if (sem_id<0)
        {
            error = 4 ;
            break;
        }

         /*lock share memory and read*/
        ECM_autocfg_usr_sem_lock(sem_id);

        /*share memory read*/
        ECM_autocfg_copy_to_usr(ptr_shm_autocfg,ptr_autocfg);

        /*release share memory lock*/
        ECM_autocfg_usr_sem_unlock(sem_id);

        /*release share memory map*/
        ECM_autocfg_usr_rm_map(ptr_shm_autocfg);

    } while(0);

    if (0 != error)
    {
        ECM_log(ECM_LOG_L_1,"ECM_autocfg_usr_read err=%d", error);
    }

    return error;

}


