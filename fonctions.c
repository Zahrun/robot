#include "fonctions.h"

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);

// TODO ajout sémaphore pour le lancer

void battery(void * arg) {
    int bat;
    DMessage *message;
    DBattery *battery = d_new_battery();
    int status;

    rt_printf("tbattery : Attente du sémaphore semWatchdog\n");
    rt_sem_p(&semBatterie, TM_INFINITE);
    rt_printf("tbattery : Lancement de test de batterie (sémaphore reçue)\n");

    rt_task_set_periodic(NULL, TM_NOW, QUARTER_SECOND); // 250ms

    while (1) {
        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        status = etatCommRobot;
        rt_mutex_release(&mutexEtat);

        if (status != STATUS_OK) {
            break;
        }

        status = robot->get_vbat(robot, &bat);
        if (status == STATUS_OK) {
            rt_printf("Battery OK status: %d, niveau: %d\n", status, bat);

            battery = d_new_battery();
            battery->set_level(battery, bat);
            message = d_new_message();
            message->put_battery_level(message, battery);
            message->print(message, 100);

            if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                message->free(message);
            }
        } else {
            rt_printf("Battery ERROR status: %d, niveau: %d\n", status, bat);
        }

        rt_task_wait_period(NULL); // plus précis que : rt_task_sleep_until()
    }
}

/* Notes pratiques par rapport à la LED du robot :
- LED clignote rapidement <=> batterie faible ( faut amener ce robot au responsable et changer de robot )
- LED qui clignot normalement <=> robot en attente de co
- LED allumée fixe <=> robot connecté */


void calibrer(void * arg) {
    rt_printf("tcalibrer : Attente du sémarphore semDetectArena\n");
    rt_sem_p(&semDetectArena, TM_INFINITE);
    rt_printf("tcalibrer : Lancement de la calibration d'arène (sémaphore reçue)\n");

    //init msg
    DMessage *message;

    //init camera
    DCamera *camera;
    camera = d_new_camera();
    //init Dimage;
    DImage *img;
    //init Djpegimage
    DJpegimage *jpegimg;
    while (1) {
        img = d_new_image();
        jpegimg = d_new_jpegimage();

        camera->get_frame(camera, img);
        d_jpegimage_compress(jpegimg, img);
        message = d_new_message();
        d_message_put_jpeg_image(message, jpegimg);
    }

}

