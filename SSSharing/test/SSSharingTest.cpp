/*
Old implementation

#include <iostream>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>


int main(){
    // set modulus for finite field
    int mod_q = 15* std::pow(2,27) +1; 
    // Set the total number of shares
    static int n = 3;
    int secret_int = 2323984798;

    // Initialize the finite field
    NTL::ZZ_p::init(NTL::ZZ(mod_q));

    // Set the secret value
    NTL::ZZ_p secret;
    secret = NTL::to_ZZ_p(secret_int);

    // Generate the random polynom of degree n-1 = 2
    NTL::ZZ_pX poly;

    // Generate random coefficients
    NTL::vec_ZZ_p a;
    a.SetLength(n-1);
    for(int i = 0; i < n-1; i++){
        a[i] = NTL::random_ZZ_p();
    }

    // Set the coefficients of the polynomial
    NTL::SetCoeff(poly, 0, secret);
    for(int i = 0; i < n-1; i++){
        NTL::SetCoeff(poly, i+1, a[i]);
    }

    // Print the polynomial at x = 0, should be secret
    std::cout << NTL::eval(poly, NTL::to_ZZ_p(0)) << std::endl;
    std::cout << "Polynomial: " << poly << std::endl;

    // Generate the shares
    NTL::vec_ZZ_p shares;
    shares.SetLength(n);
    
    // Set random x values for the share pairs
    NTL::vec_ZZ_p x;
    x.SetLength(n);
    for(int i = 0; i < n; i++){
        x[i] = NTL::random_ZZ_p();
    }

    // Generate the shares by evaluating the polynomial at the x values
    for(int i = 0; i < n; i++){
        shares[i] = NTL::eval(poly, x[i]);
    }

    // Print the shares
    for(int i = 0; i < n; i++){
        std::cout << "Share " << i << ": " << shares[i] << std::endl;
    }

    // Convert the shares and x values to constant vectors
    const NTL::vec_ZZ_p xs = x;
    const NTL::vec_ZZ_p ys = shares;


    // Combine the shares to recover the secret
    NTL::ZZ_pX lagrange;
    NTL::interpolate(lagrange, xs, ys);
    NTL::ZZ_p recoveredSecret = NTL::coeff(lagrange, 0);
    
    // Print the original secret and the recovered secret
    std::cout << "Original Secret: " << secret << std::endl;
    std::cout << "Recovered Secret: " << recoveredSecret << std::endl;

    return 0;
}

*/
