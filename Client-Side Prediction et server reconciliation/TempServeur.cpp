#include "GameServer.h"
#include <iostream>

#include <string> // Pour les string
#include <cstring>
#include "bat.h"
#include "ball.h"
#include "Status.h"
#include <pthread.h>
#include <queue>
#include <SFML/Graphics.hpp>
#include <time.h>
#include <sys/time.h>

using namespace std;

int windowWidth = 1024;
int windowHeight = 768;

int initServer(GameServer& server);

int initGame(Text& hud,string fontPath , RectangleShape (&separators)[16]);

int sendHudSeparator(Text& hud, string fontPath, RectangleShape (&separators)[16],bool isClient1);

int AnalyseEvent(Bat &batC1, Bat &batC2);

void HandleBall(Ball &ball, Bat &batC1, Bat &batC2, int &scoreC1, int &scoreC2);

int SendInfoToClient(GameClient *client, Ball &ball, Bat &batC1, Bat &batC2, int scoreC1, int scoreC2, bool isClient1);

int SendAllInfoToClients(Ball &ball, Bat &batC1, Bat &batC2, int scoreC1, int scoreC2);
int stopConnection(string Data,bool isClient1);

void *FctThreadReceive(void *setting);
void handleThreadReceiveStatus(int status, char *Data, bool isClient1);
int receiveDataClient1(string& data);
int receiveDataClient2(string& data);

// Fonction pour calculer la différence de temps en millisecondes
float time_diff(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 + (end->tv_nsec - start->tv_nsec) / 1000000.0;
}

GameClient* client1=new GameClient();
GameClient* client2=new GameClient();

pthread_t threadRecv;

pthread_mutex_t mutexReceiveClient1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condReceiveClient1 = PTHREAD_COND_INITIALIZER;
queue<string> receivedQueueClient1; //fifo
bool receiveDataAvailableClient1=false;

pthread_mutex_t mutexReceiveClient2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condReceiveClient2 = PTHREAD_COND_INITIALIZER;
queue<string> receivedQueueClient2; //fifo
bool receiveDataAvailableClient2=false;



bool erreurRcv = false;
pthread_mutex_t mutexErreurRcv = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condErreur = PTHREAD_COND_INITIALIZER;

struct Input
{
    string action;
    long sequenceNumber;
};

Input lastInput; // Dernière entrée reçue du client
long sequenceNumber = 0; // Numéro de séquence que le serveur doit traiter et renvoyer


