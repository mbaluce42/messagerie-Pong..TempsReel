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

int main(int argc, char *argv[])
{
    int windowWidth = 1024;
    int windowHeight = 768;

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
        std::cout << "Error joining the server: " << status << std::endl;
        return status;
    }

    std::cout << "Connected to the server" << std::endl;

    bool focus;

        // Clear everything from the last frame
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
        window.display();

    

    while (window.isOpen())
    {
        Event event;

        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                // Someone closed the window- bye
                window.close();
        else if(event.type == sf::Event::GainedFocus) focus=true;
        else if(event.type == sf::Event::LostFocus) focus=false;
        }

        if(focus)
        {
            // Send bat position to the server
            Vector2f batPos;
            if (Keyboard::isKeyPressed(Keyboard::Up))
            {
                // Move bat up
                bat.moveUp();
                batPos = Vector2f(bat.getPosition().width , bat.getPosition().height);
                client.send((char*)&batPos);
            }
            else if (Keyboard::isKeyPressed(Keyboard::Down))
            {
                // Move bat down
                bat.moveDown();
                batPos = sf::Vector2f(bat.getPosition().width , bat.getPosition().height);
                client.send((char*)&batPos);
            }

            if (Keyboard::isKeyPressed(Keyboard::Z)){
                if(enemy_bat.getPosition().top > 0)
                    enemy_bat.moveUp();
                    batPos = Vector2f(enemy_bat.getPosition().width , enemy_bat.getPosition().height);

            }
            else if (Keyboard::isKeyPressed(Keyboard::S))
            {
                if(enemy_bat.getPosition().top < windowHeight - enemy_bat.getShape().getSize().y)
                    enemy_bat.moveDown();
                    batPos = Vector2f(enemy_bat.getPosition().width , enemy_bat.getPosition().height);
            }

            if (Keyboard::isKeyPressed(sf::Keyboard::Escape))
            {
                // quit...
                // Someone closed the window- bye
                window.close();
            }
        }


        // Receive positions from the server
        Vector2f posBat;
        status = client.receive((char*)&posBat);
        if (status != OK)
        {
            std::cout << "Error receiving bat position: " << status << std::endl;
            return status;
        }

        // Update bats
        bat.setYPosition(posBat.y);
        enemy_bat.update();

        // Update ball
        ball.update();

        // Clear window
        window.clear(sf::Color::Black);

        // Draw bats and ball
        window.draw(bat.getShape());
        window.draw(enemy_bat.getShape());
        window.draw(ball.getShape());

        // Display window
        window.display();
    }

    return 0;
}
