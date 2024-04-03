#include "ConnectionHandler.h"

// Saves the IP and the port in the connection class for the data server
ConnectionHandler::ConnectionHandler(const std::string& ServerIP, const int& port){
    this->ServerIP = ServerIP;
    this->ServerPort = std::to_string(port);
}

// Function to get the zero knowledge proof B_i from the server
void ConnectionHandler::getB_i_proof_from_server(const std::string& proof_export_path){
    Client client = Client(this->ServerIP, this->ServerPort);
    std::string msg = "getProofB_i";
    client.send(msg);
    client.receive_file(proof_export_path);
}

// Function to send data to the server
void ConnectionHandler::send_data(const std::string& msg_identifier, const std::string& msg_content){
    Client client = Client(this->ServerIP, this->ServerPort);
    client.send(msg_identifier);
    client.send_data(msg_content);
}

// Function to send a message to the server
void ConnectionHandler::send(const std::string& msg_identifier){
    Client client = Client(this->ServerIP, this->ServerPort);
    client.send(msg_identifier);
}

// Function to send the zero knowledge proof to the server
void ConnectionHandler::sendProofToServer(const std::string& proof_path){
    Client client = Client(this->ServerIP, this->ServerPort);
    std::string msg = "validateAccess";
    client.send(msg);
    client.send_file(proof_path);
}