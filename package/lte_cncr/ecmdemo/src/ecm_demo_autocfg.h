/* ============================================================================
 * @file: ecm_demo_autocfg.h
 * @author: Wei,LI
 * @Copyright:                      GoSUNCN
* @Website:                         www.ztewelink.com
* @Email:                           ztewelink@zte.com.cn
* @version:                         "ECM_CALLV1.0.1B04"
* @date:                            "2019-03-11"
* ============================================================================*/

#include <sys/time.h>
#include <time.h>

#ifndef __ECM_DEMO_AUTOCFG__H
#define __ECM_DEMO_AUTOCFG__H

typedef int ECM_ATOMIC_T;

#define ECM_AUTO_CFG_SHM_SIZE 4096



typedef enum
{
    SIM_NOT_INSERT=0,
    SIM_FAILURE=1,
    SIM_PIN=2,
    SIM_PUK=3,
    SIM_BUSY=4,
    SIM_ERROR=5,

} ECM_AUTO_CFG_SIM_STATUS_T;


typedef enum
{
    ECM_NET_NOT_CONNECT=0,
    ECM_NET_CONNECTED=1,
} ECM_AUTO_CFG_NET_STATUS_T;


/*
*
* server interface definition
*
*/

/*
sharememory definition
*/
int               ECM_autocfg_srv_new_shm(void);

void*             ECM_autocfg_srv_map_shm(int shmid);

int               ECM_autocfg_srv_rm_map(ECM_auto_monitor_t* addr);

int               ECM_autocfg_srv_rm_shm(int shmid);

void              ECM_autocfg_srv_read_from(ECM_auto_monitor_t* from, ECM_auto_monitor_t* to);

void              ECM_autocfg_srv_set_back(ECM_auto_monitor_t* back, ECM_auto_monitor_t* from);

/*
semphore definition
*/
int               ECM_autocfg_srv_new_sem(void);

int               ECM_autocfg_srv_sem_cfg(int semid);

void              ECM_autocfg_srv_sem_lock(int semid);

void              ECM_autocfg_srv_sem_unlock(int semid);

int               ECM_autocfg_srv_rm_sem(int semid);


/*
*
* user or client interface definition
*
*/

/*
sharememory definition
*/
int               ECM_autocfg_usr_new_shm(void);

void*             ECM_autocfg_usr_map_shm(int shmid);

int               ECM_autocfg_usr_rm_map(ECM_auto_monitor_t* addr);

void              ECM_autocfg_copy_to_usr(ECM_auto_monitor_t* from, ECM_auto_monitor_t* to);

/*
semphore definition
*/
int               ECM_autocfg_usr_new_sem(void);

void              ECM_autocfg_usr_sem_lock(int semid);

void              ECM_autocfg_usr_sem_unlock(int semid);

unsigned int      ECM_autocfg_usr_read(ECM_auto_monitor_t* ptr_autocfg);

#endif




