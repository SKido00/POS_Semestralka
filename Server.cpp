#include "Server.h"

std::mutex poleMutex;
std::vector<std::pair<int, int>> suradniceCiernychBuniek;

// Funkcia na ukladanie ciernych buniek do suboru na servery
void ulozSuradniceDoSuboru(const char* subor) {
    std::ofstream file(subor);
    if (!file.is_open()) {
        std::cerr << "Error pri otvarani suboru na ulozenie\n";
        return;
    }

    std::lock_guard<std::mutex> lock(poleMutex);
    for (const auto& coord : suradniceCiernychBuniek) {
        file << coord.first << ' ' << coord.second << std::endl;
    }

    file.close();
}

// Funkcia na spracovanie requestov od uzivatelov
void handleClient(int clientSocket) {
    char buffer[1024];

    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        std::cerr << "Error pri ziskavani dat od klienta\n";
        close(clientSocket);
        return;
    }

    std::string request(buffer, bytesRead);

    if (request.find("SAVE") == 0) {
        // Ukladanie suradnic ciernych buniek
        std::lock_guard<std::mutex> lock(poleMutex);
        std::istringstream iss(request.substr(5));
        int x, y;
        while (iss >> x >> y) {
            suradniceCiernychBuniek.emplace_back(x, y);
        }
        std::cout << "Ziskane a ulozene suradnice od klienta\n";
    } else if (request == "DOWNLOAD") {
        // Posielanie suradnic ciernych buniek klientovi
        std::lock_guard<std::mutex> lock(poleMutex);
        std::ostringstream oss;
        for (const auto& coord : suradniceCiernychBuniek) {
            oss << coord.first << ' ' << coord.second << ' ';
        }
        std::string response = oss.str();
        ssize_t bytesSent = send(clientSocket, response.c_str(), response.size(), 0);
        if (bytesSent == -1) {
            std::cerr << "Error pri posielani suradnic klientovi\n";
        } else {
            std::cout << "Suradnice boli poslane klientovi\n";
        }
    }

    close(clientSocket);
}

int main() {

    int serverPort = 12345;

    // Vytvaranie socketu
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error pri vytvarani socketu\n";
        exit(EXIT_FAILURE);
    }

    // Pripravovanie serverovej struktury
    sockaddr_in serverAddressStruct;
    serverAddressStruct.sin_family = AF_INET;
    serverAddressStruct.sin_addr.s_addr = INADDR_ANY;
    serverAddressStruct.sin_port = htons(serverPort);

    // Nastavenie socketu
    if (bind(serverSocket, (struct sockaddr*)&serverAddressStruct, sizeof(serverAddressStruct)) == -1) {
        std::cerr << "Error pri nastavovani socketu\n";
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Cakanie na prichadzajuce pripojenia
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error pri pocuvani prichadzajucich pripojeni\n";
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server pocuva na porte " << serverPort << std::endl;

    while (true) {
        // Prijmanie prichadzajuceho pripojenia
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            std::cerr << "Error pri prijamni pripojenia klienta\n";
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        // Spracovanie klienta vo vedlajsom vlakne
        std::thread(handleClient, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}
