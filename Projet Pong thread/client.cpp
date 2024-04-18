#include "GameClient.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include "bat.h"
#include "ball.h"
#include <string>
#include <pthread.h>

using namespace std;
using namespace sf;

int initHud(GameClient& client, Text& hud, Font& font);
int initSeparator(GameClient& client, RectangleShape (&separators)[16]);
int initTerrain(GameClient& client, Text& hud, Font& font, RectangleShape (&separators)[16]);
int sendEvent(GameClient& client, bool focus);
void finalDataDeserialization(string AllData, Ball& ball, Bat& batC1, Bat& batC2, stringstream& ss);
void afficheTerrain(Text &hud, RectangleShape (&separators)[16], RenderWindow &window,stringstream& ss, Ball& ball, Bat& batC1, Bat& batC2);
int stopConnection(GameClient& client);

void *FctThreadReceive(void *setting);
void *FctThreadSend(void *setting);

void signalSendData(const string& data);
void signalReceiveData();

int receiveData(string& data);

pthread_t threadSend;
pthread_t threadRecv;

pthread_mutex_t mutexReceive = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSend = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condReceive = PTHREAD_COND_INITIALIZER;
pthread_cond_t condSend = PTHREAD_COND_INITIALIZER;


bool erreurRcv = false;
pthread_mutex_t mutexErreurRcv = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condErreur = PTHREAD_COND_INITIALIZER;


bool recvDataAvailable=false;
bool sendDataAvailable=false;
int threadStatus = 0;

int scoreC1 = 0;
int scoreC2 = 0;
int windowWidth = 1024;
int windowHeight = 768;

char receivedData[1024];
string sendData;

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
    
    int res=pthread_create(&threadRecv, NULL, FctThreadReceive, &client);
    if(res==0){cout<<endl<<"ThreadRecv " + to_string(threadRecv) + " cree avec succe"<<endl;}
    else{cout<<endl<<"ERREUR creation ThreadRecv"<<endl; return -1;}

    res=pthread_create(&threadSend, NULL, FctThreadSend, &client);
    if(res==0){cout<<endl<<"ThreadSend " + to_string(threadSend) + " cree avec succe"<<endl;}
    else{cout<<endl<<"ERREUR creation ThreadSend"<<endl; pthread_cancel(threadRecv); return -1;}

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
        pthread_cancel(threadSend);
        pthread_cancel(threadRecv);
        return status;
    }
    else
    {
        cout<<"(CLIENT)HUD et SEPARATOR initialisé"<<endl;
    }
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
                if(status != OK){pthread_cancel(threadSend); pthread_cancel(threadRecv); return status;}
                else{window.close(); cout<<endl<<"(CLIENT)Fenetre fermée" << endl;  pthread_cancel(threadSend); pthread_cancel(threadRecv);return 0; }
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
            pthread_cancel(threadSend);
            pthread_cancel(threadRecv);

            pthread_join(threadSend, NULL);
            pthread_join(threadRecv, NULL);

            return status;
        }
        else
        {
            cout<<"(CLIENT)Evenements envoyés au serveur"<<endl;
        }

        cout<<"En cours de construction du terrain"<<endl;


        /*char AllData[1024];
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
        }*/
        string finalData;
        status= receiveData(finalData);
        if(status != OK)
        {
            cout<<"(CLIENT)ERREUR reception des données du serveur"<<endl;
            cout<<endl<< finalData << endl;
            pthread_cancel(threadSend);
            pthread_cancel(threadRecv);
            return status;

        }

        if( finalData=="STOP")
        {
            cout<<"(CLIENT)Fin de connexion Recu(STOP)" << endl;
            cout << "(CLIENT)Fin de connexion confirmée" << endl;
            break;
        }
        
        
        cout<<"(CLIENT)Données FINAL reçues du serveur "<<endl;
        finalDataDeserialization(finalData, ball, batC1, batC2, ss);

        afficheTerrain(hud, separators, window, ss, ball, batC1, batC2);

        cout<<endl<<"!!! (CLIENT) terrain construit avec succes !!!"<<endl;
    }

    pthread_cancel(threadSend);
    pthread_cancel(threadRecv);

    pthread_join(threadSend, NULL);
    pthread_join(threadRecv, NULL);


    return 0;
}



