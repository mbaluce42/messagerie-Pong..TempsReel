#include "GameClient.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include "bat.h"
#include "ball.h"
#include <string>
#include <thread>

using namespace std;
using namespace sf;

int scoreC1 = 0;
int scoreC2 = 0;
int windowWidth = 1024;
int windowHeight = 768;

int initHud(GameClient& client, Text& hud, Font& font);
int initSeparator(GameClient& client, RectangleShape (&separators)[16]);
int initTerrain(GameClient& client, Text& hud, Font& font, RectangleShape (&separators)[16]);
int sendEvent(GameClient& client, bool focus);
void finalDataDeserialization(char* AllData, Ball& ball, Bat& batC1, Bat& batC2, stringstream& ss);
void afficheTerrain(Text &hud, RectangleShape (&separators)[16], RenderWindow &window,stringstream& ss, Ball& ball, Bat& batC1, Bat& batC2);
int stopConnection(GameClient& client);


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Usage: " << argv[0] << " <server_address> <server_port>" << endl;
        return -1;
    }
    char* ipAdresse = argv[1];
    int port = atoi(argv[2]);
    GameClient client;

    //client va se connecter au serveur
    int status = client.join(ipAdresse, port);
    if (status != OK)
    {
        cout << "(CLIENT)ERREUR de connexion au serveur" << endl;
        return status;
    }

    cout << "(CLIENT)Connection au serveur REUSSI" << endl;
    
    RenderWindow window(VideoMode(windowWidth, windowHeight), "Pong Client");
    Text hud;
    Font font;
    RectangleShape separators[16];

    Ball ball(windowWidth / 2, windowHeight / 2);
    Bat batC1(10, windowHeight / 2);
    Bat batC2(windowWidth - 10, windowHeight / 2);
    bool focus;
    stringstream ss;


    cout << endl<<"(CLIENT)initialisation du terrain ......................" << endl;

    // Create a HUD (Head Up Display)
    status= initTerrain(client, hud, font, separators);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR initialisation du terrain"<<endl;
        return status;
    }
    else
        cout << "(CLIENT)HUD et SEPARATOR initialisé" << endl;

    //afficheTerrain(hud, separators, window, ss, ball, batC1, batC2);

    cout << "(CLIENT)Terrain initialisé" << endl;



    while (window.isOpen())
    {
        Event event;

        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                status=stopConnection(client);
                if(status != OK){return status;}
                else{window.close(); cout<<endl<<"(CLIENT)Fenetre fermée" << endl; return 0; }
            }
            else if(event.type == sf::Event::GainedFocus) {focus=true; cout<<endl<<"(CLIENT)Fenetre active" << endl;}
            else if(event.type == sf::Event::LostFocus) {focus=false; cout<<endl<<"(CLIENT)Fenetre non active" << endl;}
        }
        status= sendEvent(client, focus);
        if(status != OK)
        {
            if(status == 99)
            {
                //cout<<"(CLIENT)Fin de connexion"<<endl;
                window.close();
                break;
            }
            cout<<"(CLIENT)ERREUR envoi des evenements au serveur"<<endl;
            return status;
        }
        else
        {
            cout<<"(CLIENT)Evenements envoyés au serveur"<<endl;
        }

        cout<<"En cours de construction du terrain"<<endl;

        char AllData[1024];
        status= client.receive(AllData);
        if(status != OK)
        {
            cout<<"(CLIENT)ERREUR reception des données du serveur"<<endl;
            cout<<endl<< AllData << endl;
            return status;
        }

        if(strcmp(AllData, "STOP") == 0)
        {
            cout<<"(CLIENT)Fin de connexion Recu(STOP)" << endl;
            cout << "(CLIENT)Fin de connexion confirmée" << endl;
            break;
        }
        
        
        cout<<"(CLIENT)Données FINAL reçues du serveur "<<endl;
        finalDataDeserialization(AllData, ball, batC1, batC2, ss);

        afficheTerrain(hud, separators, window, ss, ball, batC1, batC2);

        cout<<endl<<"!!! (CLIENT) terrain construit avec succes !!!"<<endl;
    }

    return 0;
}



