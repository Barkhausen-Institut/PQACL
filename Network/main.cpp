#include <iostream>
#include <memory>
#include <string.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include "Client.h"
#include "Server.h"


/*
Execute the compiled program as followed:
    For using the client:
        sudo ./keywordSearch.o client -a [ip_address_of_server=localhost(default)] -p [port=4444(default)]

    For using the server:
        sudo ./keywordSearch.o server -p [port=4444(default)]


*/



// reads input arguments and sets parameters for initializing the according mode.
void processInputArguments(int ac, char* av[], bool* IS_CLIENT, std::string* IP, int* PORT)
{
    try {
        int port;
        std::string mode;
        std::string ip;
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("mode,M", boost::program_options::value<std::string>(&mode)->default_value("client"), 
                "starts program in [server|client] mode")
            ("port,p", boost::program_options::value<int>(&port)->default_value(4444),
                  "listen/send on this port")
            ("ip_address,a", boost::program_options::value<std::string>(&ip)->default_value("localhost"),
                  "specifies the ip address of the server, only necessary in client mode")
        ;

        boost::program_options::positional_options_description p;
        p.add("mode", -1);

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(ac, av).
                  options(desc).positional(p).run(), vm);
        boost::program_options::notify(vm);

        if (vm.count("help")){
            std::cout << "Usage: options_description {mode} [options]\n";
            std::cout << desc;
            exit(0);
        }
        if (vm.count("mode")){
            if (mode == "client") {
                *IS_CLIENT = true;
            } else if (mode == "server") {
                *IS_CLIENT = false;
            } else {
                std::cout << "Mode incorrect" << std::endl;
                exit(0);
            }
            std::cout << "Mode: " << mode << std::endl;

        }
        if (vm.count("ip_address")){
            *IP = ip;
            std::cout << "IP: " << ip << std::endl;
        }
        if (vm.count("port")){
            *PORT = port;
            std::cout << "Port: " << port << std::endl;
        }

    }
    catch(std::exception& e){
        std::cout << e.what() << "\n";
        exit(1);
    }
}


int main(int ac, char* av[]){
    
    // variable if program acts as client or server
    bool IS_CLIENT;
    std::string IP;
    int PORT;

    processInputArguments(ac, av, &IS_CLIENT, &IP, &PORT);

    // chose classes to start the application
    if(IS_CLIENT){
        Client client = Client(IP, std::to_string(PORT)); 

        client.send("Hello from client\n");

        std::string received_msg = "";
        client.receive(received_msg);
        std::cout << received_msg << std::endl;

    } else {
        boost::asio::io_context io_context;
        Server server = Server(io_context, PORT);;
        

        std::string received_msg = "";
        server.receive(received_msg);
        std::cout << received_msg << std::endl;
        
        server.send("Hello from server\n");
        
    }
    


}