int main(int argc, char *argv[])
{

    if(argc < 2)
    {
        cout<<endl <<"(SERVEUR)!!! Merci d'entrer Un numero de port(argument) pour lancer le serveur !!!"<< endl;
        return -1;
    }

    // Conv l'argument du port en entier
    int port = atoi(argv[1]);


    GameServer server(port); // Declare and initialize the server object
    int status= initServer(server);
    if(status != OK)
    {
        //cout << "(SERVEUR)ERREUR d'initialisation du serveur" << endl;
        return status;
    }

    
    cout << "(SERVEUR)En attente de connexion du client 1 puis 2 :" << endl;
    status = server.acceptClient(client1);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR de connexion client " << endl;
        return status;
    }
    cout << "(SERVEUR)Client 1 connecté" << endl;

    status = server.acceptClient(client2);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR de connexion client " << endl;
        return status;
    }
    cout << "(SERVEUR)Client 2 connecté" << endl;

    int res=pthread_create(&threadRecv, NULL, FctThreadReceive,NULL);
    if(res==0){cout<<endl<<"ThreadRecv " + to_string(threadRecv) + " cree avec succe"<<endl;}
    else{cout<<endl<<"ERREUR creation ThreadRecv"<<endl; return -1;}

    Bat batC1 (0, windowHeight/2);
    Bat batC2(windowWidth-batC1.getShape().getSize().x, windowHeight/2);
    Ball ball(windowWidth / 2, windowHeight/2);
    int scoreC1=0, scoreC2=0;
    char Data[1024];

    Text hud;
    Font font;
    font.loadFromFile("OpenSans-Bold.ttf");
    hud.setFont(font);
    hud.setCharacterSize(75);
    hud.setFillColor(sf::Color::White);
    hud.setPosition(Vector2f((windowWidth/2)-100,0));

    RectangleShape separators[16];
    int y_sepa = 0;
    for (int i = 0; i<16;i++){
        separators[i].setSize(Vector2f(20,32));
        separators[i].setPosition(Vector2f(windowWidth/2-10,y_sepa));

        y_sepa+=64;
    }
    struct timespec debut;
    struct timespec fin;

    const int tickRate = 64; // Taux de rafraîchissement en Hz
    const float tickDuration = 10000.f / tickRate; // Durée d'une boucle en microsecondes

    //init game
    cout << "(SERVEUR)Initialisation du jeu en cours ..........." << endl;

   status=initGame(hud, "OpenSans-Bold.ttf", separators);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'initialisation du jeu" << endl;
        pthread_cancel(threadRecv); pthread_join(threadRecv, NULL);
        return status;
    }

    cout << "(SERVEUR)!!! Jeu initialisé avec succes !!!" << endl;
    cout << "(SERVEUR)!!! Jeu commence !!!" << endl;
    ball.start();
    bool start = true;
    
    sf::Clock clock;

    while (start==true)
    {
        sequenceNumber++;
        clock_gettime(CLOCK_REALTIME, &debut);

        cout << "(SERVEUR)En attente d'Even des clients ; 1 puis 2" << endl;
        status = AnalyseEvent(batC1, batC2);
        if (status != OK)
        {
            if (status == 99)
            {
                //cout << "(SERVEUR)Fin de connexion" << endl;
                break;
            }
            
            cout << "(SERVEUR)ERREUR d'analyse des Evenements" << endl;
            pthread_cancel(threadRecv); pthread_join(threadRecv, NULL);
            return status;
        }

        HandleBall(ball, batC1, batC2, scoreC1, scoreC2);
        //send all info to clients
        status= SendAllInfoToClients(ball, batC1, batC2, scoreC1, scoreC2);
        if(status != OK)
        {
            cout << "(SERVEUR)ERREUR d'envoi des positions ball et bats aux clients" << endl;
            pthread_cancel(threadRecv); pthread_join(threadRecv, NULL);
            return status;
        }

        clock_gettime(CLOCK_REALTIME, &fin);
        float diff = time_diff(&debut, &fin); // Temps écoulé en millisecondes

        cout<<"(SERVEUR)Temps écoulé(ms) : "<<diff<<endl;
        if (diff < tickDuration) { // Si le temps écoulé est inférieur à la durée d'une boucle
            //sf::sleep(sf::milliseconds(tickDuration - diff)); // Attendre le temps restant
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (tickDuration - diff) * 1000000; // Convertir le temps restant en nanosecondes
            nanosleep(&ts, NULL); // Attendre le temps restant
        }

    }// This is the end of the "while" loop

    pthread_cancel(threadRecv);
    pthread_join(threadRecv, NULL);
    return 0;
}


int initServer(GameServer& server)
{
    int status;
    status = server.initialize();
    switch (status)
    {
    case ALREADY_READY:
        cout << "(SERVEUR)ERREUR, Serveur deja initialisé" << endl;
        return status;
        break;

    case SOCKET_ERROR:
        cout << "(SERVEUR)ERREUR de creation de socket" << endl;
        return status;
        break;

    case BIND_ERROR: // si port deja utilisé par un autre processus
        cout << "(SERVEUR)ERREUR de bind" << endl;
        return status;
        break;
    case LISTEN_ERROR: //impossible de passer le serveur(port) en mode ecoute
        cout << "(SERVEUR)ERREUR de listen" << endl;
        return status;
        break;

    case OK:
        cout << "(SERVEUR)Serveur initialisé avec succes" << endl;
        break;

    default:
        return -1;
    }
    return status;
}


int initGame(Text& hud,string fontPath , RectangleShape (&separators)[16])
{
    int status;
    status = sendHudSeparator(hud, fontPath, separators, true);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'initialisation de HUD & SEPARATOR pour client 1" << endl;
        return status;
    }

    status = sendHudSeparator(hud, fontPath, separators, false);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'initialisation de HUD & SEPARATOR pour client 2" << endl;
        return status;
    }

    //je peux aussi envoyer les positions des bats et de la balle


    return OK;
}

