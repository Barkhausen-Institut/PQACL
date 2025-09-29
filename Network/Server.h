#ifndef SERVER
#define SERVER

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <fstream>
#include <vector>
#include <iostream>


// Class to start a network communication as a server
class Server {
    public:
        // initiates the network stack on the given port
        Server(boost::asio::io_context& io_context, int port);
        // wait until data is received from the client
        int receive(std::string& msg);
        // receive data and close socket 
        int receive_data(std::string& msg);
        // receive for binary data
        int receive_file(const std::string& filename);

        // send data to the client
        int send(std::string msg);
        // send binary data to the client
        int send_file(const std::string& filename);

    private:
        // provides all i/o functionality
        boost::asio::io_context& io_context_;
        // functionality to accept incoming connections
        boost::asio::ip::tcp::acceptor acceptor_;
        // socket used for the connection
        boost::asio::ip::tcp::socket socket_;
};

#endif  // SERVER