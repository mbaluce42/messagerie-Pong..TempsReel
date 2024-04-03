#include "GameClient.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include "bat.h"
#include "ball.h"
#include <string>
#include <thread>

using namespace std;
using namespace sf;

int main(int argc, char *argv[])
{
    int windowWidth = 1024;
    int windowHeight = 768;
    int scoreC1 = 0;
    int scoreC2 = 0;

    RenderWindow window(VideoMode(windowWidth, windowHeight), "Pong Client");
    Text hud;
    Font font;
    RectangleShape separators[16];

    GameClient client;
    bool focus;

    if (argc < 3)
    {
        cout << "Usage: " << argv[0] << " <server_address> <server_port>" << std::endl;
        return -1;
    }

    char* serverAddr = argv[1];
    int serverPort = atoi(argv[2]);

//client va se connecter au serveur
    int status = client.join(serverAddr, serverPort);
    if (status != OK)
    {
        cout << "(CLIENT)ERREUR de connexion au serveur" << endl;
        return status;
    }

    cout << "(CLIENT)Connection au serveur REUSSI" << std::endl;


    cout << endl<<"(CLIENT)initialisation du terrain ......................" << endl;

    // Create a HUD (Head Up Display)

    status= client.send( (char*)("HUD & SEPARATOR") );
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR envoi de la commande HUD & SEPARATOR au serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Commande HUD & SEPARATOR envoyé au serveur"<<endl;
    }
    char graph[1024]="";
    status=client.receive(graph);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR reception des données HUD du serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Données HUD reçues du serveur"<<endl;
        //cout<<endl<< graph << endl;
    }

    istringstream iss(graph);
    string fontFilename;
    int characterSize;  Uint32 fillColor;
    float positionX, positionY;

    iss >> fontFilename >> characterSize >> fillColor >> positionX >> positionY;
    font.loadFromFile(fontFilename);
    hud.setFont(font);
    hud.setCharacterSize(characterSize);
    hud.setFillColor(sf::Color(fillColor));
    hud.setPosition(Vector2f(positionX, positionY));

    // Create separator area
    memset(graph, 0, 1024);

    status=client.receive(graph);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR reception des données SEPARATOR du serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Données SEPARATOR reçues du serveur"<<endl;
        //cout<<endl<< graph << endl;
    }

    iss=istringstream(graph);


    for (int i = 0; i<16;i++)
    {
        float sizeX, sizeY, positionX, positionY;
        iss >> sizeX >> sizeY >> positionX >> positionY;
        separators[i].setSize(Vector2f(sizeX, sizeY));
        separators[i].setPosition(Vector2f(positionX, positionY));
    }

    cout << "(CLIENT)Terrain initialisé" << endl;

    while (window.isOpen())
    {
        Event event;

        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                status= client.send( (char*)("CLOSED"));
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi AU REVOIR au serveur (CLOSED)"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT)Message envoyé au serveur (CLOSED)"<<endl;
                    window.close(); 
                    cout<<endl<<"(CLIENT)Fenetre fermée" << endl;
                    return 0;
                }
            }
            else if(event.type == sf::Event::GainedFocus) {focus=true; cout<<endl<<"(CLIENT)Fenetre active" << endl;}
            else if(event.type == sf::Event::LostFocus) {focus=false; cout<<endl<<"(CLIENT)Fenetre non active" << endl;}
        }
        if(focus)
        {
            if (Keyboard::isKeyPressed(sf::Keyboard::Escape))
            {
                status= client.send( (char*)("ESC"));
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi ESC au serveur"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT)Message envoyé au serveur (ESC) succes"<<endl;
                    cout<<"(CLIENT)Fin de connexion " << endl;
                    window.close();
                    break;
                }
            }
            if (Keyboard::isKeyPressed(Keyboard::Up))
            {
                status= client.send( (char*)("Up") );
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi de la position du bat au serveur (Up)"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT)Message envoyé au serveur (Up) succes"<<endl;
                }
            }
            else if (Keyboard::isKeyPressed(Keyboard::Down))
            {
                status=client.send( (char*)("Down") );
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi de la position du bat au serveur (Down)"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT)Message envoye au serveur (Down) succes"<<endl;
                }
            }
            else
            {
                cout<<endl<<"(CLIENT) fenetre active mais AUCUN MOUV" << endl;
                status=client.send( (char*)("NOT") );
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi de la position du enemyBat au serveur (NOT)"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT)Message envoyé au serveur (NOT) succes "<<endl;
                }
            }
        }
        else
        {
            cout<<endl<<"(CLIENT)Fenetre non active" << endl;
            status=client.send( (char*)("NOT") );
            if (status != OK)
            {
                cout<<"(CLIENT)ERREUR envoi de la position du enemyBat au serveur (NOT)"<<endl;
                return status;
            }
            else
            {
                cout<<"(CLIENT)Message envoyé au serveur (NOT) succes"<<endl;
            }
        }
        cout<<"En cours de construction du terrain"<<endl;

        char AllData[1024]="";
        status= client.receive(AllData);
        if(status != OK)
        {
            cout<<"(CLIENT)ERREUR reception des données du serveur"<<endl;
            return status;
        }
        else
        {
            cout<<"(CLIENT)Données FINAL(ball,bats et score) reçues du serveur: "<<endl;
            cout<<endl<< AllData << endl;
        }
        if(strcmp(AllData, "ESC") == 0)
        {
            cout<<"(CLIENT)Fin de connexion Recu(ESC)" << endl;
            cout << "(CLIENT)Fin de connexion confirmée" << endl;
            break;
        }
        else if(strcmp(AllData, "CLOSED") == 0)
        {
            cout<<"(CLIENT)Fin de connexion Recu (CLOSED)" << endl;
            cout << "(CLIENT)Fin de connexion confirmée" << endl;
            break;
        }

        //deserialisation des données
        RectangleShape ballShape, batC1Shape, batC2Shape;

        istringstream iss(AllData);// flux iss  va permettre d'extrire les variable de la chaine de caractère
        float ballLeft, ballTop,
              batC1Left, batC1Top, 
              batC2Left, batC2Top;

        iss >> ballLeft >> ballTop
            >> scoreC1 >> scoreC2
            >> batC1Left >> batC1Top 
            >> batC2Left >> batC2Top;


        Ball ball(ballLeft, ballTop);
        Bat batC1(batC1Left, batC1Top);
        Bat batC2(batC2Left, batC2Top);

        std::stringstream ss;
        ss << scoreC1<< "\t" << scoreC2;
        
        hud.setString(ss.str());

        // Clear everything from the last frame
        window.clear(Color(0, 0, 0,255));
        //draw everything
        window.draw(batC1.getShape());
        window.draw(ball.getShape());
        window.draw(batC2.getShape());

        // Draw our score
        window.draw(hud);

        // draw separator
        for (int i = 0; i<16;i++){
            window.draw(separators[i]);
        }
        // Show everything we just drew
        window.display();

        cout<<endl<<"!!! (CLIENT) terrain construit avec succes !!!"<<endl;
    }

    return 0;
}