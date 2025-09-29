#include "Client.h"

Client::Client(std::string ip, std::string port): resolver_(io_context_), socket_(io_context_) {
    endpoints_ = resolver_.resolve(ip, port);
}

int Client::send(std::string msg){
    try{
        boost::asio::connect(socket_, endpoints_); // open connection to server
        boost::system::error_code error_code;  // debug information in case of error
        boost::asio::write(socket_, boost::asio::buffer(msg), error_code); // 

        // print error if occuring
        if (error_code) {
            std::cerr << "Error occurred during writing: " << error_code.message() << std::endl;
            exit(1);
        }

    } catch (std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

int Client::send_data(std::string msg){
    if(socket_.is_open()){
        socket_.close(); // close connection to server
    }

    try{
        boost::asio::connect(socket_, endpoints_); // open connection to server
        boost::system::error_code error_code;  // debug information in case of error
        boost::asio::write(socket_, boost::asio::buffer(msg), error_code); // 

        // print error if occuring
        if (error_code) {
            std::cerr << "Error occurred during writing: " << error_code.message() << std::endl;
            exit(1);
        }

    } catch (std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    socket_.close(); // close connection to server
    return 0;
}

int Client::send_file(const std::string& filename){
    try
    {
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

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    socket_.close();    // close the socket
    return 0;
    
}

int Client::receive(std::string& msg){
    try {

        boost::array<char, 100000> buf;  // receiving buffer
        boost::system::error_code error;  // error code for transmision
        size_t len = socket_.read_some(boost::asio::buffer(buf), error); // reading content on socket
        if (error) {
            std::cerr << "Error occurred during reading: " << error.message() << std::endl;
            exit(1);
        }
        msg = std::string(buf.data(), len); // store data from server in msg
        // std::cout.write(buf.data(), len); // print data from server 

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    socket_.close(); // close connection to server
    return 0;
}

int Client::receive_file(const std::string& filename){
    try {
        boost::array<char, 100000> buf;    // buffer needs to be large enough to hold the data
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