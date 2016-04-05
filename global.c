/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

RT_TASK tcommuniquer;
RT_TASK tconnect;
RT_TASK tdeplacer;
RT_TASK tbattery;
RT_TASK tenvoyer;
RT_TASK twatchdog;

RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;

RT_SEM semConnecterRobot;

RT_QUEUE queueMsgGUI;

int etatCommMoniteur = 1;
int etatCommRobot = 1;
DRobot *robot;
DMovement *move;
DServer *serveur;


int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 30;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 25;

//liuzp add start
int PRIORITY_BATTERY = 5;
int PRIORITY_WATCHDOG = 30;
int PRIORITY_LOCALISER = 15;
int PRIORITY_CALIBRER = 25;
//liuzp add end 