int sendHudSeparator(Text& hud, string fontPath, RectangleShape (&separators)[16],bool isClient1)
{
    string Data;
    int status=OK;
    if(isClient1)
    {
        //send HUD & SEPARATOR to client 1
        status=receiveDataClient1(Data);
        if(status != OK)
        {
            cout << "(SERVEUR)ERREUR de reception de la commande HUD & SEPARATOR du client 1" << endl;
            return status;
        }
    }
    else
    {
        //send HUD & SEPARATOR to client 2
        status=receiveDataClient2(Data);
        if(status != OK)
        {
            cout << "(SERVEUR)ERREUR de reception de la commande HUD & SEPARATOR du client 2" << endl;
            return status;
        }
    }

    cout << "(SERVEUR)Commande HUD & SEPARATOR reçu du client "<<((isClient1) ? "1" : "2") << endl;

    //serialisation et envoie de HUD & SEPARATOR au client
    ostringstream oss_HUD;
    oss_HUD << fontPath<<" " << hud.getCharacterSize() << " " << hud.getFillColor().toInteger() << " " << hud.getPosition().x << " " << hud.getPosition().y;
    
    ostringstream oss_sepa; 

    for (int i = 0; i < 16; i++) 
    {
        oss_sepa << separators[i].getSize().x << " " << separators[i].getSize().y << " ";
        oss_sepa << separators[i].getPosition().x << " " << separators[i].getPosition().y << " ";
    }

    if(isClient1)
    {
        status = client1->send((char*) oss_HUD.str().c_str());
        if(status != OK)
        {
            cout <<endl<< "(SERVEUR)ERREUR d'envoi GraphDATA HUD vers client 1" << endl;
            return status;
        }
        else
        {
            cout << endl<<"(SERVEUR)HUD vers client 1 envoyé avec succes:" << endl;
            cout<<endl<<oss_HUD.str()<<endl;
        }

         status = client1->send((char *)oss_sepa.str().c_str());
        if (status != OK)
        {
            cout << endl<< "(SERVEUR)ERREUR d'envoi SEPARATOR vers client 1" << endl;
            return status;
        }
        else
        {
            cout << endl<< "(SERVEUR)SEPARATOR vers client 1 envoyé avec succes:" << endl;
            cout << endl<< oss_sepa.str() << endl;
        }

    }
    else
    {
         status = client2->send((char*) oss_HUD.str().c_str());
        if(status != OK)
        {
            cout <<endl<< "(SERVEUR)ERREUR d'envoi GraphDATA HUD vers client 2" << endl;
            return status;
        }
        else
        {
            cout << endl<<"(SERVEUR)HUD vers client 2 envoyé avec succes:" << endl;
            cout<<endl<<oss_HUD.str()<<endl;
        }

         status = client2->send((char *)oss_sepa.str().c_str());
        if (status != OK)
        {
            cout << endl<< "(SERVEUR)ERREUR d'envoi SEPARATOR vers client 2" << endl;
            return status;
        }
        else
        {
            cout << endl<< "(SERVEUR)SEPARATOR vers client 2 envoyé avec succes:" << endl;
            cout << endl<< oss_sepa.str() << endl;
        }
    }

    return OK;
}

