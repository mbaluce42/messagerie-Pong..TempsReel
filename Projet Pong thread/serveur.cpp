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

struct Input
{
    string action;
    long long timestamp_ms;
};

int initServer(GameServer& server);

int initGame(Text& hud,string fontPath , RectangleShape (&separators)[16]);

int sendHudSeparator(Text& hud, string fontPath, RectangleShape (&separators)[16],bool isClient1);

int AnalyseEvent(Bat &batC1, Bat &batC2);

int HandleEven(string data, Bat &batC1, Bat &batC2, bool isClient1);

void HandleBall(Ball &ball, Bat &batC1, Bat &batC2, int &scoreC1, int &scoreC2);

int SendInfoToClient(GameClient *client, Ball &ball, Bat &batC1, Bat &batC2, int scoreC1, int scoreC2, bool isClient1);

int SendAllInfoToClients(Ball &ball, Bat &batC1, Bat &batC2, int scoreC1, int scoreC2);
int stopConnection(string Data,bool isClient1);

void *FctThreadReceiveClient1(void *setting);
void *FctThreadReceiveClient2(void *setting);
void handleThreadReceiveStatus(int status, char *Data, bool isClient1);
int receiveDataClient1(Input& input);
int receiveDataClient2(Input& input);

// Fonction pour calculer la différence de temps en millisecondes
float time_diff(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 + (end->tv_nsec - start->tv_nsec) / 1000000.0;
}

GameClient* client1=new GameClient();
GameClient* client2=new GameClient();

pthread_t threadRecv1;

pthread_t threadRecv2;


pthread_mutex_t mutexReceiveClient1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condReceiveClient1 = PTHREAD_COND_INITIALIZER;
queue<Input> receivedQueueClient1; //fifo
bool receiveDataAvailableClient1=false;
Input inputClient1;

pthread_mutex_t mutexReceiveClient2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condReceiveClient2 = PTHREAD_COND_INITIALIZER;
queue<Input> receivedQueueClient2; //fifoµ
bool receiveDataAvailableClient2=false;
Input inputClient2;


bool erreurRcv1 = false;
pthread_mutex_t mutexErreurRcv1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condErreur1 = PTHREAD_COND_INITIALIZER;

