#ifndef LANGTON_H
#define LANGTON_H

#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <random>

enum class StavBunky {
    BIELA,
    CIERNA
};

enum class SmerMravca {
    HORE,
    VPRAVO,
    DOLE,
    VLAVO
};

enum class Kolizia {
    ZANIK,
    PREZITIE,
    ZMENA_LOGIKY
};

struct Mravec {
    int x, y;
    SmerMravca smer;
    bool inverzna;
};

extern std::vector<std::vector<StavBunky>> pole;
extern std::vector<Mravec> mravce;
extern std::mutex poleMutex;

extern int VELKOST_POLA; //rozmer poľa
extern int POCET_MRAVCOV; // počet mravcov
extern Kolizia MOD_KOLIZIE; // mód (ako sa mravce zachovajú pri stretnutí na jednom poli)
extern bool simulaciaBezi;
extern bool skonciProgram;
extern bool paused;

void vykresliPole();
void prepisPole();
void nahodneNastavenieCiernychBuniek(int pocetCiernych, std::vector<std::vector<StavBunky>> &plocha);
void manualneNastavenieCiernychBuniek();
void nastavenieCiernychBuniekZoSuboru(const std::string &nazovSuboru);
void ulozeniePolaDoSuboru(const std::string &nazovSuboru);
void nahodneNastaveniePozicieMravcov();
void manualneNastaveniePozicieMravcov();
void handleCollision(Kolizia Kolizia);
void uzivatelskyVstup();
void simulacia();

#endif // LANGTON_H
