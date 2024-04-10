#include "GameServer.h"
#include <iostream>

#include <string> // Pour les string
#include <cstring>
#include "bat.h"
#include "ball.h"
#include "Status.h"
#include <thread>
#include <SFML/Graphics.hpp>

using namespace std;

int windowWidth = 1024;
int windowHeight = 768;

int initServer(GameServer& server);
int waitClient(GameServer& server, GameClient& client);
int initGame(GameClient& client1, GameClient& client2, Text& hud,string fontPath , RectangleShape (&separators)[16]);

int sendHudSeparator(GameClient& client, Text& hud, string fontPath, RectangleShape (&separators)[16]);

int AnalyseEvent(GameClient &client1, GameClient &client2, Bat &batC1, Bat &batC2);

void HandleBall(Ball &ball, Bat &batC1, Bat &batC2, int &scoreC1, int &scoreC2);

int SendInfoToClient(GameClient &client, Ball &ball, Bat &batC1, Bat &batC2, int scoreC1, int scoreC2, bool isClient1);

int SendAllInfoToClients(GameClient &client1, GameClient &client2, Ball &ball, Bat &batC1, Bat &batC2, int scoreC1, int scoreC2);

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


    cout << "(SERVEUR)En attente de connexion du client 1 puis 2 :" << endl;
    GameClient client1;
    status=waitClient(server, client1);
    if(status !=OK){ return status;}
    cout << "(SERVEUR)Client 1 connecté" << endl;

    GameClient client2;
    status=waitClient(server, client2);
    if(status !=OK){ return status;}
    cout << "(SERVEUR)Client 2 connecté" << endl;

    //init game
    cout << "(SERVEUR)Initialisation du jeu en cours ..........." << endl;

   status=initGame(client1, client2, hud, "OpenSans-Bold.ttf", separators);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'initialisation du jeu" << endl;
        return status;
    }

    cout << "(SERVEUR)!!! Jeu initialisé avec succes !!!" << endl;
    cout << "(SERVEUR)!!! Jeu commence !!!" << endl;
    ball.start();
    bool start = true;

    while (start==true)
    {
        cout << "(SERVEUR)En attente d'Even des clients ; 1 puis 2" << endl;
        status = AnalyseEvent(client1, client2, batC1, batC2);
        if (status != OK)
        {
            if (status == -99)
            {
                //cout << "(SERVEUR)Fin de connexion" << endl;
                break;
            }
            
            cout << "(SERVEUR)ERREUR d'analyse des Evenements" << endl;
            return status;
        }

        HandleBall(ball, batC1, batC2, scoreC1, scoreC2);
        //send all info to clients
        status= SendAllInfoToClients(client1, client2, ball, batC1, batC2, scoreC1, scoreC2);
        if(status != OK)
        {
            cout << "(SERVEUR)ERREUR d'envoi des positions ball et bats aux clients" << endl;
            return status;
        }

    }// This is the end of the "while" loop

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

int waitClient(GameServer& server, GameClient& client)
{
    int status;
    status = server.acceptClient(&client);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR de connexion client " << endl;
        return status;
    }

    return OK;
}

int initGame(GameClient& client1, GameClient& client2, Text& hud,string fontPath , RectangleShape (&separators)[16])
{
    int status;
    status = sendHudSeparator(client1, hud, fontPath, separators);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'initialisation de HUD & SEPARATOR pour client 1" << endl;
        return status;
    }

    status = sendHudSeparator(client2, hud, fontPath, separators);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'initialisation de HUD & SEPARATOR pour client 2" << endl;
        return status;
    }

    //je peux aussi envoyer les positions des bats et de la balle


    return OK;
}

int sendHudSeparator(GameClient& client, Text& hud, string fontPath, RectangleShape (&separators)[16])
{
    char Data[1024];
    int status=OK;
    status=client.receive(Data);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR de reception de la commande HUD & SEPARATOR du client 1" << endl;
        return status;
    }
    
    cout << "(SERVEUR)Commande HUD & SEPARATOR reçu du client 1" << endl;
    
    ostringstream oss_HUD;
    oss_HUD << fontPath<<" " << hud.getCharacterSize() << " " << hud.getFillColor().toInteger() << " " << hud.getPosition().x << " " << hud.getPosition().y;
    status = client.send((char*) oss_HUD.str().c_str());
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

    ostringstream oss_sepa; 

    for (int i = 0; i < 16; i++) 
    {
        oss_sepa << separators[i].getSize().x << " " << separators[i].getSize().y << " ";
        oss_sepa << separators[i].getPosition().x << " " << separators[i].getPosition().y << " ";
    }
    status = client.send((char *)oss_sepa.str().c_str());
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

    return OK;
}

