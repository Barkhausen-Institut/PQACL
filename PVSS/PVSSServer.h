#ifndef PVSS_SERVER_H
#define PVSS_SERVER_H

#include <nlohmann/json.hpp>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <NTL/vec_ZZ_p.h>
#include <NTL/mat_ZZ_p.h>
#include <vector>
#include "helperFunctions.h"


/*
This PVSSServer class represents the GHL+ scheme of our paper.
It is an implementation of a LWE encryption described in the PVSS_GHL+_scheme.pdf file.
It implements the calculation for the data server side.
*/
class PVSSServer{
    public:
        // initializes data server with the modulus q, 
        // m is the length of the secret vector 
        // l the encoding reduncancy parameter
        // k a LWE security parameter
        // A the public matrix 
        // the smallness of e and the smallness of s
        PVSSServer(const int& mod_q, const int& m, const int& l, const int& k, const NTL::mat_ZZ_p& A, const int& smallness_e, const int& smallness_s);
        
        // exports the values to a json file, which is then used as an input of the risc0 framework 
        void export_to_json(const std::string& filename, const NTL::mat_ZZ_p& A);
        void export_to_json(const std::string& filename, const NTL::vec_ZZ_p& c_1, const NTL::vec_ZZ_p& d_i, const NTL::vec_ZZ_p& x_i);

        // creates a proof of decryption with the risc0 framework
        void prove_dec(const int& m, const int& l, const int& k, const std::string& json_data, const std::string& proof_export_path);

        // verifies the zero knowledge proof of encryption with the risc0 framework
        bool verify_enc(const int& m, const int& l, const int& k, const std::string& json_data_export_path, const std::string& proof_path);

        // Read a vector from a json file to a NTL::vec_ZZ_p
        NTL::vec_ZZ_p read_json_to_vector(const std::string& filename, const std::string& vector_name);

        // Decode the access key 
        // uses decoding algorithm from the paper: https://eprint.iacr.org/2021/1397.pdf page 10 (section: plaintext encoding)
        void decodeAccessKey(const int& l, const int& m, const int& q, const std::string& json_data_path, const std::string& json_export_name);

        // Retrieve the decoded message parts
        std::vector<long> getDecodedMessageParts();

    private:
        NTL::mat_ZZ_p s_i; // secret key
        NTL::mat_ZZ_p b_i; // public key of this server 
        NTL::mat_ZZ_p e_i; // random vector to be stored

        std::vector<long> decodedMessageParts; // placeholder for the decoded message parts of the user
        
        int smallness_e;
        int smallness_s;

        // helper functions
        // Divides a vecror into n parts and returns the nth part
        NTL::vec_ZZ_p splitAndRetrieveNthPart(const NTL::vec_ZZ_p& originalVector, const int& n, const int& ServerID);
};

#endif /* PVSS_SERVER_H */