void localiser(void * arg) { // En cours ( Alexis )
    int status, action;
    DMessage *message;
    //init camera
    DCamera *camera;
    camera = d_new_camera();
    camera->open(camera);
    //init Dimage;
    DImage *image;
    //init Djpegimage
    DJpegimage *jpeg;
    DPosition *position;
    DArena *arena;

    rt_printf("tlocaliser : Attente du sémarphore semLocalisation\n");
    rt_sem_p(&semLocalisation, TM_INFINITE);
    rt_printf("tlocaliser : Lancement de la localisation selon etatLocalisation (sémaphore reçue)\n");

    rt_task_set_periodic(NULL, TM_NOW, PERIOD_LOCALISER); // sujet -> 600ms

    while (1) {
        rt_task_wait_period(NULL);

        //On check l'état de la communication avec le moniteur
        //Pour cela on récupère le mutexEtat
        //On écrit l'état de la comunication dans la variable status et on relanche le mutexEtat
        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        status = etatCommMoniteur;
        rt_mutex_release(&mutexEtat);

        if (status == STATUS_OK) {

            //On veut mettre l'etat de l'action a effectuer dans la variable action
            //On recupere le mutexCamera puis on le relache
            rt_mutex_acquire(&mutexCamera, TM_INFINITE);
            action = etatLocalisation;
            rt_mutex_release(&mutexCamera);

            //On regarde quelle est la valeur de la variable action
            //En fonction, on va soit calculer la position du robot, soit ne rien faire

            switch (action) {
                    //Calcul de la position du robot dans l'arene
                case ACTION_STOP_COMPUTE_POSITION:
                    rt_printf("tlocaliser : ACTION_STOP_COMPUTE_POSITION detecté\n");
                    image = d_new_image();
                    camera->get_frame(camera, image);

                    if (image != NULL) {
                        jpeg = d_new_jpegimage();
                        jpeg->compress(jpeg, image);

                        message = d_new_message();
                        message->put_jpeg_image(message, jpeg);

                        // TODO : Dire à calibrer arène qu'on a envoyé une image pour qu'il calibre à partir de cette image?
                        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                            message->free(message);
                        }

                        image->free(image);
                        jpeg->free(jpeg);
                    }
                    break;

                case ACTION_COMPUTE_CONTINUOUSLY_POSITION:
                    rt_printf("tlocaliser : ACTION_COMPUTE_CONTINUOUSLY_POSITION detecté\n");
                    image = d_new_image();
                    camera->get_frame(camera, image);

                    if (image != NULL) {

                        rt_mutex_acquire(&mutexPosition, TM_INFINITE);
                        position = image->compute_robot_position(image, arena);
                        rt_mutex_release(&mutexPosition);


                        if (position != NULL) {
                            d_imageshop_draw_position(image, position);
                            message = d_new_message();
                            message->put_position(message, position);

                            if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                                message->free(message);
                            }
                            jpeg = d_new_jpegimage();
                            jpeg->compress(jpeg, image);

                            message = d_new_message();
                            message->put_jpeg_image(message, jpeg);

                            if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                                message->free(message);
                            }
                            jpeg->free(jpeg);
                        }
                        image->free(image);

                    }

                    break;

            }
        }
    }
}

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
    rt_task_set_periodic(NULL, TM_NOW, HALF_SECOND);

    rt_printf("twathdog : Attente du sémarphore semWatchdog\n");
    rt_sem_p(&semWatchdog, TM_INFINITE);
    rt_printf("twatchdog : Watchdog en marche (sémaphore reçue)\n");

    while (1) { // changer cond?	

        status = robot->get_status(robot);

        if (status == STATUS_OK) {

            do {
                rt_printf("twatchdog : envoi du reload\n");
                status = robot->reload_wdt(robot);

                if (status == STATUS_OK) {
                    rt_printf("twatchdog : reload envoyé au robot\n");
                } else {
                    rt_printf("PROBLEME => twatchdog : reload NON envoyé au robot %d : %c\n", status, status);
                }
            } while (status != STATUS_OK);
        } else {
            rt_printf("PROBLEME => twatchdog : get_status initial %d : %c\n", status, status);
            rt_mutex_acquire(&mutexEtat, TM_INFINITE);
            etatCommRobot = status;
            rt_mutex_release(&mutexEtat);
        }

        rt_task_wait_period(NULL);
        // pas de : rt_task_sleep_until(ONE_SECOND);
        // on a une seconde pile comme ça, pas 1s + tps d'activité du watchdog	
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

            rt_mutex_acquire(&mutexEtat, TM_INFINITE);
            etatCommRobot = status;
            rt_mutex_release(&mutexEtat);

            if (status == STATUS_OK) {
                rt_printf("tconnect : Robot démarrer\n");

                // libération du semaphore pour lancer le watchdog ( twatchdog était en attente )
                rt_sem_v(&semWatchdog);
                // libération du semaphore pour lancer le test de batterie ( tbattery était en attente )
                rt_sem_v(&semBatterie);
                // libération du semaphore pour lancer le thread de localisation ( tlocaliser était en attente )
                rt_sem_v(&semLocalisation);
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
            /*Type du message. Les différents types sont : 
             * #MESSAGE_TYPE_UNKNOWN, #MESSAGE_TYPE_CHAR, #MESSAGE_TYPE_INT, 
             * #MESSAGE_TYPE_STRING, #MESSAGE_TYPE_STATE, #MESSAGE_TYPE_IMAGE,
             * #MESSAGE_TYPE_BATTERY, #MESSAGE_TYPE_MOVEMENT, #MESSAGE_TYPE_ACTION, 
             * #MESSAGE_TYPE_ORDER, #MESSAGE_TYPE_POSITION, #MESSAGE_TYPE_MISSION*/
            switch (msg->get_type(msg)) {
                case MESSAGE_TYPE_ACTION:
                    /*possibles sont :#ACTION_FIND_ARENA, #ACTION_ARENA_FAILED,
                     * #ACTION_ARENA_IS_FOUND, #ACTION_COMPUTE_CONTINUOUSLY_POSITION,
                     * #ACTION_CONNECT_ROBOT, #ACTION_ARENA_FAILED*/
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
                        case ACTION_FIND_ARENA:
                            rt_printf("tserver : Action trouver arene\n");
                            rt_sem_v(&semDetectArena);
                            break;
                        case ACTION_ARENA_FAILED: // TODO
                            rt_printf("tserver : Action échec detection arene\n");
                            break;
                        case ACTION_ARENA_IS_FOUND: // TODO
                            rt_printf("tserver : Action arene trouvée\n");
                            break;
                        case ACTION_COMPUTE_CONTINUOUSLY_POSITION: // TODO
                            rt_printf("tserver : Action calculer position en continu\n");

                            //On desire changer l'etat de la variable etatCamera ( qui sera lu par le thread tlocaliser )
                            //Pour cela on récupère le mutexCamera et on ecrit l'etat
                            rt_mutex_acquire(&mutexCamera, TM_INFINITE);
                            etatLocalisation = ACTION_COMPUTE_CONTINUOUSLY_POSITION;
                            rt_mutex_release(&mutexCamera);
                            break;

                        case ACTION_STOP_COMPUTE_POSITION: // TODO
                            rt_printf("tserver : Action arreter calcul position\n");

                            //On desire changer l'etat de la variable etatCamera ( qui sera lu par le thread tlocaliser )
                            //Pour cela on récupère le mutexCamera et on ecrit l'etat
                            rt_mutex_acquire(&mutexCamera, TM_INFINITE);
                            etatLocalisation = ACTION_STOP_COMPUTE_POSITION;
                            rt_mutex_release(&mutexCamera);
                            break;

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
            if (move->get_speed(move) > 50) { // ne marche pas en vitesse rapide...
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
                    default:
                        rt_printf("pas bonne direction\n");
                        break;

                }
            } else {
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
                    default:
                        rt_printf("pas bonne direction\n");
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