int initHud(GameClient& client, Text& hud, Font& font)
{
    int status= client.send( (char*)("HUD & SEPARATOR") );
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR envoi de la commande HUD & SEPARATOR au serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Commande HUD & SEPARATOR envoyé au serveur"<<endl;
    }
    char graph[1024];
    status=client.receive(graph);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR reception des données HUD & SEPARATOR du serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Données HUD & SEPARATOR reçues du serveur"<<endl;
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
    return OK;

}
int initSeparator(GameClient& client, RectangleShape (&separators)[16])
{
    char graph[1024];
    int status=client.receive(graph);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR reception des données SEPARATOR du serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Données SEPARATOR reçues du serveur"<<endl;
    }

    istringstream iss(graph);

    for (int i = 0; i<16;i++)
    {
        float sizeX, sizeY, positionX, positionY;
        iss >> sizeX >> sizeY >> positionX >> positionY;
        separators[i].setSize(Vector2f(sizeX, sizeY));
        separators[i].setPosition(Vector2f(positionX, positionY));
    }
    return OK;

}
int initTerrain(GameClient& client, Text& hud, Font& font, RectangleShape (&separators)[16])
{
    int status= initHud(client, hud, font);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR initialisation du HUD"<<endl;
        return status;
    }
    status= initSeparator(client, separators);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR initialisation du SEPARATOR"<<endl;
        return status;
    }
    return OK;
}

int sendEvent(GameClient &client, bool focus)
{
    int status;
    if (focus)
    {
        if (Keyboard::isKeyPressed(sf::Keyboard::Escape))
        {
            status=stopConnection(client);
            if(status != OK){return status;}
            else{return 99;}
        }
        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            status = client.send((char *)("Up"));
            if (status != OK)
            {
                cout << "(CLIENT)ERREUR envoi de la position du bat au serveur (Up)" << endl;
                return status;
            }
            else
            {
                cout << "(CLIENT)Message envoyé au serveur (Up) succes" << endl;
            }
        }
        else if (Keyboard::isKeyPressed(Keyboard::Down))
        {
            status = client.send((char *)("Down"));
            if (status != OK)
            {
                cout << "(CLIENT)ERREUR envoi de la position du bat au serveur (Down)" << endl;
                return status;
            }
            else
            {
                cout << "(CLIENT)Message envoye au serveur (Down) succes" << endl;
            }
        }
        else
        {
            cout << endl
                 << "(CLIENT) fenetre active mais AUCUN MOUV" << endl;
            status = client.send((char *)("NOT"));
            if (status != OK)
            {
                cout << "(CLIENT)ERREUR envoi de la position du enemyBat au serveur (NOT)" << endl;
                return status;
            }
            else
            {
                cout << "(CLIENT)Message envoyé au serveur (NOT) succes " << endl;
            }
        }
    }
    else
    {
        cout << endl
             << "(CLIENT)Fenetre non active" << endl;
        status = client.send((char *)("NOT"));
        if (status != OK)
        {
            cout << "(CLIENT)ERREUR envoi de la position du enemyBat au serveur (NOT)" << endl;
            return status;
        }
        else
        {
            cout << "(CLIENT)Message envoyé au serveur (NOT) succes" << endl;
        }
    }
    return OK;
}

void finalDataDeserialization(char* AllData, Ball& ball, Bat& batC1, Bat& batC2, stringstream& ss)
{

    istringstream iss(AllData); // flux iss  va permettre d'extrire les variable de la chaine de caractère
    float ballLeft, ballTop,
        batC1Left, batC1Top,
        batC2Left, batC2Top;

    iss >> ballLeft >> ballTop >> scoreC1 >> scoreC2 >> batC1Left >> batC1Top >> batC2Left >> batC2Top;

    ball.setPosition(ballLeft, ballTop);
    batC1.setPosition(batC1Left, batC1Top);
    batC2.setPosition(batC2Left, batC2Top);
    ss.str("");

    ss << scoreC1 << "\t" << scoreC2;
}

void afficheTerrain(Text &hud, RectangleShape (&separators)[16], RenderWindow &window,stringstream& ss, Ball& ball, Bat& batC1, Bat& batC2)
{
    hud.setString(ss.str());

    // Clear everything from the last frame
    window.clear(Color(0, 0, 0, 255));
    // draw everything
    window.draw(batC1.getShape());
    window.draw(ball.getShape());
    window.draw(batC2.getShape());

    // Draw our score
    window.draw(hud);

    // draw separator
    for (int i = 0; i < 16; i++)
    {
        window.draw(separators[i]);
    }
    // Show everything we just drew
    window.display();
}
int stopConnection(GameClient& client)
{
    int status= client.send( (char*)("STOP"));
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR envoi AU REVOIR au serveur (STOP)"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Message envoyé au serveur (STOP)"<<endl;
        cout << "(CLIENT)Fin de connexion " << endl;
        return OK;
    }
}