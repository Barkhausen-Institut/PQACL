#ifndef DataServerProtocol_h
#define DataServerProtocol_h

#include <vector>
#include <NTL/vec_ZZ_p.h>
#include <NTL/mat_ZZ_p.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <chrono>

#include "../PVSS/helperFunctions.h"
#include "../DPF/dpf.h"
#include "../SSSharing/SSSharing.h"

/*
This class implements the calculations for the PACL protocol on the data server side.
It can be seen as the outer LWS protocol, which is used to calculate the Tau's for the master server to decide if a user is allowed to access a file.
For realising that the class uses the DPF and SSSharing protocols.
*/
class DataServerProtocol{
    private:
        // DPF parameters
        std::vector<uint8_t> dpf_key;
        std::vector<uint8_t> dpf_values_char;
        std::vector<int> dpf_values;
        size_t dpf_key_size_logn;
        // SSSharing parameters
        // Vector of all Xshares values from all servers
        std::vector<NTL::vec_ZZ_p> Xshares;
        // Yshares for this specific server
        NTL::vec_ZZ_p Yshares;

        // outer protocol parameters
        int numberOfFiles;
        int secretSizeOfFiles;
        int mod_q;
        int serverNumber;

        std::vector<NTL::vec_ZZ_p> Asi;
        NTL::mat_ZZ_p A;
        NTL::vec_ZZ_p B;
        NTL::vec_ZZ_p TAU;


        

    public:
        // Save the publicly know parameters in the class
        DataServerProtocol(NTL::mat_ZZ_p& A, std::vector<NTL::vec_ZZ_p>& Asi, const int& numberOfFiles, const int& secretSizeOfFiles, const int& mod_q, const int& serverNumber){
            this->Asi = Asi;
            this->A = A;
            this->numberOfFiles = numberOfFiles;
            this->secretSizeOfFiles = secretSizeOfFiles;
            this->mod_q = mod_q;
            this->serverNumber = serverNumber;
            this->B.SetLength(secretSizeOfFiles);
        };
        // Set the DPF key
        void setDPFKey(const std::vector<uint8_t>& key, size_t& key_size_logn);

        // Set the SSSharing keys
        void setSSSharingKeyX(std::vector<NTL::vec_ZZ_p> Xshares);
        void setSSSharingKeyY(std::vector<long> Yshares);

        // calculate Tau's by evaluating the DPF and using the SSSharing keys
        NTL::vec_ZZ_p calculateTau();

};
#endif /* DataServerProtocol_h */