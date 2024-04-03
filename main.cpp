#include <NTL/vec_ZZ_p.h>
#include <cmath>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <tuple>
#include <fstream>
#include <chrono>

#include "PVSS/PVSSServer.h"
#include "PVSS/PVSS.h"
#include "PVSS/ConnectionHandler.h"
#include "SSSharing/SSSharing.h"
#include "Network/Server.h"
#include "Network/Client.h"
#include "DPF/dpf.h"
#include "PVSS/helperFunctions.h"
#include "PACL/DataServerProtocol.h"


int main(int argc, char *argv[]){
    // Variables for the time measurements
    std::chrono::milliseconds duration_total;                       
    std::chrono::milliseconds duration_setup_pvss_server1;          
    std::chrono::milliseconds duration_setup_pvss_server2;          
    std::chrono::milliseconds duration_verify_proof_server1;        
    std::chrono::milliseconds duration_verify_proof_server2;        
    std::chrono::milliseconds duration_proof_user;                  
    std::chrono::milliseconds duration_verify_proof_user_server1;   
    std::chrono::milliseconds duration_verify_proof_user_server2;
    std::chrono::milliseconds duration_decode_pvss_message_server1;
    std::chrono::milliseconds duration_decode_pvss_message_server2;   
    std::chrono::milliseconds duration_setup_dpf;                   
    std::chrono::milliseconds duration_setup_sss;                   
    std::chrono::milliseconds duration_calculate_tau_server1;        
    std::chrono::milliseconds duration_calculate_tau_server2;       

    std::chrono::high_resolution_clock::time_point duration_total_start = std::chrono::high_resolution_clock::now();




    // Check if all command line parameters are given
    // ./AccessControl <size of secret> <number of files> <reduncancy parameter> <lwe parameter> <delay>
    if (argc < 6) {
        std::cout << "Bitte geben Sie alle Argumente. \n ./AccessControl <size of secret> <number of files> <reduncancy parameter> <lwe parameter> <delay>" << std::endl;
        return 1;
    }
    /*
    Initiate general values for the overall scheme
    */
    const int mod_q = 15* std::pow(2,27)+1;    // modulus for the finite field
    NTL::ZZ_p::init(NTL::ZZ(mod_q));            // set modulus for finite field


    const int numberOfFiles = std::atoi(argv[2]);;                  // number of files to be shared
    const int secretSizeOfFiles = std::atoi(argv[1]);;              // size of the secret to be shared 
                                                                    // secretSizeOfFiles is the same as m in PVSS 
    const int delay = std::atoi(argv[5]);;                          // Delay in seconds to allow the dataservers to compute their proofs
    /*
    Configuration for the DPF scheme
    */
    int index = 1;                      // This is the index of the file the user wants to access
    size_t N = ceil(log2(numberOfFiles) + 3);           // This the maximum number of files (2^(N-3) = number of files)
    std::cout << "N: " << N << std::endl;
    /*
    Initiate values for the PVSS scheme
    */
    const int n = 2;                        // number of data servers
    const int m = secretSizeOfFiles;        // size of the secret to be shared
    const int k = std::atoi(argv[4]);;      // LWE parameter
    const int l = std::atoi(argv[3]);;      // encoding redundancy parameter
    const int smallness_s = 3;              // smallness parameter for s_i
    const int smallness_e = 3;              // smallness parameter for e_i
    const std::vector<int> ports = {30001, 30002};
    const std::vector<std::string> ips = {"localhost", "localhost"};
   
    std::vector<NTL::vec_ZZ_p> s_i;             // vector of secret keys for each file

    NTL::vec_ZZ_p s_x;                          // secret key for the user (summation of s_i which the user has access to)     

    // Generate random secret keys for each file
    for(int i = 0; i < numberOfFiles; i++){
        NTL::vec_ZZ_p s_i_temp;
        s_i_temp.SetLength(secretSizeOfFiles);
        for(int j = 0; j < secretSizeOfFiles; j++){
            s_i_temp[j] = NTL::random_ZZ_p();
        }
        s_i.push_back(s_i_temp);
    }

    // Calculate the secret key for the user (allows access to file 2 [index 1])
    s_x.SetLength(secretSizeOfFiles);
    s_x = s_i[1];
    s_x = -s_x;

    NTL::mat_ZZ_p A_OUTER_LWS = NTL::random_mat_ZZ_p(secretSizeOfFiles, secretSizeOfFiles); // matrix A for the outer LWE
    std::vector<NTL::vec_ZZ_p> publicVectors;                                               // vector of public vectors for the outer LWE
    // calculate the public vectors for each file for the outer LWE
    for(int i = 0; i < numberOfFiles; i++){
        NTL::vec_ZZ_p publicVector;
        publicVector.SetLength(secretSizeOfFiles);
        publicVector = A_OUTER_LWS * s_i[i];
        publicVectors.push_back(publicVector);
    }

    const NTL::mat_ZZ_p A = NTL::random_mat_ZZ_p(k, k);    // Create public matrix A

    const NTL::vec_ZZ_p privateKey;                         // private key for the user
    std::vector<NTL::vec_ZZ_p> TAUs;                        // vector of TAUs to be summed up for the final TAU, used for communication between the threads

    std::chrono::high_resolution_clock::time_point duration_setup_pvss_server1_start = std::chrono::high_resolution_clock::now();

    // Thread for DataServer1
    std::thread dataServer1([&A, &n, &m, &l, &k, &mod_q, &ports, &smallness_e, &smallness_s, &A_OUTER_LWS, &publicVectors, &numberOfFiles, &secretSizeOfFiles, &TAUs, &duration_setup_pvss_server1, &duration_setup_pvss_server1_start, &duration_verify_proof_user_server1, &duration_calculate_tau_server1, &duration_decode_pvss_message_server1] {
        std::cout << "DataServer1: Creating DataServer\n" << std::endl;

        // Initialize the network connection and bind the server to the port
        boost::asio::io_context io_context;
        Server server = Server(io_context, ports[0]);;

        // Create PVSSServer object 
        PVSSServer d1 = PVSSServer(mod_q, m, l, k, A, smallness_e, smallness_s);
        
        // Export the A matrix to a JSON file as a communication between the cpp and the riscZero prover written in rust.
        d1.export_to_json("ds1_data/DataServer1.json", A);
        d1.proof_public_key(m, l, k, "ds1_data/DataServer1.json", "ds1_data/ds1_proof.bin");

        std::chrono::high_resolution_clock::time_point duration_setup_pvss_server1_end = std::chrono::high_resolution_clock::now();
        duration_setup_pvss_server1 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_setup_pvss_server1_end - duration_setup_pvss_server1_start);

        // Create the data server with the already available values. This class contains the values and protokoll for the outer LWS.
        DataServerProtocol ds1 = DataServerProtocol(A_OUTER_LWS, publicVectors, numberOfFiles, secretSizeOfFiles, mod_q, 0);

        // Interaction loop with the user via the network.
        while(true){
            std::string msg;
            server.receive(msg);
            // Send the calculated zero knowledge proof to the user
            if(msg == "getProofB_i"){
                std::cout << "DataServer1: Sending proof_b_i" << std::endl;
                server.send_file("ds1_data/ds1_proof.bin");
                continue;
            }
            // Validate the access of the user by verifying the zero knowledge proof, decoding the access key and calculating the TAU for this server
            else if(msg == "validateAccess"){
                std::cout << "DataServer1: Validating access" << std::endl;
                server.receive_file("ds1_data/ac_proof.bin");

                std::chrono::high_resolution_clock::time_point duration_verify_proof_user_server1_start = std::chrono::high_resolution_clock::now();
                d1.verify_access_proof("ds1_data/ac_proof.bin", "ds1_data/AccessControlProofValues.json");
                std::chrono::high_resolution_clock::time_point duration_verify_proof_user_server1_end = std::chrono::high_resolution_clock::now();
                duration_verify_proof_user_server1 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_verify_proof_user_server1_end - duration_verify_proof_user_server1_start);

                std::chrono::high_resolution_clock::time_point decode_pvss_message_server1_start = std::chrono::high_resolution_clock::now();                
                d1.decodeAccessKey(l, m, mod_q, n, 1, "ds1_data/AccessControlProofValues.json");
                ds1.setSSSharingKeyY(d1.getDecodedMessageParts());
                std::chrono::high_resolution_clock::time_point decode_pvss_message_server1_end = std::chrono::high_resolution_clock::now();
                duration_decode_pvss_message_server1 = std::chrono::duration_cast<std::chrono::milliseconds>(decode_pvss_message_server1_end - decode_pvss_message_server1_start);

                std::chrono::high_resolution_clock::time_point duration_calculate_tau_server1_start = std::chrono::high_resolution_clock::now();
                TAUs.push_back(ds1.calculateTau());
                std::chrono::high_resolution_clock::time_point duration_calculate_tau_server1_end = std::chrono::high_resolution_clock::now();
                duration_calculate_tau_server1 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_calculate_tau_server1_end - duration_calculate_tau_server1_start);
                break;
            }
            // Receive the DPF key from the user and save it.
            else if(msg == "sendDPFKey"){
                std::cout << "DataServer1: Receiving DPF key" << std::endl;
                std::string dpf_key_json;
                server.receive_data(dpf_key_json);
                nlohmann::json j = nlohmann::json::parse(dpf_key_json);
                std::vector<uint8_t> dpf_key = jsonToVector_uint(j);
                size_t key_size_logn = j["NrFiles"];
                ds1.setDPFKey(dpf_key, key_size_logn);
                continue; 
            }
            // Receive the x shares from the user and save them.
            else if(msg == "sendSSShareXs"){
                std::cout << "DataServer1: Receiving shares" << std::endl;
                std::string x_json;
                server.receive_data(x_json);
                nlohmann::json j = nlohmann::json::parse(x_json);
                std::vector<NTL::vec_ZZ_p> x = jsonToVector_vec_ZZ_p(j);
                ds1.setSSSharingKeyX(x);
                continue;
            } 
            // If the request is invalid, send a message to the user.
            else{
                std::cout << "DataServer1: Invalid request" << std::endl;
                server.send("Invalid request");
                continue;
            }
        }     
    });

    // Delay for the second server to allow the first server to compute its proof and measure the time independently
    std::chrono::seconds duration(delay);
    std::this_thread::sleep_for(duration);
    std::chrono::high_resolution_clock::time_point duration_setup_pvss_server2_start = std::chrono::high_resolution_clock::now();

    // Thread for DataServer2
    std::thread dataServer2([&A, &n, &m, &l, &k, &mod_q, &ports, &smallness_e, &smallness_s, &A_OUTER_LWS, &publicVectors, &numberOfFiles, &secretSizeOfFiles, &TAUs, &duration_setup_pvss_server2, &duration_setup_pvss_server2_start, &duration_verify_proof_user_server2, &duration_calculate_tau_server2, &duration_decode_pvss_message_server2] {
        std::cout << "DataServer2: Creating DataServer\n" << std::endl;

        // Initialize the network connection and bind the server to the port
        boost::asio::io_context io_context;
        Server server = Server(io_context, ports[1]);;

        // Create PVSSServer object
        PVSSServer d1 = PVSSServer(mod_q, m, l, k, A, smallness_e, smallness_s);
        
        // Export the A matrix to a JSON file as a communication between the cpp and the riscZero prover written in rust.
        d1.export_to_json("ds2_data/DataServer2.json", A);
        d1.proof_public_key(m, l, k, "ds2_data/DataServer2.json", "ds2_data/ds2_proof.bin");
        
        std::chrono::high_resolution_clock::time_point duration_setup_pvss_server2_end = std::chrono::high_resolution_clock::now();
        duration_setup_pvss_server2 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_setup_pvss_server2_end - duration_setup_pvss_server2_start);

        // Create the data server with the already available values. This class contains the values and protokoll for the outer LWS.
        DataServerProtocol ds1 = DataServerProtocol(A_OUTER_LWS, publicVectors, numberOfFiles, secretSizeOfFiles, mod_q, 1);

        // Interaction loop with the user via the network.
        while(true){
            std::string msg;
            server.receive(msg);
            // Send the calculated zero knowledge proof to the user
            if(msg == "getProofB_i"){
                std::cout << "DataServer2: Sending proof_b_i" << std::endl;
                server.send_file("ds2_data/ds2_proof.bin");
                continue;
            }
            // Validate the access of the user by verifying the zero knowledge proof, decoding the access key and calculating the TAU for this server
            else if(msg == "validateAccess"){
                std::cout << "DataServer2: Validating access" << std::endl;
                server.receive_file("ds2_data/ac_proof.bin");

                std::chrono::high_resolution_clock::time_point duration_verify_proof_user_server2_start = std::chrono::high_resolution_clock::now();
                d1.verify_access_proof("ds2_data/ac_proof.bin", "ds2_data/AccessControlProofValues.json");
                std::chrono::high_resolution_clock::time_point duration_verify_proof_user_server2_end = std::chrono::high_resolution_clock::now();
                duration_verify_proof_user_server2 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_verify_proof_user_server2_end - duration_verify_proof_user_server2_start);


                std::chrono::high_resolution_clock::time_point decode_pvss_message_server2_start = std::chrono::high_resolution_clock::now();                
                d1.decodeAccessKey(l, m, mod_q, n, 2, "ds2_data/AccessControlProofValues.json");
                ds1.setSSSharingKeyY(d1.getDecodedMessageParts());
                std::chrono::high_resolution_clock::time_point decode_pvss_message_server2_end = std::chrono::high_resolution_clock::now();
                duration_decode_pvss_message_server2 = std::chrono::duration_cast<std::chrono::milliseconds>(decode_pvss_message_server2_end - decode_pvss_message_server2_start);


                std::chrono::high_resolution_clock::time_point duration_calculate_tau_server2_start = std::chrono::high_resolution_clock::now();
                TAUs.push_back(ds1.calculateTau());
                std::chrono::high_resolution_clock::time_point duration_calculate_tau_server2_end = std::chrono::high_resolution_clock::now();
                duration_calculate_tau_server2 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_calculate_tau_server2_end - duration_calculate_tau_server2_start);
                break;
            }
            // Receive the DPF key from the user and save it.
            else if(msg == "sendDPFKey"){
                std::cout << "DataServer2: Receiving DPF key" <<  std::endl;
                std::string dpf_key_json;
                server.receive_data(dpf_key_json);
                nlohmann::json j = nlohmann::json::parse(dpf_key_json);
                std::vector<uint8_t> dpf_key = jsonToVector_uint(j);
                size_t key_size_logn = j["NrFiles"];
                ds1.setDPFKey(dpf_key, key_size_logn);
                continue; 
            }
            // Receive the x shares from the user and save them.
            else if(msg == "sendSSShareXs"){
                std::cout << "DataServer2: Receiving shares" << std::endl;
                std::string x_json;
                server.receive_data(x_json);
                nlohmann::json j = nlohmann::json::parse(x_json);
                std::vector<NTL::vec_ZZ_p> x = jsonToVector_vec_ZZ_p(j);
                ds1.setSSSharingKeyX(x);
                continue;
            }
            else{
                std::cout << "DataServer2: Invalid request" << std::endl;
                server.send("Invalid request");
                continue;
            }
        }     
    });

    std::this_thread::sleep_for(duration);

    // Thread for AccessControl of the user
    std::thread user([&A, &privateKey, &mod_q, &m, &n, &l, &k, &ips, &ports, &smallness_e, &smallness_s, &s_x, &secretSizeOfFiles, &index, &N, 
                    &duration_proof_user, &duration_verify_proof_server1, &duration_verify_proof_server2, &duration_setup_dpf, &duration_setup_sss] {
        std::cout << "User: Creating AccessControl" << std::endl;

        // Create connection to the servers 
        ConnectionHandler server1 = ConnectionHandler(ips[0], ports[0]);
        ConnectionHandler server2 = ConnectionHandler(ips[1], ports[1]);

        std::chrono::high_resolution_clock::time_point duration_setup_dpf_start = std::chrono::high_resolution_clock::now();
        // Create DPF keys
        std::pair<std::vector<uint8_t>, std::vector<uint8_t>> keys = DPF::Gen(index * 8, N);     // Generate the DPF keys with alpha = index * (8 bits) and N files
        std::vector<uint8_t> dpf_key_server1 = keys.first;
        std::vector<uint8_t> dpf_key_server2 = keys.second;
        std::chrono::high_resolution_clock::time_point duration_setup_dpf_end = std::chrono::high_resolution_clock::now();
        duration_setup_dpf = std::chrono::duration_cast<std::chrono::milliseconds>(duration_setup_dpf_end - duration_setup_dpf_start);

        // Convert the DPF keys to JSON
        nlohmann::json dpf_key_server1_json = vectorToJson_uint(dpf_key_server1);
        nlohmann::json dpf_key_server2_json = vectorToJson_uint(dpf_key_server2);
        dpf_key_server1_json["NrFiles"] = N;
        dpf_key_server2_json["NrFiles"] = N;

        // Transmit the DPF keys to the servers
        server1.send_data("sendDPFKey", dpf_key_server1_json.dump());
        server2.send_data("sendDPFKey", dpf_key_server2_json.dump());

        std::chrono::high_resolution_clock::time_point duration_setup_sss_start = std::chrono::high_resolution_clock::now();
        // Calculate the shared keys from the private key 
        std::tuple<std::vector<NTL::vec_ZZ_p>, NTL::vec_ZZ_p, NTL::vec_ZZ_p> shares = SSSharing::generateShares(n, s_x, secretSizeOfFiles, mod_q);
        // Tuple consists of:
        // 1. vector of x values for both servers
        // 2. y value for server 1
        // 3. y value for server 2
        
        std::chrono::high_resolution_clock::time_point duration_setup_sss_end = std::chrono::high_resolution_clock::now();
        duration_setup_sss = std::chrono::duration_cast<std::chrono::milliseconds>(duration_setup_sss_end - duration_setup_sss_start);

        std::cout << "User: Calculating shares done" << std::endl;


        // Send the x vectors to the servers
        server1.send_data("sendSSShareXs", vector_of_vec_ZZ_p_ToJson(std::get<0>(shares)).dump());
        server2.send_data("sendSSShareXs", vector_of_vec_ZZ_p_ToJson(std::get<0>(shares)).dump());


        // for each tuple execute the pvss scheme sending the first x and y value to the first server, the second to the second server
        // the x values are the secret and are transmitted openly to the servers and the y values are the shares and are transmitted via the pvss scheme
        
        // Create PVSS object
        PVSS pvss(mod_q, m, n, l, k, A, smallness_e, smallness_s);

        // getBProof from the servers and verify 
        server1.getB_i_proof_from_server("public_key_proof_server1.bin");
        server2.getB_i_proof_from_server("public_key_proof_server2.bin");
        
        std::chrono::high_resolution_clock::time_point duration_verify_proof_server1_start = std::chrono::high_resolution_clock::now();
        if(!pvss.verify_proof_from_server("public_key_proof_server1.bin", "DataServerProofResults1.json")) {
            std::cout << "Public key proof server 1 is not valid" << std::endl;
            exit(1);
        };
        std::chrono::high_resolution_clock::time_point duration_verify_proof_server1_end = std::chrono::high_resolution_clock::now();
        duration_verify_proof_server1 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_verify_proof_server1_end - duration_verify_proof_server1_start);

        std::chrono::high_resolution_clock::time_point duration_verify_proof_server2_start = std::chrono::high_resolution_clock::now();
        if(!pvss.verify_proof_from_server("public_key_proof_server2.bin", "DataServerProofResults2.json")){
            std::cout << "Public key proof server 2 is not valid" << std::endl;
            exit(1);
        };
        std::chrono::high_resolution_clock::time_point duration_verify_proof_server2_end = std::chrono::high_resolution_clock::now();
        duration_verify_proof_server2 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_verify_proof_server2_end - duration_verify_proof_server2_start);

        std::chrono::high_resolution_clock::time_point duration_proof_user_start = std::chrono::high_resolution_clock::now();

        // give proof to the PVSS object
        pvss.combineBFromProofs({"DataServerProofResults1.json", "DataServerProofResults2.json"});
        // set the private key
        NTL::vec_ZZ_p privateKey;
        NTL::vec_ZZ_p y0 = std::get<1>(shares);
        NTL::vec_ZZ_p y1 = std::get<2>(shares);
        privateKey.append(y0);
        privateKey.append(y1);

        pvss.setPrivateKey(privateKey);

        // calculate C1 and C2
        pvss.calculateC1AndC2();
        // export C1 and C2 to json 
        pvss.export_to_json("AccessControlProofValues.json");
        // create proof for C1 and C2
        pvss.proveC1_C2(m,n,l,k, "AccessControlProofValues.json", "ac_proof.bin");
        // send proof to the servers

        std::chrono::high_resolution_clock::time_point duration_proof_user_end = std::chrono::high_resolution_clock::now();
        duration_proof_user = std::chrono::duration_cast<std::chrono::milliseconds>(duration_proof_user_end - duration_proof_user_start);

        server1.sendProofToServer("ac_proof.bin");
        server2.sendProofToServer("ac_proof.bin");

    });

    dataServer1.join();
    dataServer2.join();
    user.join();


    // Adding all TAUs together to determine the final TAU and if the user has access to the file if TAU = [0, ..., 0]
    NTL::vec_ZZ_p TAU;
    TAU.SetLength(secretSizeOfFiles);
    for(size_t i = 0; i < TAUs.size(); i++){
        TAU += TAUs[i];
    }
    
    // Print the final TAU
    std::cout << "Final TAU: " << TAU << std::endl;

    std::chrono::high_resolution_clock::time_point duration_total_end = std::chrono::high_resolution_clock::now();
    duration_total = std::chrono::duration_cast<std::chrono::milliseconds>(duration_total_end - duration_total_start);


    // Export the time measurements to a JSON file for further analysis
    nlohmann::json j;
    j["TotalDuration"] = duration_total.count();
    j["SetupPVSSServer1"] = duration_setup_pvss_server1.count();
    j["SetupPVSSServer2"] = duration_setup_pvss_server2.count();
    j["VerifyProofServer1"] = duration_verify_proof_server1.count();
    j["VerifyProofServer2"] = duration_verify_proof_server2.count();
    j["ProofUser"] = duration_proof_user.count();
    j["VerifyProofUserServer1"] = duration_verify_proof_user_server1.count();
    j["VerifyProofUserServer2"] = duration_verify_proof_user_server2.count();
    j["DecodePVSSMessageServer1"] = duration_decode_pvss_message_server1.count();
    j["DecodePVSSMessageServer2"] = duration_decode_pvss_message_server2.count();
    j["SetupDPF"] = duration_setup_dpf.count();
    j["SetupSSS"] = duration_setup_sss.count();
    j["CalculateTauServer1"] = duration_calculate_tau_server1.count();
    j["CalculateTauServer2"] = duration_calculate_tau_server2.count();
    j["Tau"] = convert_ZZ_p_VectorTo_long_vector(TAU);

    // Write to file
    std::string filename = "Files_" + std::to_string(numberOfFiles) + "_size_" + std::to_string(secretSizeOfFiles) + "_delay_" + std::to_string(delay) + "_redundancyEnc_" + std::to_string(l) + "_redundancyLWE_" + std::to_string(k) +".json";
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
    } else {
        std::cerr << "Unable to open file " << filename << std::endl;
    }

    return 0;
}