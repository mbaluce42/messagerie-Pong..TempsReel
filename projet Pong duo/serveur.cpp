#include "GameServer.h"
#include <iostream>

#include <string> // Pour les string
#include <cstring>
#include "bat.h"
#include "ball.h"
#include "Status.h"

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

    /*if(strlen(argv[1]) > 5)
    {
        cout<<endl <<"!!! Le numero de port doit avoir min et max 5 chiffres  !!!"<< endl;
        cout << "!!! Exemple : ./serveur 8080 !!!" << endl;
        return -1;
    }*/
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
            /*if(batData == "CLOSE")
            {
                cout << "(SERVEUR)Client 1 a fermé la connexion" << endl;

                return 0;
            }*/
            cout << "(SERVEUR)Position bat client 1 reçu" << endl;
            // Analyser les données
            istringstream iss(batData);// flux iss  va permettre d'extrire les variable 
            string movType;
            float posLeft, posTop, sizeX, sizeY;
            // Récupérer la position
            iss >>movType >>posLeft >> posTop>> sizeX>>sizeY;
            if(movType=="1Up")
            {
                batC1= Bat(posLeft, posTop);
                if(batC1.getPosition().top > 0)
                {
                    batC1.moveUp();
                    /*batSendData="";
                    batSendData += to_string(batC1.getPosition().left)+","+ to_string(batC1.getPosition().top);
                    status = client1.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 1 vers lui meme" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 1 vers lui meme envoyé avec succes" << endl;
                    }
                    status = client2.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 1 vers cleint 2" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 1 vers client 2 envoyé avec succes" << endl;
                    }*/
                }
            }
            else if(movType=="2UP")
            {
                batC2= Bat(posLeft, posTop);
                if(batC2.getPosition().top > 0)
                {
                    batC2.moveUp();
                    /*batSendData="";
                    batSendData += to_string(batC2.getPosition().left)+","+ to_string(batC2.getPosition().top);
                    status = client2.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 2 vers lui meme" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 2 vers lui meme envoyé avec succes" << endl;
                    }
                    status = client1.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 2 vers cleint 1" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 2 vers client 1 envoyé avec succes" << endl;
                    }*/
                }
            }
            else if (movType=="1Down")
            {
                batC1= Bat(posLeft, posTop);
                if(batC1.getPosition().top < windowHeight - batC1.getShape().getSize().y)
                {
                    batC1.moveDown();
                    /*batSendData="";
                    batSendData += to_string(batC1.getPosition().left)+","+ to_string(batC1.getPosition().top);
                    status = client1.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 1 vers lui meme" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 1 vers lui meme envoyé avec succes" << endl;
                    }
                    status = client2.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 1 vers cleint 2" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 1 vers client 2 envoyé avec succes" << endl;
                    }*/
                }
            }
            
            else if (movType=="2Down")
            {
                batC2= Bat(posLeft, posTop);
                if(batC2.getPosition().top < windowHeight - batC2.getShape().getSize().y)
                {
                    batC2.moveDown();
                    /*batSendData="";
                    batSendData += to_string(batC2.getPosition().left)+","+ to_string(batC2.getPosition().top);
                    status = client2.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 2 vers lui meme" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 2 vers lui meme envoyé avec succes" << endl;
                    }
                    status = client1.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 2 vers cleint 1" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 2 vers client 1 envoyé avec succes" << endl;
                    }*/
                }   
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
            float posLeft, posTop, sizeX, sizeY;
            // Récupérer la position
            iss >>movType >>posLeft >> posTop>> sizeX>>sizeY;
            if(movType=="1Up")
            {
                batC2= Bat(posLeft, posTop);
                if(batC2.getPosition().top > 0)
                {
                    batC2.moveUp();
                    /*batSendData="";
                    batSendData += to_string(batC2.getPosition().left)+","+ to_string(batC2.getPosition().top);
                    status = client2.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 2 vers lui meme" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 2 vers lui meme envoyé avec succes" << endl;
                    }
                    status = client1.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 2 vers client 1" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 2 vers client 1 envoyé avec succes" << endl;
                    }*/
                }
            }
            else if(movType=="2UP")
            {
                batC1= Bat(posLeft, posTop);
                if(batC1.getPosition().top > 0)
                {
                    batC1.moveUp();
                    /*batSendData="";
                    batSendData += to_string(batC1.getPosition().left)+","+ to_string(batC1.getPosition().top);
                    status = client1.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 1 vers lui meme" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 1 vers lui meme envoyé avec succes" << endl;
                    }
                    status = client2.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 1 vers client 2" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 1 vers client 2 envoyé avec succes" << endl;
                    }*/
                }
            }
            else if (movType=="1Down")
            {
                batC2= Bat(posLeft, posTop);
                if(batC2.getPosition().top < windowHeight - batC2.getShape().getSize().y)
                {
                    batC2.moveDown();
                    /*batSendData="";
                    batSendData += to_string(batC2.getPosition().left)+","+ to_string(batC2.getPosition().top);
                    status = client2.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 2 vers lui meme" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 2 vers lui meme envoyé avec succes" << endl;
                    }
                    status = client1.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 2 vers client 1" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 2 vers client 1 envoyé avec succes" << endl;
                    }*/
                }
            }
            else if (movType=="2Down")
            {
                batC1= Bat(posLeft, posTop);
                if(batC1.getPosition().top < windowHeight - batC1.getShape().getSize().y)
                {
                    batC1.moveDown();
                    /*batSendData="";
                    batSendData += to_string(batC1.getPosition().left)+","+ to_string(batC1.getPosition().top);
                    status = client1.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 1 vers lui meme" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 1 vers lui meme envoyé avec succes" << endl;
                    }
                    status = client2.send((char*)batSendData.c_str());
                    if(status != OK)
                    {
                        cout << "(SERVEUR)ERREUR d'envoi position bat client 1 vers client 2" << endl;
                        return status;
                    }
                    else
                    {
                        cout << "(SERVEUR)Position bat client 1 vers client 2 envoyé avec succes" << endl;
                    }*/
                }
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
        batC1.update();//bat.update();
        batC2.update();//enemy_bat.update();
        ball.update();
        /*std::stringstream ss;
        ss << scoreC1<< "\t" << scoreC2;*/
        



        string ballData="";
        batSendData="";       
        ballData += to_string(ball.getPosition().left)+","+ to_string(ball.getPosition().top)+","+ to_string(scoreC1)+","+ to_string(scoreC2)+",";
        batSendData += to_string(batC1.getPosition().left)+","+ to_string(batC1.getPosition().top)+",";
        batSendData += to_string(batC2.getPosition().left)+","+ to_string(batC2.getPosition().top);
        
        status = client1.send((char*)(ballData + batSendData).c_str());
        if(status != OK)
        {
            cout << "(SERVEUR)ERREUR d'envoi position ball vers client 1" << endl;
            return status;
        }
        else
        {
            cout << "(SERVEUR)Position ball vers client 1 envoyé avec succes" << endl;
        }

        batSendData="";
        batSendData += to_string(batC2.getPosition().left)+","+ to_string(batC2.getPosition().top)+",";
        batSendData += to_string(batC1.getPosition().left)+","+ to_string(batC1.getPosition().top);
        
        status = client2.send((char*)(ballData + batSendData).c_str());
        if(status != OK)
        {
            cout << "(SERVEUR)ERREUR d'envoi position ball vers client 2" << endl;
            return status;
        }
        else
        {
            cout << "(SERVEUR)Position ball vers client 2 envoyé avec succes" << endl;
        }

        
    }// This is the end of the "while" loop

}