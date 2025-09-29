# Network architecture

![Network Architecture Diagram](/img/networkArchitecture.png)

The UML diagram above gives a quick overview of the network class implementation of this project. The network class is passive and can be used in other functions to connect to another device. The 'msg' value in the send function contains the message to send to the opponent while the 'msg' value is the variable where the received message will be stored.

## Use

Client

```cpp
        Client client = Client(IP, std::to_string(PORT)); 

        client.send("Hello from client\n");

        std::string received_msg = "";
        client.receive(received_msg);
        std::cout << received_msg << std::endl;
```

Server

```cpp
        boost::asio::io_context io_context;
        Server server = Server(io_context, PORT);;

        std::string received_msg = "";
        server.receive(received_msg);
        std::cout << received_msg << std::endl;
        
        server.send("Hello from server\n");
```

## Test

While starting the program you can chose between the modes "client" and "server". This can be set at the execution in the command line.

- To compile the project just use:

```bash
make
```

- To execute the protocol you have to specify the mode without any flags
- If the mode is client then you have to specify the ip address of the server aswell (the default value is "localhost")
- If you want to use a certain port specify with the flag -p (the default value is "4444")

(For allowing the access of the network stack it is necassary to use sudo)

```bash
sudo ./keywordSearch.o client -a [ip] -p [port] 
sudo ./keywordSearch.o server -p [port]
```
