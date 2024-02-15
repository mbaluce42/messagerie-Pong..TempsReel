
#include "GameClient.h"
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <server_address> <server_port>" << std::endl;
        return -1;
    }

    char* serverAddr = argv[1];
    int serverPort = atoi(argv[2]);

    GameClient client;

    std::cout << "En attente de connexion au serveur..." << std::endl;

    int status = client.join(serverAddr, serverPort);
    if (status != OK)
    {
        std::cout << "(CLIENT)Erreur de connexion au server: " << status << std::endl;
        return status;
    }

    std::cout << "(CLIENT)Connexion reussi" << std::endl;


    while(1)
    {
        string message;
        cout << "Entrez un message: ";
        cin>>message;


        status = client.send((char*)(message.c_str()));
        if (status != OK)
        {
            std::cout << "(CLIENT)Erreur d'envoi de message: " << status << endl;
            return status;
        }

        if(message == "FIN")
        {
            cout << "(CLIENT)Fin de connexion demandée" << endl;
            status = client.send("FIN");
            if (status != OK)
            {
                cout << "(CLIENT)Erreur d'envoi de message de fin" << endl;
                return status;
            }
            cout << "(CLIENT)Fin de connexion envoyée" << endl;

            break;
        }

        char reponse[1024];
        status = client.receive(reponse);
        if (status != OK)
        {
            std::cout << "(CLIENT)Erreur de reception de message: " << status << endl;
            return status;
        }

        if(strcmp(reponse, "FIN") == 0)
        {
            cout << "(CLIENT)Fin de connexion confirmée" << endl;
            break;
        }

        cout << "(CLIENT)Reponse du serveur: " << reponse << endl;
        
    }
    client.~GameClient();
    cout << "(CLIENT)Connexion fermée" << endl;

    return 0;
}