int initHud(GameClient& client, Text& hud, Font& font)
{
    /*int status= client.send( (char*)("HUD & SEPARATOR") );
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR envoi de la commande HUD & SEPARATOR au serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Commande HUD & SEPARATOR envoyé au serveur"<<endl;
    }*/
    signalSendData("HUD & SEPARATOR");
    if (threadStatus != OK)
    {
        cout << "(CLIENT)ERREUR envoi de la commande HUD & SEPARATOR au serveur" << endl;
        cout << endl << "threadStatus: " << threadStatus << endl;
        return threadStatus;
    }
    else
    {
        cout << "(CLIENT)Commande HUD & SEPARATOR envoyé au serveur" << endl;
    }
    

    /*char graph[1024];
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
    }*/
    string data;
    int status= receiveData(data);
    if (status != OK)
    {
        cout << "(CLIENT)ERREUR reception des données HUD & SEPARATOR du serveur" << endl;
        return status;
    }
    else
    {
        cout << "(CLIENT)Données HUD & SEPARATOR reçues du serveur" << endl;
    }

    
    istringstream iss(data);
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
    /*char graph[1024];
    int status=client.receive(graph);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR reception des données SEPARATOR du serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Données SEPARATOR reçues du serveur"<<endl;
    }*/

    string data;
    int status= receiveData(data);
    if (status != OK)
    {
        cout << "(CLIENT)ERREUR reception des données SEPARATOR du serveur" << endl;
        return status;
    }
    else
    {
        cout << "(CLIENT)Données SEPARATOR reçues du serveur" << endl;
    }

    istringstream iss(data);

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
            /*status = client.send((char *)("Up"));
            if (status != OK)
            {
                cout << "(CLIENT)ERREUR envoi de la position du bat au serveur (Up)" << endl;
                return status;
            }
            else
            {
                cout << "(CLIENT)Message envoyé au serveur (Up) succes" << endl;
            }*/

            signalSendData("Up");
            if (threadStatus != OK)
            {
                cout << "(CLIENT)ERREUR envoi de la position du bat au serveur (Up)" << endl;
                return threadStatus;
            }
            else
            {
                cout << "(CLIENT)Message envoyé au serveur (Up) succes" << endl;
            }

        }
        else if (Keyboard::isKeyPressed(Keyboard::Down))
        {
            /*status = client.send((char *)("Down"));
            if (status != OK)
            {
                cout << "(CLIENT)ERREUR envoi de la position du bat au serveur (Down)" << endl;
                return status;
            }
            else
            {
                cout << "(CLIENT)Message envoye au serveur (Down) succes" << endl;
            }*/

            signalSendData("Down");
            if (threadStatus != OK)
            {
                cout << "(CLIENT)ERREUR envoi de la position du bat au serveur (Down)" << endl;
                return threadStatus;
            }
            else
            {
                cout << "(CLIENT)Message envoyé au serveur (Down) succes" << endl;
            }
        }
        else
        {
            cout << endl << "(CLIENT) fenetre active mais AUCUN MOUV" << endl;
            /*status = client.send((char *)("NOT"));
            if (status != OK)
            {
                cout << "(CLIENT)ERREUR envoi de la position du enemyBat au serveur (NOT)" << endl;
                return status;
            }
            else
            {
                cout << "(CLIENT)Message envoyé au serveur (NOT) succes " << endl;
            }*/

            signalSendData("NOT");
            if (threadStatus != OK)
            {
                cout << "(CLIENT)ERREUR envoi de la position du enemyBat au serveur (NOT)" << endl;
                return threadStatus;
            }
            else
            {
                cout << "(CLIENT)Message envoyé au serveur (NOT) succes" << endl;
            }
        }
    }
    else
    {
        cout << endl<< "(CLIENT)Fenetre non active" << endl;
        /*status = client.send((char *)("NOT"));
        if (status != OK)
        {
            cout << "(CLIENT)ERREUR envoi de la position du enemyBat au serveur (NOT)" << endl;
            return status;
        }
        else
        {
            cout << "(CLIENT)Message envoyé au serveur (NOT) succes" << endl;
        }*/
        signalSendData("NOT");
        if (threadStatus != OK)
        {
            cout << "(CLIENT)ERREUR envoi de la position du enemyBat au serveur (NOT)" << endl;
            return threadStatus;
        }
        else
        {
            cout << "(CLIENT)Message envoyé au serveur (NOT) succes" << endl;
        }
    }
    return OK;
}

