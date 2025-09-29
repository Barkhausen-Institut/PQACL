#include "PVSS.h"

PVSS::PVSS(const int &mod_q, const int& m, const int& n, const int& l, 
            const int& k, const NTL::mat_ZZ_p &A, 
            const int& smallness_e, const int& smallness_s){

    // Initialize the number space of the NTL library
    NTL::ZZ_p::init(NTL::ZZ(mod_q));

    // Set the given parameters
    this->s_small = smallness_s;
    this->e_small = smallness_e;
    this->A = A;
    this->n = n;
    this->l = l;
    this->q = mod_q;

    // Create random vectors r, e_1, e_2
    this->r.SetLength(k);
    this->e_1.SetLength(k);

    // Set the seed for the random number generator
    std::srand(std::time(0));

    // Generate random vectors r, e_1 and e_2 with smallness parameter
    for(int elem = 0; elem < k; elem++){
        this->r[elem] = std::rand() % (smallness_s + 1);
        this->e_1[elem] = std::rand() % (smallness_e + 1);
    }
    for (int i = 0; i < n; i++) {
        this->e_2_i.push_back(NTL::vec_ZZ_p());
        this->e_2_i[i].SetLength(m * l);
        for (int j = 0; j < m*l; j++) {
            this->e_2_i[i][j] = std::rand() % (smallness_e + 1);
        }
    }
}

void PVSS::encodePrivateKey(){
    for (int idx = 0; idx < this->n; ++idx) {
        NTL::vec_ZZ_p privateKey = this->privateKey_i[idx];
        NTL::vec_ZZ_p encodedPrivateKey_i;
        encodedPrivateKey_i.SetLength(privateKey.length() * this->l);

        int delta = std::floor(std::pow(this->q, 1.0 / this->l));
        int t = privateKey.length();

        for (int i = 0; i < t; ++i) {
            NTL::ZZ_p x_i = privateKey[i];
            for (int j = l-1; j >= 0; --j) {
                encodedPrivateKey_i[i * l + j] = x_i;
                x_i *= delta;
            }
        }
        this->encodedPrivateKey_i.push_back(encodedPrivateKey_i);
    }
}

void PVSS::loadBi(const std::vector<std::string>& json_paths) {
    for(int i = 0; i< this->n; i++){
        // Convert and add matrices
        NTL::mat_ZZ_p B_i = read_json_to_matrix(json_paths[i], "b_i");
        this->B_i.push_back(B_i);
    }
}

void PVSS::calculateC1AndC2(){
    // Calculate c_1 and c_2
    this->c_1 = this->A * this->r + this->e_1;
    for (int i = 0; i < this->n; i++) {
        this->c_2_i.push_back(this->B_i[i] * this->r + this->e_2_i[i] + this->encodedPrivateKey_i[i]);
    }
}

void PVSS::export_to_jsons(const std::vector<std::string>& filenames){
    for (int i = 0; i < this->n; i++) {
        // JSON for prover
        nlohmann::json j;

        // Convert and add matrices to json
        j["A"] = convert_ZZ_p_MatrixTo_long_mat(this->A);
        j["B"] = convert_ZZ_p_MatrixTo_long_mat(this->B_i[i]);
        j["r"] = convert_ZZ_p_VectorTo_long_vector(this->r);
        j["e_1"] = convert_ZZ_p_VectorTo_long_vector(this->e_1);
        j["e_2"] = convert_ZZ_p_VectorTo_long_vector(this->e_2_i[i]);
        j["priv_key"] = convert_ZZ_p_VectorTo_long_vector(this->encodedPrivateKey_i[i]);
        j["c_1"] = convert_ZZ_p_VectorTo_long_vector(this->c_1);
        j["c_2"] = convert_ZZ_p_VectorTo_long_vector(this->c_2_i[i]);

        // Write to file
        std::ofstream file(filenames[i] + "_prover.json");
        if (file.is_open()) {
            file << j.dump(4); // 4 = human friendly readable
            file.close();
        } else {
            std::cerr << "Unable to open file " << filenames[i] + "_prover.json" << std::endl;
        }

        // JSON for verifier
        nlohmann::json j2;

        // Convert and add matrices to json
        j2["c_1"] = convert_ZZ_p_VectorTo_long_vector(this->c_1);
        j2["c_2"] = convert_ZZ_p_VectorTo_long_vector(this->c_2_i[i]);

        // Write to file
        std::ofstream file2(filenames[i] + "_verifier.json");
        if (file2.is_open()) {
            file2 << j2.dump(4); // 4 = human friendly readable
            file2.close();
        } else {
            std::cerr << "Unable to open file " << filenames[i] + "_verifier.json" << std::endl;
        }
    }
}

NTL::mat_ZZ_p PVSS::read_json_to_matrix(const std::string& filename, const std::string& matrix_name) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open the JSON file: " << filename << std::endl;
        return NTL::mat_ZZ_p();
    }

    nlohmann::json j;
    file >> j;

    if (!j[matrix_name].is_array()) {
        std::cerr << "Invalid JSON format. Expected an array." << std::endl;
        return NTL::mat_ZZ_p();
    }

    std::vector<std::vector<long>> matrixData;
    for (const auto& row : j[matrix_name]) {
        if (!row.is_array()) {
            std::cerr << "Invalid JSON format. Expected an array for each row." << std::endl;
            return NTL::mat_ZZ_p();
        }

        std::vector<long> rowData;
        for (const auto& element : row) {
            if (!element.is_number_integer()) {
                std::cerr << "Invalid JSON format. Expected integer values." << std::endl;
                return NTL::mat_ZZ_p();
            }
            rowData.push_back(element.get<long>());
        }
        matrixData.push_back(rowData);
    }

    long numRows = matrixData.size();
    long numCols = (numRows > 0) ? matrixData[0].size() : 0;

    NTL::mat_ZZ_p matrix;
    matrix.SetDims(numRows, numCols);
    for (long i = 0; i < numRows; ++i) {
        for (long j = 0; j < numCols; ++j) {
            matrix[i][j] = NTL::to_ZZ_p(matrixData[i][j]);
        }
    }
    return matrix;
}

void PVSS::proveC1_C2(const int& m, const int& l, const int& k, const std::string& data_for_proof, const std::string& proof_path) {
    std::string runCommand = "./ZKP/zkstark_enc_prover " + std::to_string(m) +  " " + std::to_string(l) + " " +
        std::to_string(k) + " " + data_for_proof + " " + proof_path;
    execute_command_on_bash(runCommand);
}

void PVSS::setPrivateKey(const NTL::vec_ZZ_p& privateKey1, const NTL::vec_ZZ_p& privateKey2){
    this->privateKey_i.push_back(privateKey1);
    this->privateKey_i.push_back(privateKey2);
    this->encodePrivateKey();
}
