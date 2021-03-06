// TODO Attention aux notations
// utiliser les mêmes notations que si qu'on avait initialement 
// surtout avec les noms de threads / taches / semaphores

#include "includes.h"
#include "global.h"
#include "fonctions.h"

/**
 * \fn void innecter robot
tconnect : Ouverture de la communication avec le robot
tserver : Attente d'un message
Message{type:R size:0004 payload:0 0 0 2 }
tconnecter : Envoi message
tenvoyer : envoi d'un message au moniteur
tenvoyer : Attente d'un message
tconnect : Attente du sémarphore semConnecterRobot
tmove : Activation périodinitStruct(void)
 * \brief Initialisation de
    DJpegimage *jpeg;s structures de l'application (tâches, mutex, 
 * semaphore, etc.)
 */
void initStruct(void);

/**
 * \fn void startTasks(void)
 * \brief Démarrage des tâches
 */
void startTasks(void);

/**
 * \fn void deleteTasks(void)
 * \brief Arrêt des tâches
 */
void deleteTasks(void);

int main(int argc, char**argv) {
    printf("#################################\n");
    printf("#      DE STIJL PROJECT         #\n");
    printf("#################################\n");

    //signal(SIGTERM, catch_signal);
    //signal(SIGINT, catch_signal);

    /* Avoids memory swapping for this program */
    mlockall(MCL_CURRENT | MCL_FUTURE);
    /* For printing, please use rt_print_auto_init() and rt_printf () in rtdk.h
     * (The Real-Time printing library). rt_printf() is the same as printf()
     * except that it does not leave the primary mode, meaning that it is a
     * cheaper, faster version of printf() that avoids the use of system calls
     * and locks, instead using a local ring buffer per real-time thread along
     * with a process-based non-RT thread that periodically forwards the
     * contents to the output stream. main() must call rt_print_auto_init(1)
     * before any calls to rt_printf(). If you forget this part, you won't see
     * anything printed.
     */
    rt_print_auto_init(1);
    initStruct();
    DJpegimage *jpeg;
    startTasks();
    pause();
    deleteTasks();

    return 0;
}

void initStruct(void) {
    int err;
    /* Creation des mutex */
    if (err = rt_mutex_create(&mutexEtat, NULL)) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    DJpegimage *jpeg;
    if (err = rt_mutex_create(&mutexMove, NULL)) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (err = rt_mutex_create(&mutexCamera, NULL)) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (err = rt_mutex_create(&mutexImage, NULL)) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (err = rt_mutex_create(&mutexPosition, NULL)) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (err = rt_mutex_create(&mutexCalibration, NULL)) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (err = rt_mutex_create(&mutexArena, NULL)) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* Creation du semaphore */
    if (err = rt_sem_create(&semConnecterRobot, NULL, 0, S_FIFO)) {
        rt_printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (err = rt_sem_create(&semWatchdog, NULL, 0, S_FIFO)) {
        rt_printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (err = rt_sem_create(&semBatterie, NULL, 0, S_FIFO)) {
        rt_printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (err = rt_sem_create(&semDetectArena, NULL, 0, S_FIFO)) {
        rt_printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (err = rt_sem_create(&semLocalisation, NULL, 0, S_FIFO)) {
        rt_printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* Creation des taches */

    if (err = rt_task_create(&tbattery, NULL, 0, PRIORITY_BATTERY, 0)) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    
    if (err = rt_task_create(&tcalibrer, NULL, 0, PRIORITY_CALIBRER, 0)) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    
    if (err = rt_task_create(&tcommuniquer, NULL, 0, PRIORITY_TCOMMUNIQUER, 0)) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&tconnect, NULL, 0, PRIORITY_TCONNECT, 0)) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&tdeplacer, NULL, 0, PRIORITY_TDEPLACER, 0)) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&tenvoyer, NULL, 0, PRIORITY_TENVOYER, 0)) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&tlocaliser, NULL, 0, PRIORITY_LOCALISER, 0)) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&twatchdog, NULL, 0, PRIORITY_WATCHDOG, 0)) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* Creation des files de messages */
    if (err = rt_queue_create(&queueMsgGUI, "toto", MSG_QUEUE_SIZE * sizeof (DMessage), MSG_QUEUE_SIZE, Q_FIFO)) {
        rt_printf("Error msg queue create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* Creation des structures globales du projet */
    robot = d_new_robot();
    move = d_new_movement();
    serveur = d_new_server();
}

void startTasks() {
    int err;
    if (err = rt_task_start(&tconnect, &connecter, NULL)) {
        rt_printf("Error task connecter start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&tcommuniquer, &communiquer, NULL)) {
        rt_printf("Error task communiquer start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&tdeplacer, &deplacer, NULL)) {
        rt_printf("Error task deplacer start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&tenvoyer, &envoyer, NULL)) {
        rt_printf("Error task envoyer start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (err = rt_task_start(&tbattery, &battery, NULL)) {
        rt_printf("Error task battery start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&tlocaliser, &localiser, NULL)) {
        rt_printf("Error task localiser start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }/*
    if (err = rt_task_start(&twatchdog, &watchdog, NULL)) {
        rt_printf("Error task watchdog start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }*/
    if (err = rt_task_start(&tcalibrer, &calibrer, NULL)) {
        rt_printf("Error task calibrer start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }


}

void deleteTasks() {
    rt_task_delete(&tcommuniquer);
    rt_task_delete(&tconnect);
    rt_task_delete(&tcommuniquer);
    rt_task_delete(&tdeplacer);
    rt_task_delete(&tbattery);
    rt_task_delete(&twatchdog);
    rt_task_delete(&tlocaliser);
    rt_task_delete(&tcalibrer);
}
