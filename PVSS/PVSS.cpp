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
    this->e_2.SetLength(m*n*l);

    // Set the seed for the random number generator
    std::srand(std::time(0));

    // Generate random vectors r, e_1 and e_2 with smallness parameter
    for(int elem = 0; elem < k; elem++){
        this->r[elem] = std::rand() % (smallness_s + 1);
        this->e_1[elem] = std::rand() % (smallness_e + 1);
    }
    for(int elem = 0; elem < n*m*l; elem++){
        this->e_2[elem] = std::rand() % (smallness_e + 1);
    }
}

void PVSS::encodePrivateKey(){
    int delta = std::floor(std::pow(this->q, 1.0 / this->l));
    int t = this->privateKey.length();

    this->encodedPrivateKey.SetLength(t * this->l);

    for (int i = 0; i < t; ++i) {
        NTL::ZZ_p x_i = this->privateKey[i];
        for (int j = l-1; j >= 0; --j) {
            this->encodedPrivateKey[i * l + j] = x_i;
            x_i *= delta;
        }
    }
}

void PVSS::combineBFromProofs(const std::vector<std::string>& exported_values_json_paths){
    // Get B from servers
    std::vector<NTL::mat_ZZ_p> B_is;
    for(int i = 0; i< this->n; i++){
        // Convert and add matrices
        NTL::mat_ZZ_p B_i = read_json_to_matrix(exported_values_json_paths[i], "b_i");
        B_is.push_back(B_i);
    }
    size_t rows = B_is[0].NumRows();
    size_t cols = B_is[0].NumCols();

    // Calculate the total number of rows and columns in the combined matrix
    for (size_t i = 1; i < B_is.size(); i++) {
        rows += B_is[i].NumRows();
    }

    // Create the combined matrix with the calculated dimensions
    this->B.SetDims(rows, cols);

    size_t currentRow = 0;

    // Copy the matrices into the combined matrix
    for (const NTL::mat_ZZ_p& matrix : B_is) {
        for (long int i = 0; i < matrix.NumRows(); i++) {
            for (long int j = 0; j < matrix.NumCols(); j++) {
                this->B[currentRow + i][j] = matrix[i][j];
            }
        }
        currentRow += matrix.NumRows();
    }
}

void PVSS::calculateC1AndC2(){
    // Calculate c_1 and c_2
    this->c_1 = this->A * this->r + this->e_1;
    this->c_2 = this->B * this->r + this->e_2 + this->encodedPrivateKey;
    // std::cout << "c_1: " << this->c_1 << " c_2: " << this->c_2 << std::endl;
}

void PVSS::export_to_json(const std::string& filename){
    nlohmann::json j;

    // Convert and add matrices to json 
    j["A"] = convert_ZZ_p_MatrixTo_long_mat(this->A);
    j["B"] = convert_ZZ_p_MatrixTo_long_mat(this->B);
    j["r"] = convert_ZZ_p_VectorTo_long_vector(this->r);
    j["e_1"] = convert_ZZ_p_VectorTo_long_vector(this->e_1);
    j["e_2"] = convert_ZZ_p_VectorTo_long_vector(this->e_2);
    j["priv_key"] = convert_ZZ_p_VectorTo_long_vector(this->encodedPrivateKey);
    j["c_1"] = convert_ZZ_p_VectorTo_long_vector(this->c_1);
    j["c_2"] = convert_ZZ_p_VectorTo_long_vector(this->c_2);
    j["s_small"] = this->s_small;
    j["e_small"] = this->e_small;
    
    // For debugging
    j["priv_key_input"] = convert_ZZ_p_VectorTo_long_vector(this->privateKey);

    // Write to file
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4); // 4 = human friendly readable
        file.close();
    } else {
        std::cerr << "Unable to open file " << filename << std::endl;
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


bool PVSS::verify_proof_from_server(const std::string& proof_path, const std::string& json_data_export_path){
    std::string runCommand = "./riscZeroProver/dataServersPublicKey/verifier " + proof_path + " " + json_data_export_path;
    int exitCode = execute_command_on_bash(runCommand);
    if(exitCode != 0){
        return false;
    }
    return true;
}

void PVSS::proveC1_C2(const int& m, const int& n, const int& l, const int& k, const std::string& data_for_proof, const std::string& proof_path){
    // run prover for c1 and c2 with the given parameters
    std::string runCommand = "./riscZeroProver/accessControlUser/prover " + std::to_string(n) + " " + std::to_string(m) +  " " + std::to_string(l) + " " + 
        std::to_string(k) + " " + data_for_proof +" " + proof_path;
    execute_command_on_bash(runCommand);
}

void PVSS::setPrivateKey(const NTL::vec_ZZ_p& privateKey){
    this->privateKey = privateKey;
    this->encodePrivateKey();
}
