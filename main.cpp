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
    std::chrono::milliseconds duration_setup_pvss_user;
    std::chrono::milliseconds duration_setup_dpf;
    std::chrono::milliseconds duration_setup_sss;
    std::chrono::milliseconds duration_zkp_enc_prove;
    std::chrono::milliseconds duration_zkp_enc_verify;
    std::chrono::milliseconds duration_zkp_dec_prove;
    std::chrono::milliseconds duration_zkp_dec_verify;
    std::chrono::milliseconds duration_calc_user;
    std::chrono::milliseconds duration_calc_server1;
    std::chrono::milliseconds duration_calc_server2;

    std::chrono::high_resolution_clock::time_point duration_total_start = std::chrono::high_resolution_clock::now();

    // Check if all command line parameters are given
    // ./AccessControl <number of files> <size of secret> <reduncancy parameter> <lwe parameter>
    if (argc < 5) {
        std::cout << "./AccessControl <number of files> <size of secret> <reduncancy parameter> <lwe parameter>" << std::endl;
        return 1;
    }
    /*
    Initiate general values for the overall scheme
    */
    const int mod_q = 15* std::pow(2,27)+1;    // modulus for the finite field
    NTL::ZZ_p::init(NTL::ZZ(mod_q));            // set modulus for finite field


    const int numberOfFiles = std::atoi(argv[1]);;                  // number of files to be shared
    const int secretSizeOfFiles = std::atoi(argv[2]);;              // size of the secret to be shared 
                                                                    // secretSizeOfFiles is the same as m in PVSS 
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
    const int l = std::atoi(argv[3]);;      // encoding redundancy parameter
    const int k = std::atoi(argv[4]);;      // LWE parameter
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
    std::vector<NTL::vec_ZZ_p> TAUs;                        // vector of TAUs to be summed up for the final TAU, used for communication between the threads

    // Thread for DataServer1
    std::thread dataServer1([&A, &n, &m, &l, &k, &mod_q, &ports, &smallness_e, &smallness_s, &A_OUTER_LWS, &publicVectors, &numberOfFiles, &secretSizeOfFiles, &TAUs, &duration_setup_pvss_server1, &duration_zkp_enc_verify, &duration_zkp_dec_prove, &duration_calc_server1] {
        std::cout << "DataServer1: Creating DataServer" << std::endl;

        // Initialize the network connection and bind the server to the port
        boost::asio::io_context io_context;
        Server server = Server(io_context, ports[0]);;

        std::chrono::high_resolution_clock::time_point duration_setup_pvss_server1_start = std::chrono::high_resolution_clock::now();

        // Create PVSSServer object 
        PVSSServer d1 = PVSSServer(mod_q, m, l, k, A, smallness_e, smallness_s);

        // Create the data server with the already available values. This class contains the values and protokoll for the outer LWS.
        DataServerProtocol ds1 = DataServerProtocol(A_OUTER_LWS, publicVectors, numberOfFiles, secretSizeOfFiles, mod_q, 0);

        std::chrono::high_resolution_clock::time_point duration_setup_pvss_server1_end = std::chrono::high_resolution_clock::now();
        duration_setup_pvss_server1 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_setup_pvss_server1_end - duration_setup_pvss_server1_start);

        // Export the public key to a JSON file for the user to access
        d1.export_to_json("ds1_data/DataServer1.json", A);

        // Interaction loop with the user via the network.
        while(true){
            std::string msg;
            server.receive(msg);
            // Send the public key to the user
            if(msg == "getB_i"){
                std::cout << "DataServer1: Sending B_i" << std::endl;
                server.send_file("ds1_data/DataServer1.json");
                continue;
            }
            // Receive the encrypted access key from the user and save it.
            else if(msg == "validateAccessPre"){
                std::cout << "DataServer1: Receiving encrypted access key" << std::endl;
                server.receive_file("ds1_data/enc_verifier.json");
                continue;
            }
            // Validate the access of the user by verifying the zero knowledge proof, decoding the access key and calculating the TAU for this server
            else if(msg == "validateAccess"){
                std::cout << "DataServer1: Validating access" << std::endl;
                server.receive_file("ds1_data/enc_proof.bin");

                std::chrono::high_resolution_clock::time_point duration_zkp_enc_verify_start = std::chrono::high_resolution_clock::now();
                // d1.verify_enc(m, l, k, "ds1_data/enc_verifier.json", "ds1_data/enc_proof.bin");
                std::chrono::high_resolution_clock::time_point duration_zkp_enc_verify_end = std::chrono::high_resolution_clock::now();
                duration_zkp_enc_verify = std::chrono::duration_cast<std::chrono::milliseconds>(duration_zkp_enc_verify_end - duration_zkp_enc_verify_start);

                std::chrono::high_resolution_clock::time_point duration_calc_server1_start = std::chrono::high_resolution_clock::now();
                d1.decodeAccessKey(l, m, mod_q, "ds1_data/enc_verifier.json", "ds1_data/dec");
                ds1.setSSSharingKeyY(d1.getDecodedMessageParts());
                TAUs.push_back(ds1.calculateTau());
                std::chrono::high_resolution_clock::time_point duration_calc_server1_end = std::chrono::high_resolution_clock::now();
                duration_calc_server1 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_calc_server1_end - duration_calc_server1_start);

                std::chrono::high_resolution_clock::time_point duration_zkp_dec_prove_start = std::chrono::high_resolution_clock::now();
                // d1.prove_dec(m, l, k, "ds1_data/dec_prover.json", "ds1_data/dec_proof.bin");
                std::chrono::high_resolution_clock::time_point duration_zkp_dec_prove_end = std::chrono::high_resolution_clock::now();
                duration_zkp_dec_prove = std::chrono::duration_cast<std::chrono::milliseconds>(duration_zkp_dec_prove_end - duration_zkp_dec_prove_start);

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
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Thread for DataServer2
    std::thread dataServer2([&A, &n, &m, &l, &k, &mod_q, &ports, &smallness_e, &smallness_s, &A_OUTER_LWS, &publicVectors, &numberOfFiles, &secretSizeOfFiles, &TAUs, &duration_setup_pvss_server2, &duration_zkp_enc_verify, &duration_zkp_dec_prove, &duration_calc_server2] {
        std::cout << "DataServer2: Creating DataServer" << std::endl;

        // Initialize the network connection and bind the server to the port
        boost::asio::io_context io_context;
        Server server = Server(io_context, ports[1]);

        std::chrono::high_resolution_clock::time_point duration_setup_pvss_server2_start = std::chrono::high_resolution_clock::now();

        // Create PVSSServer object
        PVSSServer d1 = PVSSServer(mod_q, m, l, k, A, smallness_e, smallness_s);

        // Create the data server with the already available values. This class contains the values and protokoll for the outer LWS.
        DataServerProtocol ds1 = DataServerProtocol(A_OUTER_LWS, publicVectors, numberOfFiles, secretSizeOfFiles, mod_q, 1);

        std::chrono::high_resolution_clock::time_point duration_setup_pvss_server2_end = std::chrono::high_resolution_clock::now();
        duration_setup_pvss_server2 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_setup_pvss_server2_end - duration_setup_pvss_server2_start);

        // Export the public key to a JSON file for the user to access
        d1.export_to_json("ds2_data/DataServer2.json", A);

        // Interaction loop with the user via the network.
        while(true){
            std::string msg;
            server.receive(msg);
            // Send the public key to the user
            if(msg == "getB_i"){
                std::cout << "DataServer2: Sending B_i" << std::endl;
                server.send_file("ds2_data/DataServer2.json");
                continue;
            }
            // Receive the encrypted access key from the user and save it.
            else if(msg == "validateAccessPre"){
                std::cout << "DataServer2: Receiving encrypted access key" << std::endl;
                server.receive_file("ds2_data/enc_verifier.json");
                continue;
            }
            // Validate the access of the user by verifying the zero knowledge proof, decoding the access key and calculating the TAU for this server
            else if(msg == "validateAccess"){
                std::cout << "DataServer2: Validating access" << std::endl;
                server.receive_file("ds2_data/enc_proof.bin");

                // std::chrono::high_resolution_clock::time_point duration_zkp_enc_verify_start = std::chrono::high_resolution_clock::now();
                // d1.verify_enc(m, l, k, "ds2_data/enc_verifier.json", "ds2_data/enc_proof.bin");
                // std::chrono::high_resolution_clock::time_point duration_zkp_enc_verify_end = std::chrono::high_resolution_clock::now();
                // duration_zkp_enc_verify = std::chrono::duration_cast<std::chrono::milliseconds>(duration_zkp_enc_verify_end - duration_zkp_enc_verify_start);

                std::chrono::high_resolution_clock::time_point duration_calc_server2_start = std::chrono::high_resolution_clock::now();
                d1.decodeAccessKey(l, m, mod_q, "ds2_data/enc_verifier.json", "ds2_data/dec");
                ds1.setSSSharingKeyY(d1.getDecodedMessageParts());
                TAUs.push_back(ds1.calculateTau());
                std::chrono::high_resolution_clock::time_point duration_calc_server2_end = std::chrono::high_resolution_clock::now();
                duration_calc_server2 = std::chrono::duration_cast<std::chrono::milliseconds>(duration_calc_server2_end - duration_calc_server2_start);

                // std::chrono::high_resolution_clock::time_point duration_zkp_dec_prove_start = std::chrono::high_resolution_clock::now();
                // d1.prove_dec(m, l, k, "ds2_data/dec_prover.json", "ds2_data/dec_proof.bin");
                // std::chrono::high_resolution_clock::time_point duration_zkp_dec_prove_end = std::chrono::high_resolution_clock::now();
                // duration_zkp_dec_prove = std::chrono::duration_cast<std::chrono::milliseconds>(duration_zkp_dec_prove_end - duration_zkp_dec_prove_start);

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
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Thread for AccessControl of the user
    std::thread user([&A, &mod_q, &m, &n, &l, &k, &ips, &ports, &smallness_e, &smallness_s, &s_x, &secretSizeOfFiles, &index, &N,
                    &duration_setup_dpf, &duration_setup_sss, &duration_setup_pvss_user, &duration_zkp_enc_prove, &duration_calc_user] {
        std::cout << "User: Creating AccessControl" << std::endl;

        // Create connection to the servers 
        ConnectionHandler server1 = ConnectionHandler(ips[0], ports[0]);
        ConnectionHandler server2 = ConnectionHandler(ips[1], ports[1]);

        // getBi from the servers
        server1.getB_i_from_server("public_key_server1.json");
        server2.getB_i_from_server("public_key_server2.json");

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
        // the x values are transmitted openly to the servers and the y values are the shares and are transmitted via the pvss scheme

        std::chrono::high_resolution_clock::time_point duration_setup_pvss_user_start = std::chrono::high_resolution_clock::now();
        // Create PVSS object
        PVSS pvss(mod_q, m, n, l, k, A, smallness_e, smallness_s);
        std::chrono::high_resolution_clock::time_point duration_setup_pvss_user_end = std::chrono::high_resolution_clock::now();
        duration_setup_pvss_user = std::chrono::duration_cast<std::chrono::milliseconds>(duration_setup_pvss_user_end - duration_setup_pvss_user_start);

        pvss.loadBi({"public_key_server1.json", "public_key_server2.json"});

        std::chrono::high_resolution_clock::time_point duration_calc_user_start = std::chrono::high_resolution_clock::now();
        pvss.setPrivateKey(std::get<1>(shares),std::get<2>(shares));
        // calculate C1 and C2
        pvss.calculateC1AndC2();
        std::chrono::high_resolution_clock::time_point duration_calc_user_end = std::chrono::high_resolution_clock::now();
        duration_calc_user = std::chrono::duration_cast<std::chrono::milliseconds>(duration_calc_user_end - duration_calc_user_start);

        // export C1 and C2 to json 
        pvss.export_to_jsons({"enc1", "enc2"});
        server1.sendDataToServer("enc1_verifier.json");
        server2.sendDataToServer("enc2_verifier.json");

        std::chrono::high_resolution_clock::time_point duration_zkp_enc_prove_start = std::chrono::high_resolution_clock::now();
        // create proof for C1 and C2
        // pvss.proveC1_C2(m, l, k, "enc1_prover.json", "enc1_proof.bin");
        // pvss.proveC1_C2(m, l, k, "enc2_prover.json", "enc2_proof.bin");
        std::chrono::high_resolution_clock::time_point duration_zkp_enc_prove_end = std::chrono::high_resolution_clock::now();
        duration_zkp_enc_prove = std::chrono::duration_cast<std::chrono::milliseconds>(duration_zkp_enc_prove_end - duration_zkp_enc_prove_start);

        // send proof to the servers
        server1.sendProofToServer("enc1_proof.bin");
        server2.sendProofToServer("enc1_proof.bin"); // only for benchmarking: server2 doesn't perform proving or verifying
    });

    dataServer1.join();
    dataServer2.join();
    user.join();

    std::chrono::high_resolution_clock::time_point duration_zkp_dec_verify_start = std::chrono::high_resolution_clock::now();
    // Verify the decryption
    // verify_dec(m, l, k, "ds1_data/dec_verifier.json", "ds1_data/dec_proof.bin");
    // verify_dec(m, l, k, "ds2_data/dec_verifier.json", "ds2_data/dec_proof.bin");
    std::chrono::high_resolution_clock::time_point duration_zkp_dec_verify_end = std::chrono::high_resolution_clock::now();
    duration_zkp_dec_verify = std::chrono::duration_cast<std::chrono::milliseconds>(duration_zkp_dec_verify_end - duration_zkp_dec_verify_start);

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
    j["Total_Duration"] = duration_total.count();
    j["Setup_PVSS_Server1"] = duration_setup_pvss_server1.count();
    j["Setup_PVSS_Server2"] = duration_setup_pvss_server2.count();
    j["Setup_PVSS_User"] = duration_setup_pvss_user.count();
    j["Setup_DPF"] = duration_setup_dpf.count();
    j["Setup_SSS"] = duration_setup_sss.count();
    j["ZKP_STARK_Enc_Prove"] = duration_zkp_enc_prove.count();
    j["ZKP_STARK_Enc_Verify"] = duration_zkp_enc_verify.count();
    j["ZKP_STARK_Enc_Size"] = 0;
    j["ZKP_STARK_Dec_Prove"] = duration_zkp_dec_prove.count();
    j["ZKP_STARK_Dec_Verify"] = duration_zkp_dec_verify.count();
    j["ZKP_STARK_Dec_Size"] = 0;
    j["Calc_Server1"] = duration_calc_server1.count();
    j["Calc_Server2"] = duration_calc_server2.count();
    j["Calc_User"] = duration_calc_user.count();
    j["Tau"] = "TODO"; // convert_ZZ_p_VectorTo_long_vector(TAU);
    j["ZKP_BP_Enc_Prove"] = 0;
    j["ZKP_BP_Enc_Verify"] = 0;
    j["ZKP_BP_Enc_Size"] = 0;
    j["ZKP_BP_Dec_Prove"] = 0;
    j["ZKP_BP_Dec_Verify"] = 0;
    j["ZKP_BP_Dec_Size"] = 0;
    j["ZKP___"] = 0;

    // Write to file
    std::string filename = "Files_" + std::to_string(numberOfFiles) + "_size_" + std::to_string(secretSizeOfFiles) + "_redundancyEnc_" + std::to_string(l) + "_redundancyLWE_" + std::to_string(k) +".json";
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
    } else {
        std::cerr << "Unable to open file " << filename << std::endl;
    }

    return 0;
}