void finalDataDeserialization(string AllData, Ball& ball, Bat& batC1, Bat& batC2, stringstream& ss)
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
    /*int status= client.send( (char*)("STOP"));
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
    }*/

    signalSendData("STOP");
    if (threadStatus != OK)
    {
        cout << "(CLIENT)ERREUR envoi AU REVOIR au serveur (STOP)" << endl;
        return threadStatus;
    }
    else
    {
        cout << "(CLIENT)Message envoyé au serveur (STOP)" << endl;
        cout << "(CLIENT)Fin de connexion " << endl;
        return OK;
    }
}


void *FctThreadReceive(void *setting)
{
    GameClient *client = (GameClient *)setting;
    while (1)
    {
        char Data[1024];
        int status = client->receiveNonBlocking(receivedData, 3000);

        if (status == TIMEOUT)
        {
            // Gérer le cas où le receive a expiré
            cout << "(CLIENT)TIMEOUT reception des données du serveur" << endl;
            continue;
        }
        else if (status != OK)
        {
            // Gérer l'erreur de receive
            cout << "(CLIENT)ERREUR reception des données du serveur" << endl;
            pthread_mutex_lock(&mutexErreurRcv);
            erreurRcv = true;
            pthread_mutex_unlock(&mutexErreurRcv);
            pthread_cond_signal(&condErreur);
            break;//sortie de la boucle (quitte le thread)
        }
        else//status == OK
        {
            // Gérer le cas où le receive a réussi
            memset(receivedData, 0, sizeof(receivedData));
            cout << "(CLIENT)Données reçues du serveur " << receivedData << endl;
            pthread_mutex_lock(&mutexReceive);
            strcpy(receivedData,Data);//copie des données reçues dans la variable globale
            recvDataAvailable = true;
            pthread_mutex_unlock(&mutexReceive);
            pthread_cond_signal(&condReceive);
        }
    }

    pthread_exit(NULL);
}


void *FctThreadSend(void *setting)
{
    GameClient *client = (GameClient *)setting;
    
    while (1)
    {
        cout<<endl << "XX" << endl;
        pthread_mutex_lock(&mutexSend);
        
        while (!sendDataAvailable)
        {
            cout<<endl << "XXX" << endl;
            pthread_cond_wait(&condSend, &mutexSend);
            cout<<endl << "XXXX" << endl;
        }

        threadStatus = client->send((char *)sendData.c_str());
        cout<<endl << "XXXXX" << endl;

        if (threadStatus != OK)
        {
            // Gérer l'erreur de send
            cout<<endl<<"ERROR sending data\n"<<endl;
            break;
        }
        else
        {
            // Gérer le cas où le send a réussi
            cout << "(CLIENT)Message envoyé au serveur " << sendData << endl;
        }

        sendDataAvailable = false;

        pthread_mutex_unlock(&mutexSend);
    }
}

void signalSendData(const string& data)
{
    pthread_mutex_lock(&mutexSend);
    sendDataAvailable = true;
    pthread_mutex_unlock(&mutexSend);
    pthread_cond_signal(&condSend);
    sendData = data;
}

int receiveData(string& data)
{
    pthread_mutex_lock(&mutexReceive);
    while (!recvDataAvailable && !erreurRcv) // Attendre que les données soient disponibles
    {
        pthread_cond_wait(&condReceive, &mutexReceive);
    }
    if (erreurRcv)
    {
        pthread_mutex_unlock(&mutexReceive);
        return ERROR;
    }
    
    cout<<endl<<"(CLIENT)Données reçues du serveur " << receivedData << endl;
    recvDataAvailable = false;
    data = receivedData;
    pthread_mutex_unlock(&mutexReceive);
    return OK;
}