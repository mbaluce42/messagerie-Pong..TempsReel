#include "GameServer.h"
#include <iostream>

#include <string> // Pour les string
#include <cstring>
#include "bat.h"
#include "ball.h"
#include "Status.h"
#include <thread>

using namespace std;

int main(int argc, char *argv[])
{
    int status;
    int windowWidth = 1024;
    int windowHeight = 768;

    char buffer[1024];

    if(argc < 2)
    {
        cout<<endl <<"(SERVEUR)!!! Merci d'entrer Un numero de port(argument) pour lancer le serveur !!!"<< endl;
        return -1;
    }

    // Conv l'argument du port en entier
    int port = atoi(argv[1]);

    GameServer server(port);

    status=server.initialize();
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

    case BIND_ERROR:// si port deja utilisé par un autre processus
        cout << "(SERVEUR)ERREUR de bind" << endl;
        return status;
        break;
    case LISTEN_ERROR://impossible de passer le serveur(port) en mode ecoute
        cout << "(SERVEUR)ERREUR de listen" << endl;
        return status;
        break;

    case OK:
        cout << "(SERVEUR) ERREUR,Serveur initialisé avec succes" << endl;
        break;
    
    default:
        return -1;
        break;
    }

    cout << "(SERVEUR)En attente de connexion des clients(Joueur) :" << endl;
    GameClient client1;
    status=server.acceptClient(&client1);
    if(status == OK){cout << "(SERVEUR)Client 1 connecté" << endl;}
    else
    {
        cout << "(SERVEUR)ERREUR de connexion client 1" << endl;
        return status;
    }

    GameClient client2;
    status=server.acceptClient(&client2);
    if(status == OK){ cout << "(SERVEUR)Client 2 connecté" << endl;}
    else
    {
        cout << "(SERVEUR)ERREUR de connexion client 2" << endl;
        return status;
    }


    //init game
    cout << "(SERVEUR)Initialisation du jeu" << endl;
    Bat batC1 (0, 768/2);
    Bat batC2(windowWidth-batC1.getShape().getSize().x, windowHeight/2);
    Ball ball(windowWidth / 2, windowHeight/2);
    bool start = true; 
    int scoreC1=0, scoreC2=0;

    cout << "(SERVEUR)Jeu a commencé" << endl;
    ball.start();

    while (start==true)
    {
        //recoit les positions des bats
        cout << "(SERVEUR)En attente d'avoir les positions des bats des clients" << endl;
        char batData[1024]; char enemyBatData[1024];
        string batSendData = "";
        //-------------------------------recoit les positions de la bats client 1 -----------------------------------------------
        status=client1.receive(batData);
        if(status == OK)
        {
            cout << "(SERVEUR)Position bat client 1 reçu" << endl;
            // Analyser les données
            istringstream iss(batData);// flux iss  va permettre d'extrire les variable 
            string movType;
            // Récupérer la position
            iss >>movType;
            cout << "movType : " << movType << endl;
            if(movType=="Up")
            {
                if(batC1.getPosition().top > 0)
                {
                    cout<<endl<< "!! (SERVEUR)CLIENT 1 MOVE UP !!";
                    batC1.moveUp();
                }
            }
            else if (movType=="Down")
            {
                if(batC1.getPosition().top < windowHeight - batC1.getShape().getSize().y)
                {
                    cout<<endl<< "!! (SERVEUR)CLIENT 1 MOVE DOWN !!";
                    batC1.moveDown();
                }
            }
            else if(movType=="NOT")
            {
                cout <<endl<< "!! (SERVEUR)CLIENT 1 N'AS PAS MOVE !!" << endl;
                cout<<endl<< "!! (SERVEUR) conservation position bat !!";
            }
        }
        else
        {
            cout << "(SERVEUR)ERREUR de reception position bat client 1" << endl;
            return status;
        } 
        //-------------------------------recoit les positions de la bats client 2 -----------------------------------------------

        status=client2.receive(enemyBatData);
        if(status == OK)
        {
            cout << "(SERVEUR)Position bat client 2 reçu" << endl;
            // Analyser les données
            istringstream iss(enemyBatData);// flux iss  va permettre d'extrire les variable 
            string movType;
            // Récupérer la position
            iss >>movType ;
            if(movType=="Up")
            {
                if(batC2.getPosition().top > 0)
                {
                    cout<< "!! (SERVEUR)CLIENT 2 MOVE UP !!"<<endl;
                    batC2.moveUp();
                }
            }
            else if (movType=="Down")
            {
                if(batC2.getPosition().top < windowHeight - batC2.getShape().getSize().y)
                {
                    cout<<"!! (SERVEUR)CLIENT 2 MOVE DOWN !!"<<endl;
                    batC2.moveDown();
                }
            }
            else if(movType=="NOT")
            {
                cout <<endl<< "!! (SERVEUR)CLIENT 2 N'AS PAS MOVE !!" << endl;
                cout<<endl<< "!! (SERVEUR) conservation position bat !!"<<endl;
            }
        }
        else
        {
            cout << "(SERVEUR)ERREUR de reception position bat client 2" << endl;
            return status;
        } 


        if (ball.getPosition().top > windowHeight | ball.getPosition().top < 0)
        {
            // reverse the ball direction
            ball.reboundTopOrBot();
        }

        // Handle ball hitting side
        if (ball.getPosition().left < 0)
        {
            ball.hitSide(windowWidth/2, windowHeight/2);
            scoreC2++;    
        }
        // Handle ball hitting side
        if (ball.getPosition().left > windowWidth)
        {
            ball.hitSide(windowWidth/2, windowHeight/2);
            scoreC1++;        
        }
            
        // Has the ball hit the bat?
        if (ball.getPosition().intersects(batC1.getPosition()) || ball.getPosition().intersects(batC2.getPosition()))
        {
            ball.reboundBat();
        }
        //send all info to clients

        float oldPosX = ball.getPosition().left;
        batC1.update();
        batC2.update();
        ball.update();

        batSendData="";       
        string ballData =""+ to_string(ball.getPosition().left)+" "+ to_string(ball.getPosition().top) +" "+to_string(ball.getShape().getSize().x) +" "+ to_string(ball.getShape().getSize().y)+ " ";
        string score="" + to_string(scoreC1)+" "+ to_string(scoreC2)+" ";
        batSendData += to_string(batC1.getPosition().left)+" "+ to_string(batC1.getPosition().top) +" "+to_string(batC1.getShape().getSize().x) +" "+ to_string(batC1.getShape().getSize().y)+ " ";
        batSendData += to_string(batC2.getPosition().left)+" "+ to_string(batC2.getPosition().top) +" "+to_string(batC2.getShape().getSize().x) +" "+ to_string(batC2.getShape().getSize().y)+ " ";

        
        status = client1.send((char*)(ballData + score +batSendData).c_str());
        if(status != OK)
        {
            cout <<endl<< "(SERVEUR)ERREUR d'envoi position ball et bats vers client 1" << endl;
            return status;
        }
        else
        {
            cout << endl<<"(SERVEUR)Position ball[4] score[2] bats[8] vers client 1 envoyé avec succes:" << endl;
            cout<<endl<<ballData + batSendData<<endl;
        }

       /* Bat bat (0, windowHeight/2);
    Bat enemy_bat(windowWidth-bat.getShape().getSize().x, windowHeight/2);*/


        batSendData="";
        score="" + to_string(scoreC2)+" "+ to_string(scoreC1)+" ";
        batSendData += to_string(windowWidth - batC2.getPosition().left - batC2.getShape().getSize().x)+" "+ to_string(batC2.getPosition().top) +" "+to_string(batC2.getShape().getSize().x) +" "+ to_string(batC2.getShape().getSize().y)+ " ";
        batSendData += to_string(windowWidth - batC1.getPosition().left - batC1.getShape().getSize().x)+" "+ to_string(batC1.getPosition().top) +" "+to_string(batC1.getShape().getSize().x) +" "+ to_string(batC1.getShape().getSize().y)+ " ";

        status = client2.send((char*)(ballData + score +batSendData).c_str());
        if(status != OK)
        {
            cout << endl<<"(SERVEUR)ERREUR d'envoi position ball et bats vers client 2" << endl;
            return status;
        }
        else
        {
            cout << endl<<"(SERVEUR)Position ball[4] score[2] bats[8] vers client 2 envoyé avec succes:" << endl;
            cout<<ballData + score+ batSendData<<endl;
        }

    }// This is the end of the "while" loop

}