int AnalyseEvent(GameClient &client1, GameClient &client2, Bat &batC1, Bat &batC2)
{
    char Data[1024];
    int status = OK;

    // Client 1
    status = client1.receive(Data);
    if (status == OK)
    {
        cout << "(SERVEUR)Even du client 1 reçu" << endl;
        if (strcmp(Data, "ESC") == 0)
        {
            cout << "(SERVEUR)Fin de connexion demandée par client 1" << endl;
            status = client2.send((char *)"ESC");
            if (status != OK)
            {
                cout << "(SERVEUR)ERREUR d'envoi de message ECHAP au client 2" << endl;
                return status;
            }
            cout << "(SERVEUR)Fin de connexion envoyée au client 2" << endl;
            cout << "(SERVEUR)Fin de connexion" << endl;
            return -99;
        }
        else if (strcmp(Data, "CLOSED") == 0)
        {
            cout << "(SERVEUR)Fin de connexion demandée par client 1" << endl;
            status = client2.send((char *)"CLOSED");
            if (status != OK)
            {
                cout << "(SERVEUR)ERREUR d'envoi de message CLOSED au client 2" << endl;
                return status;
            }
            cout << "(SERVEUR)Fin de connexion envoyée au client 2" << endl;
            cout << "(SERVEUR)Fin de connexion" << endl;
            return -99;
        }

        // Analyser les données du client 1
        string movType = Data;
        cout << "movType : " << movType << endl;
        if (movType == "Up")
        {
            if (batC1.getPosition().top > 0)
            {
                cout << endl << "!! (SERVEUR)CLIENT 1 MOVE UP !!";
                batC1.moveUp();
            }
        }
        else if (movType == "Down")
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
    status = client2.receive(Data);
    if (status == OK)
    {
        cout << "(SERVEUR)Even du client 2 reçu" << endl;
        if (strcmp(Data, "ESC") == 0)
        {
            cout << "(SERVEUR)Fin de connexion demandée par client 2" << endl;
            status = client1.send((char *)"ESC");
            if (status != OK)
            {
                cout << "(SERVEUR)ERREUR d'envoi de message ECHAP au client 1" << endl;
                return status;
            }
            cout << "(SERVEUR)Fin de connexion envoyée au client 1" << endl;
            cout << "(SERVEUR)Fin de connexion" << endl;
            return -99;
        }
        else if (strcmp(Data, "CLOSED") == 0)
        {
            cout << "(SERVEUR)Fin de connexion demandée par client 2" << endl;
            status = client1.send((char *)"CLOSED");
            if (status != OK)
            {
                cout << "(SERVEUR)ERREUR d'envoi de message CLOSED au client 1" << endl;
                return status;
            }
            cout << "(SERVEUR)Fin de connexion envoyée au client 1" << endl;
            cout << "(SERVEUR)Fin de connexion" << endl;
            return -99;
        }

        // Analyser les données du client 2
        string movType = Data;
        cout << "movType : " << movType << endl;
        if (movType == "Up")
        {
            if (batC2.getPosition().top > 0)
            {
                cout << endl << "!! (SERVEUR)CLIENT 2 MOVE UP !!";
                batC2.moveUp();
            }
        }
        else if (movType == "Down")
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


int SendInfoToClient(GameClient &client, Ball &ball, Bat &batC1, Bat &batC2, int scoreC1, int scoreC2, bool isClient1)
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

    int status = client.send((char*)oss.str().c_str());
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



int SendAllInfoToClients(GameClient &client1, GameClient &client2, Ball &ball, Bat &batC1, Bat &batC2, int scoreC1, int scoreC2)
{
    int status= SendInfoToClient(client1, ball, batC1, batC2, scoreC1, scoreC2, true);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'envoi position ball et bats vers client 1" << endl;
        return status;
    }
    status= SendInfoToClient(client2, ball, batC1, batC2, scoreC1, scoreC2, false);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR d'envoi position ball et bats vers client 2" << endl;
        return status;
    }
}