int AnalyseEvent(Bat &batC1, Bat &batC2)
{
    string Data;
    int status = OK;
    istringstream iss;

    // Client 1
    status = receiveDataClient1(Data);
    if (status == OK)
    {
        cout << "(SERVEUR)Even du client 1 reçu" << endl;
        if (Data=="STOP")
        {
            cout << "(SERVEUR)Fin de connexion demandée par client 1" << endl;
            status = client2->send((char *)"STOP");
            if (status != OK)
            {
                cout << "(SERVEUR)ERREUR d'envoi de message STOP au client 2" << endl;
                return status;
            }
            cout << "(SERVEUR)Fin de connexion envoyée au client 2" << endl;
            cout << "(SERVEUR)Fin de connexion" << endl;
            return 99;
        }

        // Analyser les données du client 1 (mouvement bat)
        cout << "(SERVEUR) Mouvement bat + sequenceNumber " << Data << endl;
        iss= istringstream(Data);
        iss >> lastInput.action >> lastInput.sequenceNumber;
        if (lastInput.action == "Up")
        {
            if (batC1.getPosition().top > 0)
            {
                cout << endl << "!! (SERVEUR)CLIENT 1 MOVE UP !!";
                batC1.moveUp();
            }
        }
        else if (lastInput.action == "Down")
        {
            if (batC1.getPosition().top < windowHeight - batC1.getShape().getSize().y)
            {
                cout << endl << "!! (SERVEUR)CLIENT 1 MOVE DOWN !!";
                batC1.moveDown();
            }
        }
        else//movType == "NOT"
        {
            cout << endl << "!! (SERVEUR)CLIENT 1 N'AS PAS MOVE !!" << endl;
            cout << endl << "!! (SERVEUR) conservation position bat !!" << endl;
        }
    }
    else
    {
        cout << "(SERVEUR)ERREUR de reception position bat client 1" << endl;
        return status;
    }

    // Client 2
    status = receiveDataClient2(Data);
    if (status == OK)
    {
        cout << "(SERVEUR)Even du client 2 reçu" << endl;
        if (Data=="STOP")
        {
            cout << "(SERVEUR)Fin de connexion demandée par client 2" << endl;
            status = client1->send((char *)"STOP");
            if (status != OK)
            {
                cout << "(SERVEUR)ERREUR d'envoi de message STOP au client 1" << endl;
                return status;
            }
            cout << "(SERVEUR)Fin de connexion envoyée au client 1" << endl;
            cout << "(SERVEUR)Fin de connexion" << endl;
            return 99;
        }


        // Analyser les données du client 2 (²mouvement bat)
        iss= istringstream(Data);
        iss >> lastInput.action >> lastInput.sequenceNumber;
        cout << "(SERVEUR) Mouvement bat + sequenceNumber : " << Data<< endl;
        if (lastInput.action == "Up")
        {
            if (batC2.getPosition().top > 0)
            {
                cout << endl << "!! (SERVEUR)CLIENT 2 MOVE UP !!";
                batC2.moveUp();
            }
        }
        else if (lastInput.action == "Down")
        {
            if (batC2.getPosition().top < windowHeight - batC2.getShape().getSize().y)
            {
                cout << endl << "!! (SERVEUR)CLIENT 2 MOVE DOWN !!";
                batC2.moveDown();
            }
        }
        else
        {
            cout << endl << "!! (SERVEUR)CLIENT 2 N'AS PAS MOVE !!" << endl;
            cout << endl << "!! (SERVEUR) conservation position bat !!" << endl;
        }
    }
    else
    {
        cout << "(SERVEUR)ERREUR de reception position bat client 2" << endl;
        return status;
    }

    return OK;
}


void HandleBall(Ball &ball, Bat &batC1, Bat &batC2, int &scoreC1, int &scoreC2)
{
    // Handle ball hitting top or bottom
    if (ball.getPosition().top > windowHeight || ball.getPosition().top < 0)
    {
        // reverse the ball direction
        ball.reboundTopOrBot();
    }

    // Handle ball hitting left side
    if (ball.getPosition().left < 0)
    {
        ball.hitSide(windowWidth / 2, windowHeight / 2);
        scoreC2++;
        batC1.setYPosition(windowHeight / 2);
        batC2.setYPosition(windowHeight / 2);
    }

    // Handle ball hitting right side
    if (ball.getPosition().left > windowWidth)
    {
        ball.hitSide(windowWidth / 2, windowHeight / 2);
        scoreC1++;
        batC1.setYPosition(windowHeight / 2);
        batC2.setYPosition(windowHeight / 2);
    }

    // Has the ball hit the bat?
    if (ball.getPosition().intersects(batC1.getPosition()) || ball.getPosition().intersects(batC2.getPosition()))
    {
        ball.reboundBat();
    }

    batC1.update();
    batC2.update();
    ball.update();
}


