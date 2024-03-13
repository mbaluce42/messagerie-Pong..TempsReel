/*#include "GameClient.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include "bat.h"

using namespace std;
using namespace sf;

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        cout << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return -1;
    }

    string server_ip = argv[1];
    int server_port = atoi(argv[2]);

    GameClient client;
    if(client.connectToServer(server_ip, server_port) != OK)
    {
        cout << "Error connecting to server" << endl;
        return -1;
    }

    // Création de la fenêtre SFML
    RenderWindow window(VideoMode(800, 600), "Pong Client");

    // Création d'un bat pour le client
    Bat bat(0, 0); // Initialiser la position du bat

    while (window.isOpen())
    {
        // Gérer les événements
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        // Envoyer la position du bat au serveur
        Vector2f batPosition = bat.getPosition();
        if(client.send((char*)&batPosition) != OK)
        {
            cout << "Error sending bat position to server" << endl;
            return -1;
        }

        // Recevoir la position du bat de l'adversaire
        Vector2f opponentBatPosition;
        if(client.receive((char*)&opponentBatPosition) != OK)
        {
            cout << "Error receiving opponent bat position from server" << endl;
            return -1;
        }

        // Mettre à jour la position du bat de l'adversaire
        bat.setYPosition(opponentBatPosition.y);

        // Effacer l'écran
        window.clear();

        // Dessiner le bat du client
        window.draw(bat.getShape());

        // Afficher le contenu de la fenêtre
        window.display();
    }

    // Fermer la connexion avec le serveur
    client.disconnect();

    return 0;
}*/



#include "GameClient.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include "bat.h"
#include "ball.h"
#include <string>

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

    // Create bats and ball
    Bat bat (0, windowHeight/2);
    Bat enemy_bat(windowWidth-bat.getShape().getSize().x, windowHeight/2);
    Ball ball(windowWidth / 2, windowHeight/2);

    Text hud;
    Font font;
    font.loadFromFile("OpenSans-Bold.ttf");
    hud.setFont(font);
    hud.setCharacterSize(75);
    hud.setFillColor(sf::Color::White);
    hud.setPosition(Vector2f((windowWidth/2)-100,0));


    // Create separator area
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

       /* // Clear everything from the last frame
        window.clear(Color(0, 0, 0,255));

        window.draw(bat.getShape());

        window.draw(ball.getShape());

        window.draw(enemy_bat.getShape());

        // Draw our score

        // draw separator
        for (int i = 0; i<16;i++){
            window.draw(separators[i]);
        }
        // Show everything we just drew
        window.display();*/
    

    while (window.isOpen())
    {
        Event event;

        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed){
                // Someone closed the window- bye
                window.close();}
        else if(event.type == sf::Event::GainedFocus) focus=true;
        else if(event.type == sf::Event::LostFocus) focus=false;
        }

        if(focus)
        {
            string batData="";
            string enemyBatData="";
            // Send bat position to the server
            if (Keyboard::isKeyPressed(Keyboard::Up))
            {
                // Move bat up
                /*if(bat.getPosition().top > 0)
                {*/
                    //bat.moveUp();
                    batData += "1Up,";
                    batData += to_string(bat.getPosition().left)+","+ to_string(bat.getPosition().top) +",";
                    batData += to_string(bat.getShape().getSize().x) +","+ to_string(bat.getShape().getSize().y);
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

                    /*char batData[1024]; char enemyBatData[1024];

                    client.receive(batData);
                    istringstream iss(batData);// flux iss  va permettre d'extrire les variable de la chaine de caractère
                    float posLeft, posTop;
                    // Récupérer la position
                    iss >>posLeft >> posTop;
                    bat = Bat(posLeft, posTop);*/

                //}
                
            }
            else if (Keyboard::isKeyPressed(Keyboard::Down))
            {
                // Move bat down
                //if(bat.getPosition().top < windowHeight - bat.getShape().getSize().y)
                //bat.moveDown();
                batData += "1Down,";
                batData += to_string(bat.getPosition().left)+","+ to_string(bat.getPosition().top)+",";
                batData += to_string(bat.getShape().getSize().x) +","+ to_string(bat.getShape().getSize().y);

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
                //client.receive((char*)&enemy_bat.getPosition());
            }

            if (Keyboard::isKeyPressed(Keyboard::Z)){
                /*if(enemy_bat.getPosition().top > 0)
                    enemy_bat.moveUp();*/
                enemyBatData += "2Up,";
                enemyBatData += to_string(enemy_bat.getPosition().left)+","+ to_string(enemy_bat.getPosition().top)+",";
                enemyBatData += to_string(enemy_bat.getShape().getSize().x) +","+ to_string(enemy_bat.getShape().getSize().y);
                status=client.send( (char*)(enemyBatData.c_str()) );
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi de la position du enemyBat au serveur (2Up[Z])"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT)Position du enemyBat envoyé au serveur (2Up[Z])"<<endl;
                }

            }
            else if (Keyboard::isKeyPressed(Keyboard::S))
            {
                //if(enemy_bat.getPosition().top < windowHeight - enemy_bat.getShape().getSize().y)
                    //enemy_bat.moveDown();
                enemyBatData += "2Down,";
                enemyBatData += to_string(enemy_bat.getPosition().left)+","+ to_string(enemy_bat.getPosition().top)+",";
                enemyBatData += to_string(enemy_bat.getShape().getSize().x) +","+ to_string(enemy_bat.getShape().getSize().y);
                status=client.send( (char*)(enemyBatData.c_str()) );
                if (status != OK)
                {
                    cout<<"(CLIENT)ERREUR envoi de la position du enemyBat au serveur (2Down[S])"<<endl;
                    return status;
                }
                else
                {
                    cout<<"(CLIENT)Position du enemyBat envoyé au serveur (2Down[S])"<<endl;
                }

            }

            if (Keyboard::isKeyPressed(sf::Keyboard::Escape))
            {
                // quit...
                // Someone closed the window- bye
                client.send("ESC");
                window.close();
            }
        }

        // Receive updated position of the ball, bats and score from the server

        /*ballData += to_string(ball.getPosition().left)+","+ to_string(ball.getPosition().top)+","+ to_string(scoreC1)+","+ to_string(scoreC2)+",";
        batSendData += to_string(batC1.getPosition().left)+","+ to_string(batC1.getPosition().top)+",";
        batSendData += to_string(batC2.getPosition().left)+","+ to_string(batC2.getPosition().top);*/

        char AllData[1024];
        client.receive(AllData);
        istringstream iss(AllData);// flux iss  va permettre d'extrire les variable de la chaine de caractère
        float ballLeft, ballTop, batC1Left, batC1Top, batC2Left, batC2Top;

        iss >>ballLeft >> ballTop >> scoreC1 >> scoreC2 >> batC1Left >> batC1Top >> batC2Left >> batC2Top;

        bat = Bat(batC1Left, batC1Top);
        enemy_bat = Bat(batC2Left, batC2Top);
        ball = Ball(ballLeft, ballTop);
        std::stringstream ss;
        ss << scoreC1<< "\t" << scoreC2;
        
        hud.setString(ss.str());

        // Clear everything from the last frame
        window.clear(Color(0, 0, 0,255));
        window.draw(bat.getShape());
        window.draw(ball.getShape());
        window.draw(enemy_bat.getShape());

        // Draw our score
        window.draw(hud);

        // draw separator
        for (int i = 0; i<16;i++){
            window.draw(separators[i]);
        }
        // Show everything we just drew
        window.display();
    }

    return 0;
}
