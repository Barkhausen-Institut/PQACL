
/*


#include <NTL/ZZ_pX.h>
#include "SSSharing.h"

SSSharing::SSSharing(const int& numberOfShares, NTL::vec_ZZ_p secret, const int& secretSizeOfFiles,const int &mod_q){
    NTL::ZZ_p::init(NTL::ZZ(mod_q));
    this->numberOfShares = numberOfShares;
    this->secretSizeOfFiles = secretSizeOfFiles;
    this->secret = secret;
    calculateShares();
}

void SSSharing::calculateShares(){

    // Generate the shares and polinomial functions for each part m of the secret vector
    for(int i = 0; i < this->secretSizeOfFiles; i++){
        
        // Set the coefficients of all polynomial functions 
        NTL::ZZ_pX poly;
        // Generate random coefficients
        NTL::vec_ZZ_p a;
        a.SetLength(this->numberOfShares-1);
        NTL::SetCoeff(poly, 0, this->secret[i]);
        for(int j = 0; j < this->numberOfShares-1; j++){
            a[i] = NTL::random_ZZ_p();
            NTL::SetCoeff(poly, j+1, a[j]);
        }
        this->poly.append(poly);

        // Generate the shares
        NTL::vec_ZZ_p shares;
        shares.SetLength(this->numberOfShares);

        // Set random x values for the share pairs
        NTL::vec_ZZ_p x;
        x.SetLength(this->numberOfShares);
        for(int j = 0; j < this->numberOfShares; j++){
            x[j] = NTL::random_ZZ_p();
        }


        // Generate the shares by evaluating the polynomial at the x values
        for(int j = 0; j < this->numberOfShares; j++){
            shares[j] = NTL::eval(poly, x[j]);
        }

        std::tuple<NTL::vec_ZZ_p, NTL::vec_ZZ_p> sharePoint = std::make_tuple(x, shares);
        this->sharePoints.push_back(sharePoint);


    }


}

void SSSharing::printAndVeryfyShares(){
    // Reconstruction of the secret
    for(int i = 0; i < this->secretSizeOfFiles; i++){
        // Combine the shares to recover the secret
        NTL::ZZ_pX lagrange;
        NTL::interpolate(lagrange, std::get<0>(this->sharePoints[i]), std::get<1>(this->sharePoints[i]));
        NTL::ZZ_p recoveredSecret = NTL::coeff(lagrange, 0);
        
        // Print the original secret and the recovered secret
        std::cout << "Original Secret: " << this->secret[i] << std::endl;
        std::cout << "Recovered Secret: " << recoveredSecret << std::endl;
    }

}

std::vector<NTL::vec_ZZ_p> SSSharing::getShares(){
    // sharePoints = [ ([x1, x2], [y1, y2]), ([x1, x2], [y1, y2]), ([x1, x2], [y1, y2]) ]
    std::vector<NTL::vec_ZZ_p> shares;

    NTL::vec_ZZ_p x1sx2s, y1y2s;


    // print the sharePoints
    // for(auto& share : this->sharePoints){
    //     auto& [xVec, yVec] = share;
    //     std::cout << "x1: " << xVec[0] << " x2: " << xVec[1] << std::endl;
    //     std::cout << "y1: " << yVec[0] << " y2: " << yVec[1] << std::endl;
    // }

    for (int i = 0; i < this->secretSizeOfFiles; i++) {
        auto& [xVec, yVec] = this->sharePoints[i];

        // Append x1 and x2
        x1sx2s.append(xVec[0]);
        x1sx2s.append(xVec[1]);

        // Combine y1 and y2
        y1y2s.append(yVec[0]);
        y1y2s.append(yVec[1]);
    }

    shares.push_back(x1sx2s);
    shares.push_back(y1y2s);

    return shares;
}

*/