bool erreurRcv2 = false;
pthread_mutex_t mutexErreurRcv2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condErreur2 = PTHREAD_COND_INITIALIZER;


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

    int res1=pthread_create(&threadRecv1, NULL, FctThreadReceiveClient1,NULL);
    if(res1==0){cout<<endl<<"ThreadRecv1 " + to_string(threadRecv1) + " cree avec succe"<<endl;}
    else{cout<<endl<<"ERREUR creation ThreadRecv1"<<endl; return -1;}

    int res2=pthread_create(&threadRecv2, NULL, FctThreadReceiveClient2,NULL);
    if(res2==0){cout<<endl<<"ThreadRecv2 " + to_string(threadRecv2) + " cree avec succe"<<endl;}
    else{cout<<endl<<"ERREUR creation ThreadRecv2"<<endl; return -1;}

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
    const float tickDuration = 1100.f / tickRate; // Durée d'une boucle en microsecondes

    //init game
    cout << "(SERVEUR)Initialisation du jeu en cours ..........." << endl;

   status=initGame(hud, "OpenSans-Bold.ttf", separators);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'initialisation du jeu" << endl;
        pthread_cancel(threadRecv1); pthread_join(threadRecv1, NULL); pthread_cancel(threadRecv2); pthread_join(threadRecv2, NULL);
        return status;
    }

    cout << "(SERVEUR)!!! Jeu initialisé avec succes !!!" << endl;
    cout << "(SERVEUR)!!! Jeu commence !!!" << endl;
    ball.start();
    bool start = true;
    
    sf::Clock clock;

    while (start==true)
    {
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
            pthread_cancel(threadRecv1); pthread_join(threadRecv1, NULL); pthread_cancel(threadRecv2); pthread_join(threadRecv2, NULL);
            return status;
        }

        HandleBall(ball, batC1, batC2, scoreC1, scoreC2);
        //send all info to clients

        if(inputClient1.timestamp_ms <= inputClient2.timestamp_ms)
        {
            cout << "(SERVEUR)Faut envoyer la MAJ au client 1 en premier" << endl;
            int status= SendInfoToClient(client1,ball, batC1, batC2, scoreC1, scoreC2, true);
            if(status != OK)
            {
                cout << "(SERVEUR)ERREUR d'envoi position ball et bats vers client 1" << endl;
                pthread_cancel(threadRecv1); pthread_join(threadRecv1, NULL); pthread_cancel(threadRecv2); pthread_join(threadRecv2, NULL);
                return status;
            }
            cout<<"(SERVEUR) au tour du client 2"<<endl;
            status= SendInfoToClient(client2,ball, batC1, batC2, scoreC1, scoreC2, false);
            if(status != OK)
            {
                cout << "(SERVEUR)ERREUR d'envoi position ball et bats vers client 2" << endl;
                pthread_cancel(threadRecv1); pthread_join(threadRecv1, NULL); pthread_cancel(threadRecv2); pthread_join(threadRecv2, NULL);
                return status;
            }
        }
        else
        {
            cout << "(SERVEUR)Faut envoyer la MAJ au client 2 en premier" << endl;
            int status= SendInfoToClient(client2,ball, batC1, batC2, scoreC1, scoreC2, false);
            if(status != OK)
            {
                cout << "(SERVEUR)ERREUR d'envoi position ball et bats vers client 2" << endl;
                pthread_cancel(threadRecv1); pthread_join(threadRecv1, NULL); pthread_cancel(threadRecv2); pthread_join(threadRecv2, NULL);
                return status;
            }
            cout<<"(SERVEUR) au tour du client 1"<<endl;
            status= SendInfoToClient(client1,ball, batC1, batC2, scoreC1, scoreC2, true);
            if(status != OK)
            {
                cout << "(SERVEUR)ERREUR d'envoi position ball et bats vers client 1" << endl;
                pthread_cancel(threadRecv1); pthread_join(threadRecv1, NULL); pthread_cancel(threadRecv2); pthread_join(threadRecv2, NULL);
                return status;
            }

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
            cout<<endl<<endl<<endl<<endl<<"(SERVEUR)Attente de "<<(tickDuration - diff)<<" ms"<<endl;

        }

    }// This is the end of the "while" loop

    pthread_cancel(threadRecv1); pthread_join(threadRecv1, NULL); pthread_cancel(threadRecv2); pthread_join(threadRecv2, NULL);
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

    status= receiveDataClient1(inputClient1);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR de reception de la commande HUD & SEPARATOR du client 1" << endl;
        return status;
    }
    status= receiveDataClient2(inputClient2);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR de reception de la commande HUD & SEPARATOR du client 2" << endl;
        return status;
    }

    if(inputClient1.timestamp_ms <= inputClient2.timestamp_ms)
    {
        cout << "(SERVEUR)Client 1 a envoyé en premier la commande HUD & SEPARATOR" << endl;
        status = sendHudSeparator(hud, fontPath, separators, true);
        if(status != OK)
        {
            return status;
        }

        status = sendHudSeparator(hud, fontPath, separators, false);
        if(status != OK)
        {
            return status;
        }
    }
    else
    {
        cout << "(SERVEUR)Client 2 a envoyé en premier la commande HUD & SEPARATOR" << endl;
        status = sendHudSeparator(hud, fontPath, separators, false);
        if(status != OK)
        {
            return status;
        }
        status = sendHudSeparator(hud, fontPath, separators, true);
        if(status != OK)
        {
            return status;
        }
    }

    //je peux aussi envoyer les positions des bats et de la balle


    return OK;
}

int sendHudSeparator(Text& hud, string fontPath, RectangleShape (&separators)[16],bool isClient1)
{
    int status=OK;

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
    int status = OK;

    status= receiveDataClient1(inputClient1);
    if(status != OK)
    {
        cout << "(SERVEUR)[CLIENT1] ERREUR de reception du mouvement du client" << endl;
        return status;
    }
    status= receiveDataClient2(inputClient2);
    if(status != OK)
    {
        cout << "(SERVEUR)[CLIENT1] ERREUR de reception du mouvement du client" << endl;
        return status;
    }

    if(inputClient1.timestamp_ms <= inputClient2.timestamp_ms)
    {
        cout << "(SERVEUR)[CLIENT1] Client 1 a envoyé en premier sont intention de MOVE" << endl;
        status = HandleEven(inputClient1.action,batC1,batC2,true);
        if(status != OK)
        {
            return status;
        }

        cout<<"(SERVEUR)[CLIENT2] traitement de MOVE du client 2"<<endl;

        status = HandleEven(inputClient2.action,batC1,batC2,false);
        if(status != OK)
        {
            return status;
        }
    }
    else
    {
        cout << "(SERVEUR)[CLIENT2] Client 2 a envoyé en premier sont intention de MOVE" << endl;
        status = HandleEven(inputClient2.action,batC1,batC2,false);
        if(status != OK)
        {
            return status;
        }

        cout<<"(SERVEUR)[CLIENT1] traitement de MOVE du client 1";

        status = HandleEven(inputClient1.action,batC1,batC2,true);
        if(status != OK)
        {
            return status;
        }
    }

    return OK;
}

