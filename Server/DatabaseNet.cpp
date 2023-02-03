#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

#include "DatabaseNet.h"
#include "DatabaseUtils.h"
#include "DatabaseSupplies.h"

namespace AssetDatabase
{
    int serverFD, newSocket, readStat;
    struct sockaddr_in addrin;
        
    int StartServ(int port)
    {

        int opt = 1;
    
        // Creating socket file descriptor
        serverFD = socket(AF_INET, SOCK_STREAM, 0);

        if (serverFD < 0) {
            std::cout << "socketing failed: " << serverFD << std::endl;
            return errno;
        }
    
        // Forcefully attaching socket to the port 8080
        int sockOpt = setsockopt(serverFD, SOL_SOCKET,
                    SO_REUSEADDR | SO_REUSEPORT, &opt,
                    sizeof(opt)); 
        if (sockOpt){
            std::cout << "socket optioning failed: " << sockOpt << std::endl;;
            return errno;
        }

        addrin.sin_family = AF_INET;
        addrin.sin_addr.s_addr = INADDR_ANY;
        addrin.sin_port = htons(port);
    
        sockaddr adress = *reinterpret_cast<sockaddr*>(&addrin);
    
        int bindStat = bind(serverFD, &adress, sizeof(addrin));
        if (bindStat < 0) {
            std::cout << "bind failed: " << bindStat << std::endl;
            return errno;
        }

        int listenStat = listen(serverFD, 15);
        if (listenStat < 0) {
            std::cout << "listener failed: " << listenStat << std::endl;
            return errno;
        }
        return 0;
    }    
        
    Requisition ProcServ()
    {
        std::cout << "listening\n";
        int addrlen = sizeof(addrin);
        unsigned char buffer[1024] = { 0 };

        Requisition req = {};
        newSocket = accept(serverFD, (struct sockaddr*)&addrin, (socklen_t*)&addrlen);

        /*  Connection process is as follows
        *   Client greets server with custom header, and a requestorID token assigned to client via login (TODO)
        *   Server checks the token as valid (TODO), and twiddles the requestorID before returning to the client
        *   Client then verifies the returned RequestorID as being valid (not 0), and twiddles before sending with request
        */

        if (newSocket < 0) {
            std::cout << "incoming transmission failure: " << newSocket << ", " << errno << std::endl;
            return req;
        }

        readStat = read(newSocket, buffer, 5);
        if (readStat < 0) 
        {
            std::cout << "initial read failure: " << errno << std::endl;
            return req;
        }
        
        if (buffer[0] == (char)0b01100101)
            std::cout << "applett header recognized\n";
        else 
        {
            std::cout << "header byte failed, rejecting: " << buffer[0];
            return req;
        }

        int requestorID = (buffer[1] << 24) + (buffer[2] << 16) + (buffer[3] << 8) + buffer[4];
        requestorID = (requestorID << 1) & requestorID;
        buffer[0] = ((unsigned char)(requestorID >> 24) & 255);
        buffer[1] = ((unsigned char)(requestorID >> 16) & 255);
        buffer[2] = ((unsigned char)(requestorID >> 8) & 255);
        buffer[3] = ((unsigned char)(requestorID)&255);
        send(newSocket, buffer, 4, 0);

        readStat = read(newSocket, buffer, 1024);
        if (readStat < 0) 
        {
            std::cout << "followup read failure: " << errno << std::endl;
            return req;
        }
        
        int requestorCheck = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
        if (requestorCheck == ((requestorID >> 1) | requestorID))
            std::cout << "requestorID matched\n";
        else
        {
            std::cout << "requestorID mismatch: " << requestorCheck << ", " << ((requestorID >> 1) | requestorID) << std::endl;
            return req;
        }

        for (int i = 0; i < buffer[4]; i++) 
        {
            int index = 5 + i * 4;
            AssetDatabase::SupplyItem item = {};
            item.itmCode = (buffer[index] << 16) + (buffer[index + 1] << 8) + buffer[index + 2];
            item.quantity = (int)buffer[index + 3];
            req.supplyItems.push_back(item);
        }

        printf("%s\n", buffer);
        return req;
    }

    int KillServ()
    {
        // closing the connected socket
        close(newSocket);
        // closing the listening socket
        shutdown(serverFD, SHUT_RDWR);
        return 0;
    }
}