int SendInfoToClient(GameClient *client, Ball &ball, Bat &batC1, Bat &batC2, int scoreC1, int scoreC2, bool isClient1)
{
    ostringstream oss;
    if (isClient1)
    {
        oss << ball.getPosition().left << " " << ball.getPosition().top << " ";
        oss << scoreC1 << " " << scoreC2 << " ";
        oss << batC1.getPosition().left << " " << batC1.getPosition().top << " ";
        oss << batC2.getPosition().left << " " << batC2.getPosition().top <<" ";
        oss << sequenceNumber;
    }
    else
    {
        oss << (windowWidth - ball.getPosition().left - ball.getShape().getSize().x) << " " << ball.getPosition().top << " ";
        oss << scoreC2 << " " << scoreC1 << " ";
        oss << (windowWidth - batC2.getPosition().left - batC2.getShape().getSize().x) << " " << batC2.getPosition().top << " ";
        oss << (windowWidth - batC1.getPosition().left - batC1.getShape().getSize().x) << " " << batC1.getPosition().top <<" ";
        oss << sequenceNumber;
    }
    oss << endl;

    int status = client->send((char*)oss.str().c_str());
    if (status != OK)
    {
        cout << "(SERVEUR)ERREUR d'envoi position ball et bats vers client " << (isClient1 ? "1" : "2") << endl;
        //return status;
    }
    else
    {
        cout << "(SERVEUR)Position ball[2] score[2] bats[4] vers client " << (isClient1 ? "1" : "2") << " envoyé avec succes:" << endl;
        cout << oss.str() << endl;
    }
    return status;
}


int SendAllInfoToClients(Ball &ball, Bat &batC1, Bat &batC2, int scoreC1, int scoreC2)
{
    int status= SendInfoToClient(client1,ball, batC1, batC2, scoreC1, scoreC2, true);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'envoi position ball et bats vers client 1" << endl;
        return status;
    }
    status= SendInfoToClient(client2,ball, batC1, batC2, scoreC1, scoreC2, false);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'envoi position ball et bats vers client 2" << endl;
        return status;
    }
}

int stopConnection(string Data,bool isClient1)
{
    if (isClient1)
    {
        cout << "(SERVEUR)Fin de connexion demandée par client 1" << endl;
        int status = client2->send((char*)Data.c_str());
        if (status != OK)
        {
            cout << "(SERVEUR)ERREUR d'envoi de message STOP au client 2" << endl;
            return status;
        }
        cout << "(SERVEUR)Fin de connexion envoyée au client 2" << endl;
        cout << "(SERVEUR)Fin de connexion" << endl;
        return 99;
    }
    else 
    {
        cout << "(SERVEUR)Fin de connexion demandée par client 2" << endl;
        int status = client1->send((char*)Data.c_str());
        if (status != OK)
        {
            cout << "(SERVEUR)ERREUR d'envoi de message STOP au client 1" << endl;
            return status;
        }
        cout << "(SERVEUR)Fin de connexion envoyée au client 1" << endl;
        cout << "(SERVEUR)Fin de connexion" << endl;
        return 99;
    }
}



void *FctThreadReceive(void *setting)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    while (1)
    {
        char Data[1024];
        int status;

        // Reception du client 1
        status = client1->receiveNonBlocking(Data, 200);
        handleThreadReceiveStatus(status, Data, true);

        // Reception du client 2
        status = client2->receiveNonBlocking(Data, 200);
        handleThreadReceiveStatus(status, Data, false);
    }
    pthread_exit(NULL);
}

