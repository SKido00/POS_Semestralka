#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <mutex>
#include <thread>

extern std::mutex poleMutex;
extern std::vector<std::pair<int, int>> suradniceCiernychBuniek;

// Funkcia na ukladanie suradnic ciernych buniek do suboru
void ulozSuradniceDoSuboru(const char* subor);

// Funkcia na spracovanie klientov
void handleClient(int clientSocket);

#endif // SERVER_H
