#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <NTL/mat_ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include "helperFunctions.h"
#include "../Network/Client.h"

/*
This ConnectionHandler class is used by the client to communicate with the data server.
It is used to send and receive data from the server.
*/
class ConnectionHandler{
    public:
        // Takes the server IP and port as input 
        ConnectionHandler(const std::string& ServerIP, const int& port);

        // Function to get the zero knowledge proof B_i from the server
        void getB_i_proof_from_server(const std::string& proof_export_path);
        
        // Function to send the zero knowledge proof to the server
        void sendProofToServer(const std::string& proof_path);

        // Function to send data to the server
        void send_data(const std::string& msg_identifier, const std::string& msg_content);
        
        // Function to send a message to the server
        void send(const std::string& msg_identifier);

    private:

        std::string ServerIP;
        std::string ServerPort;
};

#endif  // CONNECTIONHANDLER_H