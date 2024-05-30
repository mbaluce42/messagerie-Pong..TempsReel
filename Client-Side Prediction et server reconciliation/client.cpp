#include "GameClient.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include "bat.h"
#include "ball.h"
#include <string>
#include <pthread.h>
#include <queue>



using namespace std;
using namespace sf;

int scoreC1 = 0;
int scoreC2 = 0;
int windowWidth = 1024;
int windowHeight = 768;

int initHud(Text& hud, Font& font);
int initSeparator(RectangleShape (&separators)[16]);
int initTerrain(Text& hud, Font& font, RectangleShape (&separators)[16]);
int sendEvent(bool focus);
void finalDataDeserialization(string AllData, Ball& ball, Bat& batC1, Bat& batC2, stringstream& ss);
void afficheTerrain(Text &hud, RectangleShape (&separators)[16], RenderWindow &window,stringstream& ss, Ball& ball, Bat& batC1, Bat& batC2);
int stopConnection();

void *FctThreadReceive(void *setting);
int receiveData(string& data);


void *FctThreadSend(void *setting);
void signalSendData(const string& data);


pthread_t threadSend;
pthread_t threadRecv;

pthread_mutex_t mutexReceive = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condReceive = PTHREAD_COND_INITIALIZER;
queue<string> receivedQueue; //fifo
bool receiveDataAvailable=false;

pthread_mutex_t mutexSend = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condSend = PTHREAD_COND_INITIALIZER;
bool sendDataAvailable=false;
string sendData;
int threadStatus=OK;

bool erreurRcv = false;
pthread_mutex_t mutexErreurRcv = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condErreur = PTHREAD_COND_INITIALIZER;



GameClient* client=new GameClient();
int forLag=0;

queue<String> evenPlayer;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Usage: " << argv[0] << " <server_address> <server_port> <ms_for_Sim_Lag>" << endl;
        return -1;
    }
    char* ipAdresse = argv[1];
    int port = atoi(argv[2]);
    forLag = atoi(argv[3]);


    //client va se connecter au serveur
    int status = client->join(ipAdresse, port);
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

    int res=pthread_create(&threadRecv, NULL, FctThreadReceive,NULL);
    if(res==0){cout<<endl<<"ThreadRecv " + to_string(threadRecv) + " cree avec succe"<<endl;}
    else{cout<<endl<<"ERREUR creation ThreadRecv"<<endl; return -1;}

    res=pthread_create(&threadSend, NULL, FctThreadSend,NULL);
    if(res==0){cout<<endl<<"ThreadSend " + to_string(threadSend) + " cree avec succe"<<endl;}
    else{cout<<endl<<"ERREUR creation ThreadSend"<<endl; pthread_cancel(threadRecv); pthread_join(threadRecv, NULL); return -1;}


    cout << endl<<"(CLIENT)initialisation du terrain ......................" << endl;


    // Create a HUD (Head Up Display)
    status= initTerrain(hud, font, separators);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR initialisation du terrain"<<endl;
        pthread_cancel(threadRecv); pthread_join(threadRecv, NULL);
        pthread_cancel(threadSend); pthread_join(threadSend, NULL);
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
                status=stopConnection();
                if(status != OK)
                {
                    pthread_cancel(threadRecv); pthread_join(threadRecv, NULL);
                    pthread_cancel(threadSend); pthread_join(threadSend, NULL);    
                    return status; 
                }
                else
                {
                    window.close(); 
                    cout<<endl<<"(CLIENT)Fenetre fermée" << endl; 
                    pthread_cancel(threadRecv); pthread_join(threadRecv, NULL); 
                    pthread_cancel(threadSend); pthread_join(threadSend, NULL);
                    return 0; 
                }
            }
            else if(event.type == sf::Event::GainedFocus) {focus=true; cout<<endl<<"(CLIENT)Fenetre active" << endl;}
            else if(event.type == sf::Event::LostFocus) {focus=false; cout<<endl<<"(CLIENT)Fenetre non active" << endl;}
        }


        status= sendEvent(focus);
        if(status != OK)
        {
            if(status == 99)
            {
                //cout<<"(CLIENT)Fin de connexion"<<endl;
                window.close();
                break;
            }
            cout<<"(CLIENT)ERREUR envoi des evenements au serveur"<<endl;
            pthread_cancel(threadRecv); pthread_join(threadRecv, NULL);
            pthread_cancel(threadSend); pthread_join(threadSend, NULL);
            return status;
        }
        else
        {
            cout<<"(CLIENT)Evenements envoyés au serveur"<<endl;
        }

        cout<<"En cours de construction NEW du terrain"<<endl;
        //reception des données du serveur (position des elements du terrain)
        string AllData;
        status= receiveData(AllData);
        if(status != OK)
        {
            cout<<"(CLIENT)ERREUR reception des données du serveur"<<endl;
            cout<<endl<< AllData << endl;
            pthread_cancel(threadRecv); pthread_join(threadRecv, NULL);
            pthread_cancel(threadSend); pthread_join(threadSend, NULL);
            return status;
        }

        if(AllData=="STOP")
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

    pthread_cancel(threadRecv);pthread_join(threadRecv, NULL);
    pthread_cancel(threadSend); pthread_join(threadSend, NULL);

    return 0;
}


