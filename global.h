#ifndef GLOBAL_H
#define	GLOBAL_H

#define HALF_SECOND 500000000  /* value in nanoseconds */
#define QUARTER_SECOND 250000000 
#define PERIOD_LOCALISER 600000000 

#include "includes.h"

/* @descripteurs des tâches */
extern RT_TASK tcommuniquer;
extern RT_TASK tconnect;
extern RT_TASK tdeplacer;
extern RT_TASK tenvoyer;
extern RT_TASK tbattery;
extern RT_TASK twatchdog;
extern RT_TASK tlocaliser;
extern RT_TASK tcalibrer;



/* @descripteurs des mutex */
extern RT_MUTEX mutexEtat;
extern RT_MUTEX mutexMove;
extern RT_MUTEX mutexCamera;
extern RT_MUTEX mutexImage;
extern RT_MUTEX mutexPosition;
extern RT_MUTEX mutexArena;
extern RT_MUTEX mutexCalibration;

/* @descripteurs des sempahore */
extern RT_SEM semDetectArena; // ACTION_FIND_ARENA
extern RT_SEM semConnecterRobot;
extern RT_SEM semWatchdog;
extern RT_SEM semBatterie;
extern RT_SEM semLocalisation;

/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;

/* @variables partagées */
extern int etatCommMoniteur;
extern int etatCommRobot;
extern int etatLocalisation;
extern char calibration;
extern DServer *serveur;
extern DRobot *robot;
extern DMovement *move;
extern DImage *image;
extern DArena *arena;

/* @constantes */
extern int MSG_QUEUE_SIZE;
extern int PRIORITY_TCOMMUNIQUER;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TDEPLACER;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TWATCHDOG;

//liuzp add start 
extern int PRIORITY_BATTERY;
extern int PRIORITY_WATCHDOG;
extern int PRIORITY_LOCALISER;
extern int PRIORITY_CALIBRER;
//liuzp add end

#endif	/* GLOBAL_H */

