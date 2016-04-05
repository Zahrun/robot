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
RT_MUTEX mutexCamera;
RT_MUTEX mutexPosition;

RT_SEM semConnecterRobot;
RT_SEM semWatchdog;
RT_SEM semBatterie;
RT_SEM semDetectArena;
RT_SEM semLocalisation;

RT_QUEUE queueMsgGUI;

int etatCommMoniteur = 1;
int etatCommRobot = 1;
int etatLocalisation = ACTION_STOP_COMPUTE_POSITION; // par défaut le thread tlocaliser ne calcul pas la position!
DPosition *position;
DRobot *robot;
DMovement *move;
DServer *serveur;


int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 30;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 25;
int PRIORITY_BATTERY = 5;
int PRIORITY_WATCHDOG = 30;
int PRIORITY_LOCALISER = 15;
int PRIORITY_CALIBRER = 25;
