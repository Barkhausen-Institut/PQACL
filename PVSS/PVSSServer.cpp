#include "PVSSServer.h"


PVSSServer::PVSSServer(const int& mod_q, const int& m, const int& l, const int& k, const NTL::mat_ZZ_p& A, const int& smallness_e, const int& smallness_s){
    // set modulus for finite field
    NTL::ZZ_p::init(NTL::ZZ(mod_q));

    this->smallness_e = smallness_e;
    this->smallness_s = smallness_s;

    // generate random vector e_i and s_i
    this->e_i.SetDims(m*l, k);
    this->s_i.SetDims(m*l, k);

    // Set the seed for the random number generator
    std::srand(std::time(0));

    // Generate random matrices e_i and s_i with smallness parameter
    for(int row = 0; row < m*l; row++){
        for(int col = 0; col < k; col++){
            this->e_i[row][col] = std::rand() % (smallness_e) + 1;
            this->s_i[row][col] = std::rand() % (smallness_s) + 1;
        }
    }

    // calculate b_i
    this->b_i =  s_i * A + e_i;
}

// Function to calculate the modular inverse of a modulo x
long moduloDivision(long a, long b, long x) {
    // Calculate the modular inverse of b modulo x
    long inverse = 0;
    for (long i = 1; i < x; i++) {
        if ((b * i) % x == 1) {
            inverse = i;
            break;
        }
    }
    // Perform division modulo x
    long result = (a * inverse) % x;
    return result;
}

// Function to decode the message
long decodeMessage(const std::vector<long>& x, long& q,const int& l, long& delta) {
    // Compute the yi terms
    std::vector<long> y;
    for (int i = 0; i < l - 1; i++) {
        y.push_back(x[i] - (delta * x[i+1]) % q);
    }
    
    // Compute the z term
    long z = 0;
    for (int i = 0; i < l - 1; i++) {
        z += std::pow(delta, i) * y[i];
    }

    // Compute the error e_1
    int deltaPowLMinus1 = static_cast<int>(std::pow(delta, l - 1));
    long e_1 = (z % deltaPowLMinus1);
    if (e_1 < 0) {
        e_1 += deltaPowLMinus1;
    }

    // Compute the plaintext
    long plaintext = moduloDivision((x[0] - e_1), std::pow(delta, l-1), q);

    return plaintext;
}

// Decode the access keys for the server
void PVSSServer::decodeAccessKey(const int& l, const int& m, const int& q, const int& n, const int& ServerID, const std::string& json_data_path){
    
    long this_q = q;

    // Read the data from the json file
    NTL::vec_ZZ_p c_1 = read_json_to_vector(json_data_path, "c_1");
    NTL::vec_ZZ_p c_2 = read_json_to_vector(json_data_path, "c_2");   

    // Split c_2 into the seperate messages for each server and take the nth part 
    NTL::vec_ZZ_p d_i = splitAndRetrieveNthPart(c_2, n, ServerID-1);
    
    // Calculate x_i
    NTL::vec_ZZ_p x_i;
    x_i.SetLength(l);
    x_i = d_i - this->s_i * c_1;
    
    std::vector<NTL::vec_ZZ_p> messageParts;
    for (int i = 0; i<m; i++){
        messageParts.push_back(splitAndRetrieveNthPart(x_i, m, i));
    }
    
    // Calculate delta
    static long delta = std::floor(std::pow(this_q, 1.0/l));

    // Decode the message parts
    for (int i = 0; i < m; i++) {
        this->decodedMessageParts.push_back(decodeMessage(convert_ZZ_p_VectorTo_long_vector(messageParts[i]), this_q, l, delta));
    }

}

// simple getter function for the decoded message parts
std::vector<long> PVSSServer::getDecodedMessageParts(){
    return this->decodedMessageParts;
}

// Function to split an ntl::vec_ZZ_p into n parts and retrieve the nth part
NTL::vec_ZZ_p PVSSServer::splitAndRetrieveNthPart(const NTL::vec_ZZ_p& originalVector, const int& n, const int& ServerID) {
    int totalSize = originalVector.length();
    int partSize = totalSize / n;
    int startIndex = ServerID;

    NTL::vec_ZZ_p nthPart;
    nthPart.SetLength(partSize);

    for (int i = 0; i < partSize; ++i) {
        nthPart[i] = originalVector[startIndex*partSize  + i];
    }

    return nthPart;
}

void PVSSServer::export_to_json(const std::string& filename, const NTL::mat_ZZ_p& A){
    nlohmann::json j;

    // Convert and add matrices
    j["A"] = convert_ZZ_p_MatrixTo_long_mat(A);
    j["b_i"] = convert_ZZ_p_MatrixTo_long_mat(this->b_i);
    j["s_i"] = convert_ZZ_p_MatrixTo_long_mat(this->s_i);
    j["e_i"] = convert_ZZ_p_MatrixTo_long_mat(this->e_i);
    j["e_small"] = this->smallness_e;
    j["s_small"] = this->smallness_s;

    // Write to file
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4); // 4 spaces as indentation
        file.close();
    } else {
        std::cerr << "Unable to open file " << filename << std::endl;
    }
}

void PVSSServer::proof_public_key(const int& m, const int& l, const int& k, const std::string& json_data, const std::string& proof_export_path){
    // run the risc0 prover for public key with the given parameters
    std::string runCommand = "./riscZeroProver/dataServersPublicKey/prover " + std::to_string(m) + " " + std::to_string(l) + " " + 
        std::to_string(k) + " " + json_data + " " + proof_export_path;
    int exitCode = execute_command_on_bash(runCommand);
    if(exitCode != 0){
        std::cout << "Error while proving" << std::endl;
    }
}

bool PVSSServer::verify_access_proof(const std::string& proof_path, const std::string& json_data_export_path){
    // run the risc0 verifier for the proof with the given parameters
    std::string runCommand = "./riscZeroProver/accessControlUser/verifier " + proof_path + " " + json_data_export_path;
    int exitCode = execute_command_on_bash(runCommand);
    if(exitCode != 0){
        std::cout << "Error while verifying" << std::endl;
        return false;
    }
    return true;
}

NTL::vec_ZZ_p PVSSServer::read_json_to_vector(const std::string& filename, const std::string& vector_name) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open the JSON file: " << filename << std::endl;
        return NTL::vec_ZZ_p();
    }

    nlohmann::json j;
    file >> j;

    if (!j[vector_name].is_array()) {
        std::cerr << "Invalid JSON format. Expected an array." << std::endl;
        return NTL::vec_ZZ_p();
    }

    std::vector<long> vectorData;
    for (const auto& element : j[vector_name]) {
        vectorData.push_back(element);
    }

    long numRows = vectorData.size();

    NTL::vec_ZZ_p vector;
    vector.SetLength(numRows);
    for (long i = 0; i < numRows; ++i) {
        vector[i] = NTL::to_ZZ_p(vectorData[i]);
    }

    return vector;
}
