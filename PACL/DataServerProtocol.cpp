#include "DataServerProtocol.h"


void DataServerProtocol::setDPFKey(const std::vector<uint8_t>& key, size_t& key_size_logn){
    // Save the DPF key and key size in class
    this->dpf_key = key;
    this->dpf_key_size_logn = key_size_logn;
}

void DataServerProtocol::setSSSharingKeyX(std::vector<NTL::vec_ZZ_p> Xshares){
    // Save the X shares in class
    this->Xshares = Xshares;
}
void DataServerProtocol::setSSSharingKeyY(std::vector<long> Yshares){
    // Save the Y shares in class after converting them to NTL::vec_ZZ_p
    this->Yshares.SetLength(Yshares.size()); // Setze die Länge des NTL::vec_ZZ_p

    for (size_t i = 0; i < Yshares.size(); ++i) {
        this->Yshares[i] = NTL::conv<NTL::ZZ_p>(Yshares[i]); // Konvertiere jedes Element und weise es zu
    }
}

NTL::vec_ZZ_p DataServerProtocol::calculateTau(){
    // Evaluate the DPF and save the values
    this->dpf_values_char = DPF::EvalFull(dpf_key, dpf_key_size_logn);

    // Convert the dpf_values_char to dpf_values
    for (size_t i = 0; i < dpf_values_char.size(); ++i) {
        this->dpf_values.push_back((int)dpf_values_char[i]);
    }
    
    // Calculate B = ADD(ASi * dpf_values[i]) for all i in [0, numberOfFiles]
    for(int i = 0; i < this->numberOfFiles; i++){
        // this->B += this->Asi[i] * dpf_values[i];
        // improve performance by using NTL::mul and NTL::add 
        NTL::mul(this->Asi[i], this->Asi[i], this->dpf_values[i]);
        NTL::add(this->B, this->B, this->Asi[i]);
    }

    // Negate the B if the server number is even
    if(this->serverNumber % 2 == 0){
        // this->B = -this->B;
        // improve performance by using NTL::negate
        NTL::negate(this->B, this->B);
    }
    // Calculate partial SSShrare recovery sj = y[i] * MUL(x[m]/[x[m]-x[i]]) for all i != m in [0, secretSizeOfFiles]
    NTL::vec_ZZ_p sj = SSSharing::reconstructPartialSecret(this->Xshares, this->Yshares, this->numberOfFiles, this->secretSizeOfFiles, this->mod_q, this->serverNumber);

    // Calculate Tau = B + A*sj
    this->TAU = this->B - this->A * sj;

    return this->TAU;
}