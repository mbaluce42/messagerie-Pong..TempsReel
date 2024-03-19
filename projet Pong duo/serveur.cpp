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


int main(int argc, char *argv[])
{
    int status;
    int windowWidth = 1024;
    int windowHeight = 768;

    char buffer[1024];

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
        cout << "(SERVEUR)Serveur initialisé avec succes" << endl;
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
    cout << "(SERVEUR)Initialisation du jeu en cours ..........." << endl;

    status=client1.receive(buffer);
    if(status != OK)
    {
        cout << "(SERVEUR)ERREUR de reception de la commande HUD & SEPARATOR du client 1" << endl;
        return status;
    }
    else
    {
        cout << "(SERVEUR)Commande HUD & SEPARATOR reçu du client 1" << endl;
    }

    ostringstream oss;
    oss << "OpenSans-Bold.ttf"<<" " << hud.getCharacterSize() << " " << hud.getFillColor().toInteger() << " " << hud.getPosition().x << " " << hud.getPosition().y;
    status = client1.send((char*) oss.str().c_str());
    if(status != OK)
    {
        cout <<endl<< "(SERVEUR)ERREUR d'envoi GraphDATA HUD vers client 1" << endl;
        return status;
    }
    else
    {
        cout << endl<<"(SERVEUR)GraphDATA HUD vers client 1 envoyé avec succes:" << endl;
        cout<<endl<<oss.str()<<endl;
    }

    ostringstream oss2; 

    for (int i = 0; i < 16; i++) 
    {
        oss2 << separators[i].getSize().x << " " << separators[i].getSize().y << " ";
        oss2 << separators[i].getPosition().x << " " << separators[i].getPosition().y << " ";
    }
    status = client1.send((char *)oss2.str().c_str());
    if (status != OK)
    {
        cout << endl
             << "(SERVEUR)ERREUR d'envoi SEPARATOR vers client 1" << endl;
        return status;
    }
    else
    {
        cout << endl
             << "(SERVEUR)SEPARATOR vers client 1 envoyé avec succes:" << endl;
        cout << endl<< oss2.str() << endl;
    }


    status = client2.receive(buffer);
    if (status != OK)
    {
        cout << "(SERVEUR)ERREUR de reception de la commande HUD & SEPARATOR du client 2" << endl;
        return status;
    }
    else
    {
        cout << "(SERVEUR)Commande HUD & SEPARATOR reçu du client 2" << endl;
    }


    status = client2.send((char *)oss.str().c_str());
    if (status != OK)
    {
        cout << endl<< "(SERVEUR)ERREUR d'envoi HUD vers client 2" << endl;
        return status;
    }
    else
    {
        cout << endl<< "(SERVEUR)HUD vers client 2 envoyé avec succes:" << endl;
        cout << endl<< oss.str() << endl;
    }

    status = client2.send((char *)oss2.str().c_str());
    if (status != OK)
    {
        cout << endl<< "(SERVEUR)ERREUR d'envoi SEPARATOR vers client 2" << endl;
        return status;
    }
    else
    {
        cout << endl<< "(SERVEUR)SEPARATOR vers client 2 envoyé avec succes:" << endl;
        cout << endl<< oss2.str() << endl;
    }

    Bat batC1 (0, 768/2);
    Bat batC2(windowWidth-batC1.getShape().getSize().x, windowHeight/2);
    Ball ball(windowWidth / 2, windowHeight/2);
    bool start = true; 
    int scoreC1=0, scoreC2=0;

    cout << "(SERVEUR)!!! Jeu initialisé avec succes !!!" << endl;
    cout << "(SERVEUR)!!! Jeu commence !!!" << endl;
    ball.start();

    while (start==true)
    {
        //recoit les positions des bats
        cout << "(SERVEUR)En attente d'avoir les positions des bats des clients" << endl;
        char batData[1024]; char enemyBatData[1024];
        string batSendData = ""; string movType="";
        //-------------------------------recoit les positions de la bats client 1 -----------------------------------------------
        status=client1.receive(batData);
        if(status == OK)
        {
            cout << "(SERVEUR)Position bat client 1 reçu" << endl;
            // Analyser les données
            istringstream iss(batData);// flux iss  va permettre d'extrire les variable ;
            movType=batData;
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
            movType=enemyBatData;
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

        oss =ostringstream();
        oss << ball.getPosition().left << " " << ball.getPosition().top << " ";
        oss << scoreC1 << " " << scoreC2 << " ";
        oss << batC1.getPosition().left << " " << batC1.getPosition().top << " " ;
        oss << batC2.getPosition().left << " " << batC2.getPosition().top;

        status = client1.send((char*)(oss.str().c_str()));
        if(status != OK)
        {
            cout <<endl<< "(SERVEUR)ERREUR d'envoi position ball et bats vers client 1" << endl;
            return status;
        }
        else
        {
            cout << endl<<"(SERVEUR)Position ball[2] score[2] bats[4] vers client 1 envoyé avec succes:" << endl;
            cout<<endl<<oss.str()<<endl;
        }

        oss =ostringstream();
        oss << (windowWidth - ball.getPosition().left - ball.getShape().getSize().x) << " " << ball.getPosition().top << " ";
        oss << scoreC2 << " " << scoreC1 << " ";
        oss << (windowWidth - batC2.getPosition().left - batC2.getShape().getSize().x) << " " << batC2.getPosition().top << " ";
        oss << (windowWidth - batC1.getPosition().left - batC1.getShape().getSize().x) << " " << batC1.getPosition().top;
        
        status = client2.send((char*)(oss.str().c_str()));
        if(status != OK)
        {
            cout << endl<<"(SERVEUR)ERREUR d'envoi position ball et bats vers client 2" << endl;
            return status;
        }
        else
        {
            cout << endl<<"(SERVEUR)Position ball[2] score[2] bats[4] vers client 2 envoyé avec succes:" << endl;
            cout<<oss.str()<<endl;
        }

    }// This is the end of the "while" loop

}