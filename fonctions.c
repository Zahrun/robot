#include "fonctions.h"

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);

void envoyer(void * arg) {
    DMessage *msg;
    int err;

    while (1) {
        rt_printf("tenvoyer : Attente d'un message\n");
        if ((err = rt_queue_read(&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
            rt_printf("tenvoyer : envoi d'un message au moniteur\n");
            serveur->send(serveur, msg);
            msg->free(msg);
        } else {
            rt_printf("Error msg queue write: %s\n", strerror(-err));
        }
    }
}

void watchdog(void * arg) {
	int status;
	while (1) { // changer cond?
		status = robot->get_status(robot);
		
		rt_task_sleep_until(1);// wait 1 second

        if (status == STATUS_OK) {

			status = robot->reload_wdt(robot); 

            if (status == STATUS_OK){
                rt_printf("twatchdog : reload envoyé au robot\n");
            } else {
				rt_printf("PROBLEME => twatchdog : reload NON envoyé au robot\n");
			}
        } else {
			rt_printf("PROBLEME => twatchdog : get_status initial /= STATUS_OK\n");
		}
	}
}


void connecter(void * arg) {
    int status;
    DMessage *message;

    rt_printf("tconnect : Debut de l'exécution de tconnect\n");

    while (1) {
        rt_printf("tconnect : Attente du sémarphore semConnecterRobot\n");
        rt_sem_p(&semConnecterRobot, TM_INFINITE);
        rt_printf("tconnect : Ouverture de la communication avec le robot\n");
        status = robot->open_device(robot);

        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        etatCommRobot = status;
        rt_mutex_release(&mutexEtat);

        if (status == STATUS_OK) {
             status = robot->start_insecurely(robot);
			//status = robot->start(robot); 
			// lance le watchdog qui attendra d_robot_reload_wdt toutes les 1 sec ( avec tolérance de 50 ms )

			// nouveau thread : Gestion watchdog !
			int err;
			/*if (err = rt_task_start(&twatchdog, &watchdog, NULL)) {
				rt_printf("Error task start: %s\n", strerror(-err));
				exit(EXIT_FAILURE);
			}*/



            if (status == STATUS_OK){
                rt_printf("tconnect : Robot démarrer\n");
            }
        }

        message = d_new_message();
        message->put_state(message, status);

        rt_printf("tconnecter : Envoi message\n");
        message->print(message, 100);

        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
            message->free(message);
        }
    }
}

void communiquer(void *arg) {
    DMessage *msg = d_new_message();
    int var1 = 1;
    int num_msg = 0;

    rt_printf("tserver : Début de l'exécution de serveur\n");
    serveur->open(serveur, "8000");
    rt_printf("tserver : Connexion\n");

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    etatCommMoniteur = 0;
    rt_mutex_release(&mutexEtat);

    while (var1 > 0) {
        rt_printf("tserver : Attente d'un message\n");
        var1 = serveur->receive(serveur, msg);
        num_msg++;
        if (var1 > 0) {
            switch (msg->get_type(msg)) {
                case MESSAGE_TYPE_ACTION:
                    rt_printf("tserver : Le message %d reçu est une action\n",
                            num_msg);
                    DAction *action = d_new_action();
                    action->from_message(action, msg);
                    switch (action->get_order(action)) {
                        case ACTION_CONNECT_ROBOT:
                            rt_printf("tserver : Action connecter robot\n");
                            rt_sem_v(&semConnecterRobot);
                            break;
                            // début ajout clément
                            case ACTION_FIND_ARENA: // TODO
                            rt_printf("tserver : Action trouver arene\n");
                            break;
                        case ACTION_ARENA_FAILED: // TODO
                            rt_printf("tserver : Action échec detection arene\n");
                            break;
                        case ACTION_ARENA_IS_FOUND: // TODO
                            rt_printf("tserver : Action arene trouvée\n");
                            break;
                        case ACTION_COMPUTE_CONTINUOUSLY_POSITION: // TODO
                            rt_printf("tserver : Action calculer position en continu\n");
                            break;
                        case ACTION_STOP_COMPUTE_POSITION: // TODO
                            rt_printf("tserver : Action arreter calcul position\n");
                            break;
                            //fin ajout clément
                    }
                    break;
                case MESSAGE_TYPE_MOVEMENT:
                    rt_printf("tserver : Le message reçu %d est un mouvement\n",
                            num_msg);
                    rt_mutex_acquire(&mutexMove, TM_INFINITE);
                    move->from_message(move, msg);
                    move->print(move);
                    rt_mutex_release(&mutexMove);
                    break;
            }
        }
        /*
        message = d_new_message();
        message->put_state(message, status);

        rt_printf("tconnecter : Envoi message\n");
        message->print(message, 100);

		if (write_in_queue(&queueMsgGUI, msg, sizeof (DMessage)) < 0) {
			msg->free(message);
		}
		*/
    }
}

void deplacer(void *arg) {
    int status = 1;
    int gauche;
    int droite;
    DMessage *message;

    rt_printf("tmove : Debut de l'éxecution de periodique à 1s\n");
    rt_task_set_periodic(NULL, TM_NOW, 1000000000);

    while (1) {
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_printf("tmove : Activation périodique\n");

        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        status = etatCommRobot;
        rt_mutex_release(&mutexEtat);

        if (status == STATUS_OK) {
            rt_mutex_acquire(&mutexMove, TM_INFINITE);
            if (move->get_speed(move) > 50){
            	switch (move->get_direction(move)) {
		            case DIRECTION_FORWARD:
		                gauche = MOTEUR_ARRIERE_RAPIDE;
		                droite = MOTEUR_ARRIERE_RAPIDE;
		                break;
		            case DIRECTION_LEFT:
		                gauche = MOTEUR_ARRIERE_RAPIDE;
		                droite = MOTEUR_AVANT_RAPIDE;
		                break;
		            case DIRECTION_RIGHT:
		                gauche = MOTEUR_AVANT_RAPIDE;
		                droite = MOTEUR_ARRIERE_RAPIDE;
		                break;
		            case DIRECTION_STOP:
		                gauche = MOTEUR_STOP;
		                droite = MOTEUR_STOP;
		                break;
		            case DIRECTION_STRAIGHT:
		                gauche = MOTEUR_AVANT_RAPIDE;
		                droite = MOTEUR_AVANT_RAPIDE;
		                break;
		        }
            } else{
		        switch (move->get_direction(move)) {
		            case DIRECTION_FORWARD:
		                gauche = MOTEUR_ARRIERE_LENT;
		                droite = MOTEUR_ARRIERE_LENT;
		                break;
		            case DIRECTION_LEFT:
		                gauche = MOTEUR_ARRIERE_LENT;
		                droite = MOTEUR_AVANT_LENT;
		                break;
		            case DIRECTION_RIGHT:
		                gauche = MOTEUR_AVANT_LENT;
		                droite = MOTEUR_ARRIERE_LENT;
		                break;
		            case DIRECTION_STOP:
		                gauche = MOTEUR_STOP;
		                droite = MOTEUR_STOP;
		                break;
		            case DIRECTION_STRAIGHT:
		                gauche = MOTEUR_AVANT_LENT;
		                droite = MOTEUR_AVANT_LENT;
		                break;
		        }
            }
            rt_mutex_release(&mutexMove);

            status = robot->set_motors(robot, gauche, droite);



            if (status != STATUS_OK) {
                rt_mutex_acquire(&mutexEtat, TM_INFINITE);
                etatCommRobot = status;
                rt_mutex_release(&mutexEtat);

                message = d_new_message();
                message->put_state(message, status);

                rt_printf("tmove : Envoi message \"%s\":%d\n", message, status);
                if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                    message->free(message);
                }
            }
        }
    }
}

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size) {
    void *msg;
    int err;

    msg = rt_queue_alloc(msgQueue, size);
    memcpy(msg, &data, size);

    if ((err = rt_queue_send(msgQueue, msg, sizeof (DMessage), Q_NORMAL)) < 0) {
        rt_printf("Error msg queue send: %s\n", strerror(-err));
    }
    rt_queue_free(&queueMsgGUI, msg);

    return err;
}





