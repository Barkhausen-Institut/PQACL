#include "dpf.h"
#include <cmath>

#include <iostream>


int main(){
    int index = 4000;          // This is the index of the file the user wants to access
    size_t N = 30;         // This the maximum number of files (2^(N-3) = number of files)


    std::cout << "N: " << N << std::endl;
    auto keys = DPF::Gen(index * 8, N);     // Generate the DPF keys with alpha = index * (8 bits) and N files
    auto a = keys.first;
    auto b = keys.second;
    
    std::cout << "Key size: " << a.size() << " bytes" << std::endl;
    std::cout << "N: " << N << std::endl;
    std::cout << "a: ";
    for(auto x : a) {
        std::cout << (int)x << " ";
    }
    std::cout << std::endl;
    std::cout << "b: ";
    for(auto x : b) {
        std::cout << (int)x << " ";
    }
    std::cout << std::endl;

    

    std::vector<uint8_t> aaaa;
    if(N > 10) {
        aaaa = DPF::EvalFull8(a, N);
    } else {
        aaaa = DPF::EvalFull(a, N);
    }

    std::cout << "aaaa: ";
    /*
    for(auto x : aaaa) {
        std::cout << (int)x << " ";
    }
    */

    std::cout << std::endl << aaaa.size() << std::endl;
    std::cout << std::endl;

    std::vector<uint8_t> bbbb;
    if(N > 10) {
        bbbb = DPF::EvalFull8(b, N);
    } else {
        bbbb = DPF::EvalFull(b, N);
    }
    std::cout << "bbbb: ";
    /*
    for(auto x : bbbb) {
        std::cout << (int)x << " ";
    }
    */

    std::cout << std::endl << bbbb.size() << std::endl;
    std::cout << std::endl;


    return 0;
}