/* 
 * File:   global.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:11
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#define ONE_SECOND 1000000
#define QUARTER_SECOND 250000

#include "includes.h"

/* @descripteurs des tâches */
extern RT_TASK tcommuniquer;
extern RT_TASK tconnect;
extern RT_TASK tdeplacer;
extern RT_TASK tenvoyer;

//liuzp add start
extern RT_TASK tbattery;
extern RT_TASK twatchdog;
extern RT_TASK tlocaliser;
extern RT_TASK tcalibrer;
//liuzp add end 


/* @descripteurs des mutex */
extern RT_MUTEX mutexEtat;
extern RT_MUTEX mutexMove;

/* @descripteurs des sempahore */
extern RT_SEM semDetectArena;			// ACTION_FIND_ARENA
extern RT_SEM semConnecterRobot;
extern RT_SEM semWatchdog;
extern RT_SEM semCamera;

/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;

/* @variables partagées */
extern int etatCommMoniteur;
extern int etatCommRobot;
extern DServer *serveur;
extern DRobot *robot;
extern DMovement *move;

/* @constantes */
extern int MSG_QUEUE_SIZE;
extern int PRIORITY_TSERVEUR;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TWATCHDOG; // todo

//liuzp add start 
extern int PRIORITY_BATTERY;
extern int PRIORITY_WATCHDOG;
extern int PRIORITY_LOCALISER;
extern int PRIORITY_CALIBRER;
//liuzp add end

#endif	/* GLOBAL_H */