void handleThreadReceiveStatus(int status, char *Data, bool isClient1)
{
    if (isClient1)
    {

        if (status == TIMEOUT)
        {
            // cout<<endl<<"timeout = front queue="<<receivedQueue.front()<<endl;
            pthread_mutex_lock(&mutexReceiveClient1);
            if (!receivedQueueClient1.empty())
            {
                receiveDataAvailableClient1 = true;
                pthread_cond_signal(&condReceiveClient1);
            }
            pthread_mutex_unlock(&mutexReceiveClient1);

        }
        else if (status != OK)
        {
            cout << "(CLIENT threadReceive)ERREUR reception des données du serveur" << endl;
            pthread_mutex_lock(&mutexErreurRcv);
            erreurRcv = true;
            pthread_cond_signal(&condErreur);
            pthread_mutex_unlock(&mutexErreurRcv);
            //break;
        }
        else
        {
            cout << "(CLIENT threadReceive)Données reçues du serveur " << Data << endl;
            pthread_mutex_lock(&mutexReceiveClient1);
            string tmp(Data);
            receivedQueueClient1.push(tmp);
            receiveDataAvailableClient1 = true;
            pthread_cond_signal(&condReceiveClient1);
            pthread_mutex_unlock(&mutexReceiveClient1);
        }
    }
    else
    {
        if (status == TIMEOUT)
        {
            // cout<<endl<<"timeout = front queue="<<receivedQueue.front()<<endl;
            pthread_mutex_lock(&mutexReceiveClient2);
            if (!receivedQueueClient2.empty())
            {
                receiveDataAvailableClient2 = true;
                pthread_cond_signal(&condReceiveClient2);
            }
            pthread_mutex_unlock(&mutexReceiveClient2);
        }
        else if (status != OK)
        {
            cout << "(CLIENT threadReceive)ERREUR reception des données du serveur" << endl;
            pthread_mutex_lock(&mutexErreurRcv);
            erreurRcv = true;
            pthread_cond_signal(&condErreur);
            pthread_mutex_unlock(&mutexErreurRcv);
           // break;
        }
        else
        {
            cout << "(CLIENT threadReceive)Données reçues du serveur " << Data << endl;
            pthread_mutex_lock(&mutexReceiveClient2);
            string tmp(Data);
            receivedQueueClient2.push(tmp);
            receiveDataAvailableClient2 = true;
            pthread_cond_signal(&condReceiveClient2);
            pthread_mutex_unlock(&mutexReceiveClient2);
        }
    }
}


int receiveDataClient1(string& data)
{
    pthread_mutex_lock(&mutexReceiveClient1);
    while (!receiveDataAvailableClient1 && !erreurRcv) 
    {
        pthread_cond_wait(&condReceiveClient1, &mutexReceiveClient1);
    }

    if (erreurRcv)
    {
        pthread_mutex_unlock(&mutexReceiveClient1);
        cout<<endl<<"(CLIENT1) fct receiveData()) Erreur de Receive"<<endl;
        return ERROR;
    }

    if (!receivedQueueClient1.empty())
    {
        data =String( receivedQueueClient1.front() );
        receivedQueueClient1.pop();
        cout << "(CLIENT1) fct receiveData())Données reçues du serveur: " << data << endl;
    }
    else
    {
        cout << "(CLIENT1) La file d'attente est vide" << endl;
        //return ERROR; // Ou un autre code d'erreur approprié
    }

    receiveDataAvailableClient1 = false;
    pthread_mutex_unlock(&mutexReceiveClient1);
    return OK;
}

int receiveDataClient2(string& data)
{
    pthread_mutex_lock(&mutexReceiveClient2);
    while (!receiveDataAvailableClient2 && !erreurRcv) 
    {
        pthread_cond_wait(&condReceiveClient2, &mutexReceiveClient2);
    }

    if (erreurRcv)
    {
        pthread_mutex_unlock(&mutexReceiveClient2);
        cout<<endl<<"(CLIENT2) fct receiveData()) Erreur de Receive"<<endl;
        return ERROR;
    }

    if (!receivedQueueClient2.empty())
    {
        data =String( receivedQueueClient2.front() );
        receivedQueueClient2.pop();
        cout << "(CLIENT2) fct receiveData())Données reçues du serveur: " << data << endl;
    }
    else
    {
        cout << "(CLIENT2) La file d'attente est vide" << endl;
        //return ERROR; // Ou un autre code d'erreur approprié
    }

    receiveDataAvailableClient2 = false;
    pthread_mutex_unlock(&mutexReceiveClient2);
    return OK;
}