int HandleEven(string data, Bat &batC1, Bat &batC2, bool isClient1)
{
    int status = OK;
    if (isClient1)
    {
        if (data == "STOP")
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
        cout << "Mouvement bat : " << data << endl;
        if (data == "Up")
        {
            if (batC1.getPosition().top > 0)
            {
                cout << endl<< "!! (SERVEUR)CLIENT 1 MOVE UP !!";
                batC1.moveUp();
            }
        }
        else if (data == "Down")
        {
            if (batC1.getPosition().top < windowHeight - batC1.getShape().getSize().y)
            {
                cout << endl<< "!! (SERVEUR)CLIENT 1 MOVE DOWN !!";
                batC1.moveDown();
            }
        }
        else // movType == "NOT"
        {
            cout << endl<< "!! (SERVEUR)CLIENT 1 N'AS PAS MOVE !!" << endl;
            cout << endl<< "!! (SERVEUR) conservation position bat !!" << endl;
        }
    }
    else
    {
        if (data == "STOP")
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
        cout << "Mouvement bat : " << data << endl;
        if (data == "Up")
        {
            if (batC2.getPosition().top > 0)
            {
                cout << endl<< "!! (SERVEUR)CLIENT 2 MOVE UP !!";
                batC2.moveUp();
            }
        }
        else if (data == "Down")
        {
            if (batC2.getPosition().top < windowHeight - batC2.getShape().getSize().y)
            {
                cout << endl<< "!! (SERVEUR)CLIENT 2 MOVE DOWN !!";
                batC2.moveDown();
            }
        }
        else
        {
            cout << endl<< "!! (SERVEUR)CLIENT 2 N'AS PAS MOVE !!" << endl;
            cout << endl<< "!! (SERVEUR) conservation position bat !!" << endl;
        }
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
        oss << batC2.getPosition().left << " " << batC2.getPosition().top;
    }
    else
    {
        oss << (windowWidth - ball.getPosition().left - ball.getShape().getSize().x) << " " << ball.getPosition().top << " ";
        oss << scoreC2 << " " << scoreC1 << " ";
        oss << (windowWidth - batC2.getPosition().left - batC2.getShape().getSize().x) << " " << batC2.getPosition().top << " ";
        oss << (windowWidth - batC1.getPosition().left - batC1.getShape().getSize().x) << " " << batC1.getPosition().top;
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



void *FctThreadReceiveClient1(void *setting)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    while (1)
    {
        char Data1[1024];
        int status1;

        // Reception du client 1
        status1 = client1->receiveNonBlocking(Data1, 100);
        handleThreadReceiveStatus(status1, Data1, true);
    }
    pthread_exit(NULL);
}

void *FctThreadReceiveClient2(void *setting)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    while (1)
    {
        char Data2[1024];
        int status2;

        // Reception du client 2
        status2 = client2->receiveNonBlocking(Data2, 100);
        handleThreadReceiveStatus(status2, Data2, false);
    }
    pthread_exit(NULL);
}

