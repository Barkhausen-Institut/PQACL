#ifndef CLIENT
#define CLIENT

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <fstream>

class Client {
    public:
        // initializes the network stack for the client
        Client(std::string ip, std::string port);
        // sends requested keywords to the server
        int send(std::string msg);
        // send binary data to the server
        int send_file(const std::string& filename);
        // send data and close socket
        int send_data(std::string msg);
        // receive for strings 
        int receive(std::string &msg);
        // receive for binary data
        int receive_file(const std::string& filename);
    private:
        // provides all i/o functionality
        boost::asio::io_context io_context_;
        // resolves hostname to ip 
        boost::asio::ip::tcp::resolver resolver_;
        // manages the connection 
        boost::asio::ip::tcp::socket socket_;
        // saves resolved endpoints
        boost::asio::ip::tcp::resolver::results_type endpoints_;
};

#endif   // CLIENT