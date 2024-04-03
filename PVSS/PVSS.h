#ifndef PVSS_H
#define PVSS_H

#include <NTL/mat_ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <nlohmann/json.hpp>
#include "helperFunctions.h"

/*
This PVSS class represents the GHL+ scheme of our paper. 
It is an implementation of a LWE encryption described in the PVSS_GHL+_scheme.pdf file. 
It implements the calculation for the user/client side.
*/
class PVSS{
    public:
        // Initialize the values with the given number space Z/Z_q and the given dimensions col_k and row_n
        // m is the secret to be encoded and encrypted
        // n is the number of servers
        // k is a LWE security parameter
        // l is a encoding reduncancy parameter
        // q is the modulus
        // smallness_e/s are the maximum smallness values for the error
        // A is the public matrix from the master server
        PVSS(const int &mod_q, const int& m, const int& n, const int& l, 
            const int& k, const NTL::mat_ZZ_p &A, 
            const int& smallness_e, const int& smallness_s);

        // This function takes the b_i vectors from the data servers and combines them into the B matrix 
        void combineBFromProofs(const std::vector<std::string>& exported_values_json_paths);

        // Verify the zero knowledge proof from the server
        bool verify_proof_from_server(const std::string& proof_path, const std::string& json_data_export_path);

        // Calculate the c_1 and c_2 vectors for each server and generate the proof
        void calculateC1AndC2();

        // Save the vectors and matrixes to a json file
        // this is neccessary to send the data to the risc0 framework
        void export_to_json(const std::string& filename);

        // Read a matrix from a json file to a NTL::mat_ZZ_p
        NTL::mat_ZZ_p read_json_to_matrix(const std::string& filename, const std::string& matrix_name);

        // Prepare zero knowledge proof for the servers 1 and 2
        void proveC1_C2(const int& m, const int& n, const int& l, const int& k, const std::string& data_for_proof, const std::string& proof_path);

        // set the private key
        void setPrivateKey(const NTL::vec_ZZ_p& privateKey);

    private:
        NTL::mat_ZZ_p A;    // received but for now random          // size (k,k)
        NTL::mat_ZZ_p B;    // received from servers and combined   // size (mnl,k)
        NTL::vec_ZZ_p r;    // private random vector                // size k
        NTL::vec_ZZ_p privateKey;   // given by PACL                 // size mn
        NTL::vec_ZZ_p encodedPrivateKey; // encoded private key     // size mnl
        NTL::vec_ZZ_p e_1;  // private random vector                // size k
        NTL::vec_ZZ_p e_2;  // private random vector                // size mnl
        NTL::vec_ZZ_p c_1;  // calculated                           // size k
        NTL::vec_ZZ_p c_2;  // calculated                           // size mnl
        int n;
        int l;
        int q;
        int s_small;
        int e_small;

        // internal helper functions
        // Encodes the private key with the encoding scheme from this paper
        // https://eprint.iacr.org/2021/1397.pdf page 10 (section: plaintext encoding)
        void encodePrivateKey();

};

#endif /* PVSS_H */