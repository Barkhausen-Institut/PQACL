#include "SSSharing.h"


int main() {
    const int numberOfShares = 2; // Anzahl der Shares
    const int secretSizeOfFiles = 5; // Größe des Geheimnisses (Anzahl der Elemente)
    const int mod_q = 4999; // Ein einfacher Modulus für das Feld
    NTL::vec_ZZ_p secret; // Das Geheimnis


    // Initialisierung des Geheimnisses
    NTL::ZZ_p::init(NTL::ZZ(mod_q)); // Initialisiere das Feld
    secret.SetLength(secretSizeOfFiles);
    for (int i = 0; i < secretSizeOfFiles; i++) {
        secret[i] = NTL::random_ZZ_p(); // Setze das Geheimnis
        std::cout << "Secret " << i << ": " << secret[i] << std::endl; // Drucke das Geheimnis
    }

    // Generiere Shares
    auto shares = SSSharing::generateShares(numberOfShares, secret, secretSizeOfFiles, mod_q);

    // Verifiziere und drucke Shares
    SSSharing::printAndVeryfyShares(shares, numberOfShares, secretSizeOfFiles, mod_q);

    // Rekonstruiere das Geheimnis teilweise von einem Server
    int serverNumber = 0; // Server 0 oder 1
    auto partialSecret0 = SSSharing::reconstructPartialSecret(std::get<0>(shares), std::get<1>(shares), numberOfShares, secretSizeOfFiles, mod_q, serverNumber);
    serverNumber = 1; // Server 0 oder 1
    auto partialSecret1 = SSSharing::reconstructPartialSecret(std::get<0>(shares), std::get<2>(shares), numberOfShares, secretSizeOfFiles, mod_q, serverNumber);

    // Rekonstruiere das Geheimnis
    auto reconstructedSecret = partialSecret0 + partialSecret1;
    
    // Drucke das rekonstruierte Geheimnis
    std::cout << "Reconstructed secret: " << reconstructedSecret << std::endl;

    return 0;
}
