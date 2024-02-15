#include "GameServer.h"
#include <iostream>
#include "GameClient.h"

#include <string>
#include "Status.h"

using namespace std;



int main(int argc, char *argv[])
{
    int status;

    if(argc < 2)
    {
        cout<<endl <<"(SERVEUR)!!! <num_port> !!!"<< endl;
        return -1;
    }
    int port = atoi(argv[1]);

    GameServer server(port);

    status=server.initialize();
    switch (status)
    {
    case ALREADY_READY:
        cout << "(SERVEUR)Serveur deja initialisé" << endl;
        return status;
        break;

    case SOCKET_ERROR:
        cout << "(SERVEUR)Erreur de creation de socket" << endl;
        return status;
        break;

    case BIND_ERROR:// si port deja utilisé par un autre processus
        cout << "(SERVEUR)Erreur de bind" << endl;
        return status;
        break;
    case LISTEN_ERROR://impossible de passer le serveur(port) en mode ecoute
        cout << "(SERVEUR)Erreur de listen" << endl;
        return status;
        break;

    case OK:
        cout << "(SERVEUR)Serveur initialisé avec succes" << endl;
        break;
    
    default:
        return -1;
        break;
    }

    cout << "(SERVEUR)En attente de connexion des clients :" << endl;
    GameClient client1;
    status=server.acceptClient(&client1);
    if(status == OK){cout << "(SERVEUR)Client 1 connecté" << endl;}
    else
    {
        cout << "(SERVEUR)Erreur de connexion client 1" << endl;
        return status;
    }

    GameClient client2;
    status=server.acceptClient(&client2);
    if(status == OK){ cout << "(SERVEUR)Client 2 connecté" << endl;}
    else
    {
        cout << "(SERVEUR)Erreur de connexion client 2" << endl;
        return status;
    }

    while(1)
    {
        char buffer[1024];
        string message;
        status = client1.receive(buffer);
        if (status != OK)
        {
            cout << "(SERVEUR)Erreur de reception de message du client 1: " << buffer << endl;
            return status;
        }
        cout << "(SERVEUR)Message du client 1: " << buffer << endl;

        if(strcmp(buffer, "FIN") == 0)
        {
            cout << "(SERVEUR)Fin de connexion demandée par le client 1" << endl;
            status = client2.send("FIN");
            if (status != OK)
            {
                cout << "(SERVEUR)Erreur d'envoi de message de fin au client 2" << endl;
                return status;
            }
            cout << "(SERVEUR)Fin de connexion envoyée au client 2" << endl;

            break;
        }

        message = buffer;
        status = client2.send((char*)message.c_str());
        if (status != OK)
        {
            cout << "(SERVEUR)Erreur d'envoi de message au client 2: " << message << endl;
            return status;
        }

        status = client2.receive((char*)buffer);
        if (status != OK)
        {
            cout << "(SERVEUR)Erreur de reception de message du client 2: " << buffer << endl;
            return status;
        }
        cout << "(SERVEUR)Message du client 2: " << buffer << endl;

        if(strcmp(buffer, "FIN") == 0)
        {
            cout << "(SERVEUR)Fin de connexion demandée par le client 2" << endl;
            status = client1.send("FIN");
            if (status != OK)
            {
                cout << "(SERVEUR)Erreur d'envoi de message de fin au client 1" << endl;
                return status;
            }
            cout << "(SERVEUR)Fin de connexion envoyée au client 1" << endl;
            break;
        }

        message = buffer;
        status = client1.send((char*)message.c_str());
        if (status != OK)
        {
            cout << "(SERVEUR)Erreur d'envoi de message au client 1: " << message << endl;
            return status;
        }

    }

    server.~GameServer();

    return 0;   

}