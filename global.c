#include "global.h"

RT_TASK tbattery;
RT_TASK tcalibrer;
RT_TASK tcommuniquer;
RT_TASK tconnect;
RT_TASK tdeplacer;
RT_TASK tenvoyer;
RT_TASK tlocaliser;
RT_TASK twatchdog;

RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;

RT_SEM semConnecterRobot;
RT_SEM semWatchdog;
RT_SEM semBatterie;
RT_SEM semDetectArena;

RT_QUEUE queueMsgGUI;

int etatCommMoniteur = 1;
int etatCommRobot = 1;
DRobot *robot;
DMovement *move;
DServer *serveur;


int MSG_QUEUE_SIZE = 10;

int PRIORITY_BATTERY = 5;
int PRIORITY_CALIBRER = 25;
int PRIORITY_TCOMMUNIQUER = 50;
int PRIORITY_TCONNECT = 35;
int PRIORITY_TDEPLACER = 20;
int PRIORITY_TENVOYER = 40;
int PRIORITY_LOCALISER = 15;
int PRIORITY_WATCHDOG = 30;