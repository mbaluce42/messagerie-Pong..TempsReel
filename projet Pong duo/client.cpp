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

    GameClient client;

    int status = client.join(serverAddr, serverPort);
    if (status != OK)
    {
        cout << "(CLIENT)ERREUR de connexion au serveur" << endl;
        return status;
    }

    cout << "(CLIENT)Connection au serveur REUSSI" << std::endl;

    bool focus;

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

        /*string ballData =""+ to_string(ball.getPosition().left)+" "+ to_string(ball.getPosition().top) +" "+to_string(ball.getShape().getSize().x) +" "+ to_string(ball.getShape().getSize().y)+ " ";
        string score="" + to_string(scoreC1)+" "+ to_string(scoreC2)+" ";
        batSendData += to_string(batC1.getPosition().left)+" "+ to_string(batC1.getPosition().top) +" "+to_string(batC1.getShape().getSize().x) +" "+ to_string(batC1.getShape().getSize().y)+ " ";
        batSendData += to_string(batC2.getPosition().left)+" "+ to_string(batC2.getPosition().top) +" "+to_string(batC2.getShape().getSize().x) +" "+ to_string(batC2.getShape().getSize().y)+ " ";
*/

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
        float ballLeft, ballTop, ballSizeX, ballSizeY ,
              batC1Left, batC1Top, batC1SizeX, batC1SizeY,
              batC2Left, batC2Top, batC2SizeX, batC2SizeY;

        iss >> ballLeft >> ballTop >> ballSizeX >> ballSizeY
            >> scoreC1 >> scoreC2
            >> batC1Left >> batC1Top >> batC1SizeX >> batC1SizeY
            >> batC2Left >> batC2Top >> batC2SizeX >> batC2SizeY;


        ballShape.setSize(Vector2f(ballSizeX, ballSizeY));
        ballShape.setPosition(Vector2f(ballLeft, ballTop));
        
        batC1Shape.setSize(Vector2f(batC1SizeX, batC1SizeY));
        batC1Shape.setPosition(Vector2f(batC1Left, batC1Top));

        batC2Shape.setSize(Vector2f(batC2SizeX, batC2SizeY));
        batC2Shape.setPosition(Vector2f(batC2Left, batC2Top));

        std::stringstream ss;
        ss << scoreC1<< "\t" << scoreC2;
        
        hud.setString(ss.str());

        // Clear everything from the last frame
        window.clear(Color(0, 0, 0,255));
        window.draw(batC1Shape);
        window.draw(ballShape);
        window.draw(batC2Shape);

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
