#include "Server.h"

Server::Server(boost::asio::io_context& io_context, int port) : io_context_(io_context), 
    acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), socket_(io_context_){
}

int Server::send(std::string msg){	
    // check if socket is open
    if (!socket_.is_open()) {
        std::cerr << "Socket is not open" << std::endl;
        return -1;
    }

    boost::system::error_code error_code;
    // send the message to the client
    boost::asio::write(socket_, boost::asio::buffer(msg), error_code);
    // if error occurred, print it
    if (error_code){
        std::cerr << "Error occurred during write: " << error_code.message() << std::endl;
    }
    socket_.close();    // close the socket
    return 0;
}

int Server::send_file(const std::string& filename){
    // check if socket is open
    if (!socket_.is_open()) {
        std::cerr << "Socket is not open" << std::endl;
        return -1;
    }

    // Open the binary file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }

    // Read the file contents into a buffer
    std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

    boost::system::error_code error_code;
    // send the message to the client
    boost::asio::write(socket_, boost::asio::buffer(buffer), error_code);
    // if error occurred, print it
    if (error_code){
        std::cerr << "Error occurred during write: " << error_code.message() << std::endl;
    }
    // Close the file
    file.close();

    socket_.close();    // close the socket
    return 0;
}

int Server::receive(std::string& msg){
    for (;;) {
        // create buffer to store incomming data   
        boost::array<char, 100000> buf;
        // store error code
        boost::system::error_code error;
        acceptor_.accept(socket_);
        // read data from socket and store it in the buffer
        size_t len = socket_.read_some(boost::asio::buffer(buf), error);
        // if error occurred, print it, except end of file error (eof) then continue
        if (error){
            if(error != boost::asio::error::eof){
                std::cerr << "Read error: " << error.message() << std::endl;
            }
            continue;
        }
        // store the received data in msg
        msg = std::string(buf.data(), len);
        // print the received data
        std::cout << "Received: " << std::string(buf.data(), len) << std::endl;

        break;
    }
    return 0;
}

int Server::receive_data(std::string& msg){
    // check if socket is open and close if it is
    if (socket_.is_open()) {
        socket_.close();
    }

    for (;;) {
        // create buffer to store incomming data   
        boost::array<char, 100000> buf;
        // store error code
        boost::system::error_code error;
        acceptor_.accept(socket_);
        // read data from socket and store it in the buffer
        size_t len = socket_.read_some(boost::asio::buffer(buf), error);
        // if error occurred, print it, except end of file error (eof) then continue
        if (error){
            if(error != boost::asio::error::eof){
                std::cerr << "Read error: " << error.message() << std::endl;
            }
            continue;
        }
        // store the received data in msg
        msg = std::string(buf.data(), len);
        // print the received data
        //std::cout << "Received: " << std::string(buf.data(), len) << std::endl;

        break;
    }
    socket_.close(); // close connection to server
    return 0;

}

int Server::receive_file(const std::string& filename){
    // check if socket is open
    if (!socket_.is_open()) {
        std::cerr << "Socket is not open" << std::endl;
        return -1;
    }
    try {
        boost::array<char, 100000> buf;
        boost::system::error_code error;  // error code for transmision

        // Open a new file to save the received data
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to create file" << std::endl;
            return 1;
        }
        while (true) {
            size_t len = socket_.read_some(boost::asio::buffer(buf), error); // Read content from the socket
            if (error == boost::asio::error::eof) {
                // End of file reached
                break;
            } else if (error) {
                std::cerr << "Error occurred during reading: " << error.message() << std::endl;
                return 1;
            }

            // Write the received data to the file
            file.write(buf.data(), len);
        }
        // Close the file
        file.close();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    socket_.close(); // close connection to server
    return 0;
}