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

    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <server_address> <server_port>" << std::endl;
        return -1;
    }

    char* serverAddr = argv[1];
    int serverPort = atoi(argv[2]);

    

    // Initialize SFML window
    RenderWindow window(VideoMode(windowWidth, windowHeight), "Pong Client");

    Text hud;
    Font font;
    /*font.loadFromFile("OpenSans-Bold.ttf");
    hud.setFont(font);
    hud.setCharacterSize(75);
    hud.setFillColor(sf::Color::White);
    hud.setPosition(Vector2f((windowWidth/2)-100,0));*/
    RectangleShape separators[16];

    GameClient client;

    int status = client.join(serverAddr, serverPort);
    if (status != OK)
    {
        cout << "(CLIENT)ERREUR de connexion au serveur" << endl;
        return status;
    }

    cout << "(CLIENT)Connection au serveur REUSSI" << std::endl;

    bool focus;

    // Create a HUD (Head Up Display)

    status= client.send( (char*)("HUD") );
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR envoi de la commande HUD au serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Commande HUD envoyé au serveur"<<endl;
    }
    char graph[1024]="";
    status=client.receive(graph);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR reception des données du serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Données HUD reçues du serveur"<<endl;
        //cout<<endl<< graph << endl;
    }

    istringstream iss(graph);
    string fontFilename;
    int characterSize, fillColor;
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
        cout<<"(CLIENT)ERREUR reception des données du serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Données INITIAL(ball,bats et score) reçues du serveur"<<endl;
        //cout<<endl<< graph << endl;
    }

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
            if (event.type == Event::Closed){
                // Someone closed the window- bye
                string bye;

                status= client.send( (char*)(bye.c_str()) );
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi AU REVOIR au serveur (CLOSED)"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT) AU REVOIR envoyé au serveur (CLOSED)"<<endl;
                }

                window.close(); cout<<endl<<"(CLIENT)Fenetre fermée" << endl;}
        else if(event.type == sf::Event::GainedFocus) {focus=true; cout<<endl<<"(CLIENT)Fenetre active 1" << endl;}
        else if(event.type == sf::Event::LostFocus) {focus=false; cout<<endl<<"(CLIENT)Fenetre non active" << endl;}
        }

        if(focus)
        {
            if (Keyboard::isKeyPressed(sf::Keyboard::Escape))
            {
                // quit...
                // Someone closed the window- bye
                window.close();
            }


            cout<<endl<<"(CLIENT)Fenetre active 2" << endl;
            string batData="";
            string enemyBatData="";
            // Send bat position to the server
            if (Keyboard::isKeyPressed(Keyboard::Up))
            {
                batData += "Up ";

                status= client.send( (char*)(batData.c_str()) );
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi de la position du bat au serveur (1Up)"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT)Position du bat envoyé au serveur (1UP)"<<endl;
                }
            
            }
            else if (Keyboard::isKeyPressed(Keyboard::Down))
            {
                batData += "Down ";

                status=client.send( (char*)(batData.c_str()) );
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi de la position du bat au serveur (1Down)"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT)Position du bat envoyé au serveur (1Down)"<<endl;
                }
            }
            else
            {
                cout<<endl<<"(CLIENT)Fenetre non active" << endl;
                string notData="";
                notData += "NOT ";
                status=client.send( (char*)(notData.c_str()) );
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi de la position du enemyBat au serveur (NOT move)"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT)Position des bat envoyé au serveur (NOT move])"<<endl;
                }
            }
        }
        else
        {
            cout<<endl<<"(CLIENT)Fenetre non active" << endl;
            string notData="";
            notData += "NOT ";
            status=client.send( (char*)(notData.c_str()) );
            if (status != OK)
            {
                cout<<"(CLIENT)ERREUR envoi de la position du enemyBat au serveur (NOT move)"<<endl;
                return status;
            }
            else
            {
                cout<<"(CLIENT)Position des bat envoyé au serveur (NOT move])"<<endl;
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
            cout<<"(CLIENT)Données FINAL(ball,bats et score) reçues du serveur"<<endl;
            cout<<endl<< AllData << endl;
        }
        RectangleShape ballShape, batC1Shape, batC2Shape;

        istringstream iss(AllData);// flux iss  va permettre d'extrire les variable de la chaine de caractère
        float ballLeft, ballTop,
              batC1Left, batC1Top, 
              batC2Left, batC2Top;

        iss >> ballLeft >> ballTop
            >> scoreC1 >> scoreC2
            >> batC1Left >> batC1Top 
            >> batC2Left >> batC2Top ;


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
        //sleep(0.1);
    }


    return 0;
}