int initHud(Text& hud, Font& font)
{
    int status;
    signalSendData("HUD & SEPARATOR");
    status= threadStatus;
    if(status!= OK)
    {
        cout<<"(CLIENT)ERREUR envoi de la commande HUD & SEPARATOR au serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Commande HUD & SEPARATOR envoyé au serveur"<<endl;
    }

    string graph;
    status=receiveData(graph);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR reception des données HUD & SEPARATOR du serveur"<<endl;
        return status;
    }
    else
    {
        cout<<"(CLIENT)Données HUD reçues du serveur: "<<endl;
        cout<< graph << endl;
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
int initSeparator(RectangleShape (&separators)[16])
{
    string graph;
    int status=receiveData(graph);
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
        cout <<endl<<"sizeY="<< positionY<<endl;
    }
    return OK;

}
int initTerrain(Text& hud, Font& font, RectangleShape (&separators)[16])
{
    int status= initHud(hud, font);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR initialisation du HUD"<<endl;
        return status;
    }
    status= initSeparator(separators);
    if(status != OK)
    {
        cout<<"(CLIENT)ERREUR initialisation du SEPARATOR"<<endl;
        return status;
    }
    return OK;
}

int sendEvent(bool focus)
{
    int status;
    if (focus)
    {
        if (Keyboard::isKeyPressed(sf::Keyboard::Escape))
        {
            status=stopConnection();
            if(status != OK){return status;}
            else{return 99;}
        }
        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            //status = client->send((char *)("Up"));
            signalSendData("Up");
            status= threadStatus;
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
            //status = client->send((char *)("Down"));
            signalSendData("Down");
            status= threadStatus;
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
            cout << endl<< "(CLIENT) fenetre active mais AUCUN MOUV" << endl;
            //status = client->send((char *)("NOT"));
            signalSendData("NOT");
            status= threadStatus;
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
        cout << endl<< "(CLIENT)Fenetre non active" << endl;
        //status = client->send((char *)("NOT"));
        signalSendData("NOT");
        status= threadStatus;
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
int stopConnection()
{
    //int status= client->send( (char*)("STOP"));
    signalSendData("STOP");
    int status= threadStatus;
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


void *FctThreadReceive(void *setting)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    while (1)
    {
        char Data[1024];
        int status = client->receiveNonBlocking(Data, 200);

        struct timespec wait;
        // Conversion des millisecondes en secondes et nanosecondes
        wait.tv_sec = forLag / 1000;
        wait.tv_nsec = (forLag % 1000) * 1000000;
        nanosleep(&wait, NULL);

        if (status == TIMEOUT)
        {
            //cout<<endl<<"timeout = front queue="<<receivedQueue.front()<<endl;
            pthread_mutex_lock(&mutexReceive);
            if (!receivedQueue.empty())
            {
                receiveDataAvailable = true;
                pthread_cond_signal(&condReceive);
            }
            pthread_mutex_unlock(&mutexReceive);

            //continue;

        }
        else if (status != OK)
        {
            cout << "(CLIENT threadReceive)ERREUR reception des données du serveur" << endl;
            pthread_mutex_lock(&mutexErreurRcv);
            erreurRcv = true;
            pthread_cond_signal(&condErreur);
            pthread_mutex_unlock(&mutexErreurRcv);
            break;
        }
        else
        {
            cout << "(CLIENT threadReceive)Données reçues du serveur " << Data << endl;
            pthread_mutex_lock(&mutexReceive);
            string tmp(Data);
            receivedQueue.push(tmp); 
            receiveDataAvailable = true;
            pthread_cond_signal(&condReceive);
            pthread_mutex_unlock(&mutexReceive);
        }

        
    }

    pthread_exit(NULL);
}


int receiveData(string& data)
{
    pthread_mutex_lock(&mutexReceive);
    while (!receiveDataAvailable && !erreurRcv) 
    {
        pthread_cond_wait(&condReceive, &mutexReceive);
    }

    if (erreurRcv)
    {
        pthread_mutex_unlock(&mutexReceive);
        cout<<endl<<"(CLIENT fct receiveData()) Erreur de Receive"<<endl;
        return ERROR;
    }

    if (!receivedQueue.empty())
    {
        data = receivedQueue.front();
        receivedQueue.pop();
        cout << "(CLIENT fct receiveData())Données reçues du serveur: " << data << endl;
    }
    else
    {
        cout << "(CLIENT)La file d'attente est vide" << endl;
        //return ERROR; // Ou un autre code d'erreur approprié
    }

    receiveDataAvailable = false;
    pthread_mutex_unlock(&mutexReceive);
    return OK;
}


void *FctThreadSend(void *setting)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    while (1)
    {
        string data;
        pthread_mutex_lock(&mutexSend);
        
        while (!sendDataAvailable)
        {
            pthread_cond_wait(&condSend, &mutexSend);
        }
        data =String(sendData);

        cout << "(CLIENT threadSend)Prêt à envoyer : " << data << endl;

        int status = client->send((char *)data.c_str());

        if (status != OK)
        {
            // Gérer l'erreur de send
            cout << "(CLIENT threadSend)ERREUR sending data: " << status << endl;
            pthread_mutex_unlock(&mutexSend);
            continue;
        }
        else
        {
            // Gérer le cas où le send a réussi
            
            cout << "(CLIENT)Message envoyé au serveur " << data << endl;
            threadStatus=OK;
        }

        sendDataAvailable = false;

        pthread_mutex_unlock(&mutexSend);

        // Mettre à jour le statut
        pthread_mutex_lock(&mutexSend);
        threadStatus = status;
        pthread_mutex_unlock(&mutexSend);
    }
}


void signalSendData(const string& data)
{
    pthread_mutex_lock(&mutexSend);
    sendData = String(data);
    sendDataAvailable = true;// donnees dispo pour send serveur 
    pthread_cond_signal(&condSend);
    pthread_mutex_unlock(&mutexSend);
}