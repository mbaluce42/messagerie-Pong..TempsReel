// These "include" code from the C++ library and SFML too
//https://gamecodeschool.com/sfml/coding-a-simple-pong-game-with-sfml/
#include <iostream>
#include "bat.h"
#include "ball.h"
#include <sstream>
#include <cstdlib>
#include <SFML/Graphics.hpp>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

using namespace sf;
int main()
{
    int windowWidth = 1024;
    int windowHeight = 768;
    // Make a window that is 1024 by 768 pixels
    // And has the title "Pong"
    RenderWindow window(VideoMode(windowWidth, windowHeight), "Pong");
    // create a bat
    Bat bat (0, windowHeight/2);
    Bat enemy_bat(windowWidth-bat.getShape().getSize().x, windowHeight/2);

    // create a ball
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
    int score1, score2;
    score1=0;
    score2=0;
    bool focus;
    ball.start();

    #include <iostream>
    std::cout << "Jeu a commencé" << std::endl;
    while (window.isOpen())
    {
        /*
            Handle the player input
            *********************************************************************
            *********************************************************************
            *********************************************************************
        */
        
        Event event;
        
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                // Someone closed the window- bye
                window.close();
        else if(event.type == sf::Event::GainedFocus) focus=true;
        else if(event.type == sf::Event::LostFocus) focus=false;
        }
        if (focus){
            if (Keyboard::isKeyPressed(Keyboard::Up))
            {
                // move up...
                if(bat.getPosition().top > 0)
                        bat.moveUp();
            }
            else if (Keyboard::isKeyPressed(Keyboard::Down))
            {
                // move Down...
                if(bat.getPosition().top < windowHeight - bat.getShape().getSize().y)
                        bat.moveDown();
            }

            if (Keyboard::isKeyPressed(Keyboard::Z)){
                if(enemy_bat.getPosition().top > 0)
                    enemy_bat.moveUp();
            }
            else if (Keyboard::isKeyPressed(Keyboard::S))
            {
                if(enemy_bat.getPosition().top < windowHeight - enemy_bat.getShape().getSize().y)
                    enemy_bat.moveDown();
            }
            

            if (Keyboard::isKeyPressed(sf::Keyboard::Escape))
            {
                // quit...
                // Someone closed the window- bye
                window.close();
            }
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
            score2++;
        
        
        }
        // Handle ball hitting side
        if (ball.getPosition().left > windowWidth)
        {
            ball.hitSide(windowWidth/2, windowHeight/2);
            score1++;
        
        
        }

        
        // Has the ball hit the bat?
        if (ball.getPosition().intersects(bat.getPosition()) || ball.getPosition().intersects(enemy_bat.getPosition()))
        {
            ball.reboundBat();
        }

        float oldPosX = ball.getPosition().left;
        bat.update();
        ball.update();
        std::stringstream ss;
        ss << score1<< "\t" << score2;
        enemy_bat.update();



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
    }// This is the end of the "while" loop

    return 0;
}
