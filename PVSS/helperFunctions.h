#ifndef HELPER_FUNCTIONS
#define HELPER_FUNCTIONS

#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>


// Converts std::vector<uint8_t> to JSON
inline nlohmann::json vectorToJson_uint(const std::vector<uint8_t>& vec) {
    // Directly convert the vector to JSON
    nlohmann::json j;
    j["vec"] = vec;
    return j;
}

// Converts JSON back to std::vector<uint8_t>
inline std::vector<uint8_t> jsonToVector_uint(const nlohmann::json& j) {
    std::vector<uint8_t> vec = j["vec"].get<std::vector<uint8_t>>();
    return vec;
}

// Converts NTL::vec_ZZ_p to JSON
inline nlohmann::json vector_ZZ_p_ToJson(const NTL::vec_ZZ_p& vec) {
    nlohmann::json j;
    std::vector<long> vec_std;
    for (long i = 0; i < vec.length(); ++i) {
        vec_std.push_back(NTL::conv<long>(vec[i]));
    }
    j["vec"] = vec_std;
    return j;
}


// Converts NTL::vec_ZZ_p to JSON
inline nlohmann::json vector_of_vec_ZZ_p_ToJson(const std::vector<NTL::vec_ZZ_p>& vecs) {
    nlohmann::json j = nlohmann::json::array();
    
    for (const auto& vec : vecs) {
        std::vector<long> vec_std;
        for (long i = 0; i < vec.length(); ++i) {
            vec_std.push_back(NTL::conv<long>(vec[i]));
        }
        j.push_back(vec_std);
    }
    
    return j;
}



// Converts JSON back to NTL::vec_ZZ_p
inline std::vector<NTL::vec_ZZ_p> jsonToVector_vec_ZZ_p(const nlohmann::json& j) {
    std::vector<NTL::vec_ZZ_p> vecs;

    if (!j.is_array()) {
        throw std::invalid_argument("Das übergebene JSON muss ein Array sein.");
    }

    for (const auto& jVec : j) {
        NTL::vec_ZZ_p vec;
        vec.SetLength(jVec.size());
        for (size_t i = 0; i < jVec.size(); ++i) {
            vec[i] = NTL::conv<NTL::ZZ_p>(jVec[i].get<long>());
        }
        vecs.push_back(vec);
    }

    return vecs;
}

// Converts NTL::mat_ZZ_p to matrix of long vectors
inline std::vector<std::vector<long>> convert_ZZ_p_MatrixTo_long_mat(const NTL::mat_ZZ_p& m) {
    std::vector<std::vector<long>> stdMatrix;
    for (long i = 0; i < m.NumRows(); ++i) {
        std::vector<long> row;
        for (long j = 0; j < m.NumCols(); ++j) {
            row.push_back(NTL::conv<long>(m[i][j]));
        }
        stdMatrix.push_back(row);
    }
    return stdMatrix;
}

// Converts vector NTL::vec_ZZ_p to vector of long
inline std::vector<long> convert_ZZ_p_VectorTo_long_vector(const NTL::vec_ZZ_p& v){
    std::vector<long> stdV;
    for (long i = 0; i < v.length(); ++i) {
        stdV.push_back(NTL::conv<long>(v[i]));
    }
    return stdV;
}

// executes a command on the bash
inline int execute_command_on_bash(const std::string& command) {
    return std::system(command.c_str());
}

#endif  // HELPER_FUNCTIONS