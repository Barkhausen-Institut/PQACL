#include <gtest/gtest.h>
#include <thread>
#include <iostream>
#include <fstream> 
#include <string>
#include "../Server.h"
#include "../Client.h"


// Function to compare two files for equality
bool compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1, std::ios::binary | std::ios::ate);
    std::ifstream f2(file2, std::ios::binary | std::ios::ate);

    if (f1.fail() || f2.fail()) {
        return false; // Failed to open files
    }

    if (f1.tellg() != f2.tellg()) {
        return false; // File sizes are different
    }

    f1.seekg(0);
    f2.seekg(0);

    std::istreambuf_iterator<char> it1(f1);
    std::istreambuf_iterator<char> it2(f2);

    std::istreambuf_iterator<char> end;

    while (it1 != end && it2 != end) {
        if (*it1 != *it2) {
            return false; // Files are different
        }

        ++it1;
        ++it2;
    }

    return true; // Files are equal
}

TEST(NetworkTests, ServerClientCommunication) {

    std::string client_msg = "Hello from client\n this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string";
    std::string server_msg = "Hello from server\n this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string this is a very long string";

    std::string client_received_msg = "";
    std::string server_received_msg = "";

    std::thread client_thread([&client_msg, &client_received_msg] {
        std::string IP = "localhost";
        int PORT = 4000;

        Client client = Client(IP, std::to_string(PORT)); 

        client.send(client_msg);

        client.receive(client_received_msg);

    });


    std::thread server_thread([&server_msg, &server_received_msg] {
        boost::asio::io_context io_context;
        Server server = Server(io_context, 4000);;
        
        server.receive(server_received_msg);
       
        server.send(server_msg);
    });
    client_thread.join();
    server_thread.join();

    EXPECT_EQ(client_msg, server_received_msg);
    EXPECT_EQ(server_msg, client_received_msg);
    
}

TEST(NetworkTests1, ServerClientCommunication_with_bin_file) {

    std::string send_file_path = "../data/ds1_proof.bin";
    std::string receive_file_path = "../data/ds1_proof_received_on_client.bin";
    std::string msg = "I want to receive a file";

    std::thread server_thread([&msg, &send_file_path] {
        boost::asio::io_context io_context;
        Server server = Server(io_context, 4001);;
        
        std::string server_received_msg = "";
        server.receive(server_received_msg);
        if (server_received_msg == msg){
            server.send_file(send_file_path);
        }
    });

    std::thread client_thread([&msg, &receive_file_path] {
        std::string IP = "localhost";
        int PORT = 4001;

        Client client = Client(IP, std::to_string(PORT)); 

        client.send(msg);

        client.receive_file(receive_file_path);

    });

    client_thread.join();
    server_thread.join();

    ASSERT_TRUE(compareFiles(send_file_path, receive_file_path));
}


TEST(NetworkTests2, ServerReceicesBinFile) {

    std::string send_file_path = "../data/ds1_proof.bin";
    std::string receive_file_path = "../data/ds1_proof_received_on_server.bin";
    std::string msg = "I want to send a message";

    std::thread server_thread([&msg, &receive_file_path] {
        boost::asio::io_context io_context;
        Server server = Server(io_context, 4001);;
        
        std::string server_received_msg = "";
        server.receive(server_received_msg);
        if (server_received_msg == msg){
            server.receive_file(receive_file_path);
        }
    });

    std::thread client_thread([&msg, &send_file_path] {
        std::string IP = "localhost";
        int PORT = 4001;

        Client client = Client(IP, std::to_string(PORT)); 

        client.send(msg);

        client.send_file(send_file_path);

    });

    client_thread.join();
    server_thread.join();

    ASSERT_TRUE(compareFiles(send_file_path, receive_file_path));
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}