void handleThreadReceiveStatus(int status, char *Data, bool isClient1)
{
    struct timespec timestemp;
    if (isClient1)
    {

        if (status == TIMEOUT)
        {
            // cout<<endl<<"timeout = front queue="<<receivedQueue.front()<<endl;
            pthread_mutex_lock(&mutexReceiveClient1);
            if (!receivedQueueClient1.empty())// un message est bloque dans la file d'attente
            {
                receiveDataAvailableClient1 = true;
                pthread_cond_signal(&condReceiveClient1);
            }
            pthread_mutex_unlock(&mutexReceiveClient1);

        }
        else if (status != OK)
        {
            cout << "(CLIENT threadReceiveClient1)ERREUR reception des données du serveur" << endl;
            pthread_mutex_lock(&mutexErreurRcv1);
            erreurRcv1 = true;
            pthread_cond_signal(&condErreur1);
            pthread_mutex_unlock(&mutexErreurRcv1);
            //break;
        }
        else
        {
            clock_gettime(CLOCK_REALTIME, &timestemp);
            cout << "(CLIENT threadReceiveClient1)Données reçues du serveur " << Data << endl;
            pthread_mutex_lock(&mutexReceiveClient1);
            string tmp(Data);
            //convertion timestemp en microsecondes
            long long microsec = timestemp.tv_sec * 1000000 + timestemp.tv_nsec / 1000;
            Input input;
            input.action = tmp;
            input.timestamp_ms = microsec;

            receivedQueueClient1.push(input);
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
            cout << "(CLIENT threadReceiveClient2)ERREUR reception des données du serveur" << endl;
            pthread_mutex_lock(&mutexErreurRcv2);
            erreurRcv2 = true;
            pthread_cond_signal(&condErreur2);
            pthread_mutex_unlock(&mutexErreurRcv2);
           // break;
        }
        else
        {
            clock_gettime(CLOCK_REALTIME, &timestemp);
            cout << "(CLIENT threadReceiveClient2)Données reçues du serveur " << Data << endl;
            pthread_mutex_lock(&mutexReceiveClient2);
            string tmp(Data);
            //convertion timestemp en microsecondes
            long long microsec = timestemp.tv_sec * 1000000 + timestemp.tv_nsec / 1000;
            Input input;
            input.action = tmp;
            input.timestamp_ms = microsec;

            receivedQueueClient2.push(input);
            receiveDataAvailableClient2 = true;
            pthread_cond_signal(&condReceiveClient2);
            pthread_mutex_unlock(&mutexReceiveClient2);
        }
    }
}


int receiveDataClient1(Input& input)
{
    pthread_mutex_lock(&mutexReceiveClient1);
    while (!receiveDataAvailableClient1 && !erreurRcv1) 
    {
        pthread_cond_wait(&condReceiveClient1, &mutexReceiveClient1);
    }

    if (erreurRcv1)
    {
        pthread_mutex_unlock(&mutexReceiveClient1);
        cout<<endl<<"(SERVEUR)[CLIENT1] fct receiveDataClient1()) Erreur de Receive"<<endl;
        return ERROR;
    }

    if (!receivedQueueClient1.empty())
    {
        input = receivedQueueClient1.front();
        receivedQueueClient1.pop();
        cout << "(SERVEUR)[CLIENT1] fct receiveDataClient1()Données reçues du serveur: " << input.action << " + timestemp(ms): "<<input.timestamp_ms<< endl;
    }
    else
    {
        cout << "(SERVEUR)[CLIENT1] La file d'attente est vide, aucun traitenment a faire" << endl;
        //return ERROR; // Ou un autre code d'erreur approprié
    }

    receiveDataAvailableClient1 = false;
    pthread_mutex_unlock(&mutexReceiveClient1);
    return OK;
}

int receiveDataClient2(Input& input)
{
    pthread_mutex_lock(&mutexReceiveClient2);
    while (!receiveDataAvailableClient2 && !erreurRcv2) 
    {
        pthread_cond_wait(&condReceiveClient2, &mutexReceiveClient2);
    }

    if (erreurRcv2)
    {
        pthread_mutex_unlock(&mutexReceiveClient2);
        cout<<endl<<"(SERVEUR)[CLIENT2] fct receiveDataClient2() Erreur de Receive"<<endl;
        return ERROR;
    }

    if (!receivedQueueClient2.empty())
    {
        input = receivedQueueClient2.front();
        receivedQueueClient2.pop();
        cout << "(SERVEUR)[CLIENT2] fct receiveDataClient2() Données reçues du serveur: " << input.action << " + timestemp(ms): "<<input.timestamp_ms<< endl;
    }
    else
    {
        cout << "(SERVEUR)[CLIENT2] La file d'attente est vide, aucun traitenment a faire" << endl;
        //return ERROR; // Ou un autre code d'erreur approprié
    }

    receiveDataAvailableClient2 = false;
    pthread_mutex_unlock(&mutexReceiveClient2);
    return OK;
}