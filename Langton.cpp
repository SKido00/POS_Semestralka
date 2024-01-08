#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <random>
#include "Langton.h"

std::vector<std::vector<StavBunky>> pole;
std::vector<Mravec> mravce;
std::mutex poleMutex;

int VELKOST_POLA; //rozmer poľa
int POCET_MRAVCOV; // počet mravcov
Kolizia MOD_KOLIZIE; // mód (ako sa mravce zachovajú pri stretnutí na jednom poli)
bool simulaciaBezi = true;
bool skonciProgram = false;
bool paused = false;

// Vykreslí plochu simulácie, zobrazuje pozície mravcov a stav buniek
void vykresliPole() {
    // Ak je simulácia pozastavená, nevypisujeme mriežku
    if (paused) {
        return;
    }

    // Vypíšeme čísla stĺpcov (používame len čísla 0-9, pretože pri 2-ciferných by nám nesedelo formátovanie)
    std::cout << "  ";
    for (int j = 0; j < VELKOST_POLA; ++j) {
        std::cout << j % 10 << ' ';
    }
    std::cout << std::endl;
    for (int i = 0; i < VELKOST_POLA; ++i) {
        // Vypíšeme číslo riadku
        std::cout << i << ' ';
        for (int j = 0; j < VELKOST_POLA; ++j) {
            std::lock_guard<std::mutex> lock(poleMutex);
            // Nájdeme mravca na danej pozíci
            bool antFound = false;
            for (const auto &mravec: mravce) {
                if (i == mravec.y && j == mravec.x) {
                    char symbol;
                    // Podľa smeru mravca určíme jeho symbol
                    switch (mravec.smer) {
                        case SmerMravca::HORE:
                            symbol = '^';
                            break;
                        case SmerMravca::VPRAVO:
                            symbol = '>';
                            break;
                        case SmerMravca::DOLE:
                            symbol = 'v';
                            break;
                        case SmerMravca::VLAVO:
                            symbol = '<';
                            break;
                    }
                    // Vypíšeme symbol mravca
                    std::cout << symbol << ' ';
                    antFound = true;
                    break;
                }
            }
            // Ak žiadny mravec nie je na danej pozícii, vypíšeme bodku alebo krížik podľa stavu bunky ( . -> Biela, # -> Čierna)
            if (!antFound) {
                std::cout << (pole[i][j] == StavBunky::BIELA ? '.' : '#') << ' ';
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Aktualizuje stav mriežky na základe pohybu mravcov a pravidel pre ich správanie
void prepisPole() {
    for (auto &mravec: mravce) {
        {
            std::lock_guard<std::mutex> lock(poleMutex);

            // Zmeníme stav bunky pod mravcom na opak (čierna na bielu a naopak)
            pole[mravec.y][mravec.x] = (pole[mravec.y][mravec.x] == StavBunky::BIELA) ? StavBunky::CIERNA : StavBunky::BIELA;

            // Posunieme mravca v závislosti od jeho smeru a obnovíme jeho pozíciu v mriežke
            switch (mravec.smer) {
                case SmerMravca::HORE:
                    mravec.y = (mravec.y - 1 + VELKOST_POLA) % VELKOST_POLA;
                    break;
                case SmerMravca::VPRAVO:
                    mravec.x = (mravec.x + 1) % VELKOST_POLA;
                    break;
                case SmerMravca::DOLE:
                    mravec.y = (mravec.y + 1) % VELKOST_POLA;
                    break;
                case SmerMravca::VLAVO:
                    mravec.x = (mravec.x - 1 + VELKOST_POLA) % VELKOST_POLA;
                    break;
            }

            // Podľa farby nového políčka pod mravcom upravíme jeho smer pohybu
            switch (pole[mravec.y][mravec.x]) {
                case StavBunky::BIELA:
                    if (mravec.inverzna) {
                        // Inverzná logika: otočíme mravca o 90° vľavo
                        mravec.smer = static_cast<SmerMravca>((static_cast<int>(mravec.smer) + 1) % 4);
                        break;
                    } else {
                        // Priama logika: otočíme mravca o 90° vpravo
                        mravec.smer = static_cast<SmerMravca>((static_cast<int>(mravec.smer) + 3) % 4);
                        break;
                    }
                case StavBunky::CIERNA:
                    if (mravec.inverzna) {
                        // Inverzná logika: otočíme mravca o 90° vpravo
                        mravec.smer = static_cast<SmerMravca>((static_cast<int>(mravec.smer) + 3) % 4);
                        break;
                    } else {
                        // Priama logika: otočíme mravca o 90° vľavo
                        mravec.smer = static_cast<SmerMravca>((static_cast<int>(mravec.smer) + 1) % 4);
                        break;
                    }

            }
        }
    }
}

// Funkcia na náhodné vygenerovanie čiernych buniek v mriežke.
// Parametre:
//   - pocetCiernych: Počet čiernych buniek, ktoré majú byť náhodne vygenerované.
//   - plocha: Referencia na maticu (pole), do ktorej sa vygenerované bunky pridajú.
void nahodneNastavenieCiernychBuniek(int pocetCiernych, std::vector<std::vector<StavBunky>> &plocha) {
    std::random_device rd;
    std::mt19937 gen(rd());  // Vytvorenie generátora čísel s Mersenne Twister algoritmom

    int velkostPola = plocha.size();  // Získanie veľkosti mriežky

    // Kontrola, aby sme náhodne nevygenerovali viac buniek, ako je veľkosť pola
    pocetCiernych = std::min(pocetCiernych, velkostPola * velkostPola);

    for (int i = 0; i < pocetCiernych; ++i) {
        int x = gen() % velkostPola;
        int y = gen() % velkostPola;
        plocha[y][x] = StavBunky::CIERNA;
    }
}

// Funkcia pre manuálne nastavenie čiernych buniek
void manualneNastavenieCiernychBuniek() {
    // Výzva pre používateľa na zadanie súradníc čiernych buniek
    std::cout << "Zadaj suradnice ciernych buniek podla vzoru 'x y', zadaj -1 pre koniec):" << std::endl;

    int x, y;
    // Nekonečná smyčka na načítanie súradníc, kým používateľ nezadá -1
    while (true) {
        std::cin >> x;
        if (x == -1) break;

        std::cin >> y;
        if (y == -1) break;

        // Kontrola, či súradnice sú v rozsahu mriežky
        if (x >= 0 && x < VELKOST_POLA && y >= 0 && y < VELKOST_POLA) {
            // Nastavenie bunky na čiernu farbu
            pole[y][x] = StavBunky::CIERNA;
        } else {
            // Vypísanie chybovej hlášky pre neplatné súradnice
            std::cout << "Suradnice nie su v rozsahu. Skus to znovu." << std::endl;
        }
    }
}

// Funkcia na nastavenie čiernych buniek podľa súradníc zo súboru
void nastavenieCiernychBuniekZoSuboru(const std::string &nazovSuboru) {
    // Otvorenie súboru pre čítanie
    std::ifstream subor(nazovSuboru);
    if (!subor.is_open()) {
        std::cout << "Nepodarilo sa otvorit subor: " << nazovSuboru << std::endl;
        return;
    }

    int x, y;
    // Čítanie súradníc zo súboru
    while (subor >> x >> y) {
        // Kontrola, či súradnice sú v rozsahu mriežky
        if (x >= 0 && x < VELKOST_POLA && y >= 0 && y < VELKOST_POLA) {
            pole[y][x] = StavBunky::CIERNA;
        } else {
            std::cout << "Neplatne suradnice v subore: " << x << " " << y << std::endl;
        }
    }
    // Zatvorenie súboru po dokončení čítania
    subor.close();
}

// Funkcia na uloženie finálneho stavu mriežky do súboru
void ulozeniePolaDoSuboru(const std::string &nazovSuboru) {
    std::ofstream subor(nazovSuboru);
    if (!subor.is_open()) {
        std::cout << "Chyba otvorenia suboru na ulozenie: " << nazovSuboru << std::endl;
        return;
    }

    for (int i = 0; i < VELKOST_POLA; ++i) {
        for (int j = 0; j < VELKOST_POLA; ++j) {
            // Zabezpečíme, aby manipulácie s mriežkou boli bezpečné pomocou mutexu
            std::lock_guard<std::mutex> lock(poleMutex);
            // Zapíšeme do súboru bodku alebo krížik podľa stavu bunky (. -> Biela, # -> Čierna)
            subor << (pole[i][j] == StavBunky::BIELA ? '.' : '#') << ' ';
        }
        subor << std::endl;
    }
    subor.close();
}

// Funkcia na náhodné určenie počiatočných pozícií mravcov v mriežke.
void nahodneNastaveniePozicieMravcov() {
    std::random_device rd;  // Inicializácia generátora náhodných čísel pomocou hardvérového generátora
    std::mt19937 gen(rd());  // Vytvorenie generátora čísel s Mersenne Twister algoritmom

    for (auto &mravec : mravce) {
        mravec.x = gen() % VELKOST_POLA;
        mravec.y = gen() % VELKOST_POLA;
    }
}

// Funkcia na manuálne určenie počiatočných pozícií mravcov v mriežke.
void manualneNastaveniePozicieMravcov() {
    for (int i = 0; i < POCET_MRAVCOV; ++i) {
        std::cout << "Zadaj pociatocne súradnice pre mravca " << (i + 1) << " (x y): ";
        std::cin >> mravce[i].x >> mravce[i].y;

        // Kontrola, či zadané súradnice nie sú mimo rozmerov pola
        if (mravce[i].x < 0 || mravce[i].x >= VELKOST_POLA || mravce[i].y < 0 || mravce[i].y >= VELKOST_POLA) {
            std::cout << "Suradnice mimo rozsah. Skuste to znova." << std::endl;
            --i;  // Vrátime sa k predchádzajúcej iterácii cyklu, aby sme mohli zadávať súradnice znovu
        }
    }
}

void handleCollision(Kolizia kolizia) {
    //zániknú všetky zúčastnené mravce
    if (kolizia == Kolizia::ZANIK) {
        std::vector<Mravec> prezivsi;

        for (std::size_t i = 0; i < mravce.size(); ++i) {
            bool zrazka = false;

            for (std::size_t j = 0; j < mravce.size(); ++j) {
                if (i != j && mravce[i].x == mravce[j].x && mravce[i].y == mravce[j].y) {
                    zrazka = true;
                    break;
                }
            }

            if (!zrazka) {
                prezivsi.push_back(mravce[i]);
            }
        }

        mravce = prezivsi;
    }
        //prežije iba jeden mravec
    else if (kolizia == Kolizia::PREZITIE) {
        std::vector<Mravec> prezivsi;

        for (auto &mravec: mravce) {
            auto it = std::find_if(prezivsi.begin(), prezivsi.end(),
                                   [&mravec](const Mravec &otherAnt) {
                                       return mravec.x == otherAnt.x && mravec.y == otherAnt.y;
                                   });

            if (it == prezivsi.end()) {
                prezivsi.push_back(mravec);
            }
        }

        mravce = prezivsi;
    }
        //polovica mravcov sa začne správať podľa inverznej logiky
    else if (kolizia == Kolizia::ZMENA_LOGIKY) {
        std::vector<std::size_t> mravceVZrazke;

        // Detekuje mravce na rovnakom policku
        for (std::size_t i = 0; i < mravce.size(); ++i) {
            for (std::size_t j = i + 1; j < mravce.size(); ++j) {
                if (mravce[i].x == mravce[j].x && mravce[i].y == mravce[j].y) {
                    mravceVZrazke.push_back(i);
                    mravceVZrazke.push_back(j);
                }
            }
        }

        // Zmeni logiku pohybu pre polovicu mravcov, ktorý sa zúčastnili kolízie
        for (std::size_t i = 0; i < mravceVZrazke.size() / 2; ++i) {
            mravce[mravceVZrazke[i]].inverzna = true;
        }
    }
}

// Funkcia pre vstup od používateľa
void uzivatelskyVstup() {
    std::cin.ignore();  // Ignoruje existujúci vstup

    // Výzva pre používateľa na stlačenie 'Enter' pre pozastavenie/obnovenie simulácie alebo 'Q' pre ukončenie programu
    std::cout << "Stlacte 'Enter' pre pozastavenie/obnovenie simulacie alebo napiste 'Q' pre ukoncenie." << std::endl;

    while (true) {
        char uzivatelskyVstup;
        std::cin.get(uzivatelskyVstup);  // Načíta vstup od používateľa

        // Kontrola vstupu od používateľa
        if (uzivatelskyVstup == 'Q' || uzivatelskyVstup == 'q') {
            if (!simulaciaBezi) {
                skonciProgram = true;
                break;
            }
        } else if (uzivatelskyVstup == '\n') {
            // Zmeníme stav simulácie (pauza/obnovenie) stlačením klávesy 'Enter'
            simulaciaBezi = !simulaciaBezi;
            paused = !simulaciaBezi;
            std::cout << (simulaciaBezi ? "Obnovovanie" : "Pozastavenie") << " simulacie..." << std::endl;
        }
    }
}

// Hlavná smyčka simulácie
void simulacia() {
    while (!skonciProgram) {
        while (simulaciaBezi) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));  //
            prepisPole();
            handleCollision(MOD_KOLIZIE);
            vykresliPole();
        }

        // Ak je simulácia pozastavená, informuje používateľa o možnostiach obnovenia alebo ukončenia programu
        if (paused) {
            std::cout << "Simulacia je pozastavena. Stlacte 'Enter' pre obnovenie alebo 'Q' pre ukoncenie.\n" << std::endl;
            paused = false;
        }
    }
}

int main() {
    // Získanie veľkosti mriežky a počtu mravcov od používateľa
    std::cout << "Zadajte velkost pola: \n";
    std::cin >> VELKOST_POLA;

    std::cout << "Zadajte pocet mravcov: \n";
    std::cin >> POCET_MRAVCOV;

    // Inicializácia mriežky a mravcov
    pole = std::vector<std::vector<StavBunky>>(VELKOST_POLA, std::vector<StavBunky>(VELKOST_POLA, StavBunky::BIELA));
    mravce = std::vector<Mravec>(POCET_MRAVCOV, {VELKOST_POLA / 2, VELKOST_POLA / 2, SmerMravca::HORE});

    // Voľba režimu kolízií
    std::cout << "Zvolte rezim spravania pri kolizii :\n";
    std::cout << "1. Vsetky mravce, ktore sa stretnu na rovnakej pozicii zaniknu\n";
    std::cout << "2. Jeden mravec prezije, ak sa stretne viac mravcov na rovnakej pozicii\n";
    std::cout << "3. Polovica mravcov, ktore sa stretnu na rovnakej pozicii zacne vyuzivat inverznu logiku\n";

    int vyberTypKolizie;
    std::cin >> vyberTypKolizie;

    switch (vyberTypKolizie) {
        case 1:
            MOD_KOLIZIE = Kolizia::ZANIK;
            break;
        case 2:
            MOD_KOLIZIE = Kolizia::PREZITIE;
            break;
        case 3:
            MOD_KOLIZIE = Kolizia::ZMENA_LOGIKY;
            break;
        default:
            std::cout << "Chybne zadanie rezimu. Ukoncujem program...\n";
            return 1;
    }

    // Voľba režimu inicializácie čiernych buniek
    std::cout << "Zvolte rezim na ulozenie ciernych buniek:\n";
    std::cout << "1. Nahodny\n";
    std::cout << "2. Manualny\n";
    std::cout << "3. Nacitanie zo subora\n";

    int rezim;
    std::cin >> rezim;

    if (rezim == 1) {
        int pocetCiernych;
        std::cout << "Zadajte pocet ciernych buniek:\n";
        std::cin >> pocetCiernych;
        nahodneNastavenieCiernychBuniek(pocetCiernych, pole);
    } else if (rezim == 2) {
        manualneNastavenieCiernychBuniek();
    } else if (rezim == 3) {
        std::cout << "Zadajte nazov suboru na nacitanie pola:\n";
        std::string nazovSuboru;
        std::cin >> nazovSuboru;
        std::cout << "Otvaranie suboru: " << nazovSuboru << "..."<< std::endl;
        nastavenieCiernychBuniekZoSuboru(nazovSuboru);
    } else {
        std::cout << "Chybne zadanie rezimu. Ukoncujem program...\n";
        return 1;
    }

    // Voľba režimu inicializácie pozícií mravcov
    std::cout << "Zvolte rezim nastavenia pozicie mravcov:\n";
    std::cout << "1. Nahodne pozicie\n";
    std::cout << "2. Manualne zadanie pozicii\n";

    int mravecPos;
    std::cin >> mravecPos;

    if (mravecPos == 1) {
        nahodneNastaveniePozicieMravcov();
    } else if (mravecPos == 2) {
        manualneNastaveniePozicieMravcov();
    } else {
        std::cout << "Chybne zadanie rezimu. Ukoncujem program...\n";
        return 1;
    }

    // Zobrazenie počiatočnej mriežky
    std::cout << "Pociatocne pole: \n";
    vykresliPole();
    // Inštrukcie pre používateľa
    std::cout << "Ovladanie:\n";
    std::cout << "Ak simulacia bezi, stlacte 'Enter' pre pozastavenie. \n";
    std::cout << "Ak je simulacia pozastavena, stlacte 'Enter' pre obnovenie, alebo 'Q' pre ukoncenie. \n\n";
    // Voľba logiky pohybu mravcov
    std::cout << "Zvolte logiku pohybu mravcov:\n";
    std::cout << "1. Priama Logika\n";
    std::cout << "2. Inverzna Logika\n";

    int pohybMravca;
    std::cin >> pohybMravca;

    if (pohybMravca == 1) {
        // Priama Logika
        for (auto &mravec: mravce) {
            mravec.inverzna = false;
            mravec.smer = SmerMravca::HORE;
        }
    } else if (pohybMravca == 2) {
        // Inverzná logika
        for (auto &mravec: mravce) {
            mravec.inverzna = true;
            mravec.smer = SmerMravca::HORE;
        }
    } else {
        std::cout << "Chybne zadanie. Ukoncujem program...\n";
        return 1;
    }
    // Spustenie vlákien pre simuláciu a vstup od používateľa
    std::thread simulationThread(simulacia);
    std::thread userInputThread(uzivatelskyVstup);

    simulationThread.join();  // Čakáme na ukončenie vlákna simulácie
    userInputThread.join();   // Čakáme na ukončenie vlákna pre používateľský vstup

    // Výzva na používateľa, aby uložil finálnu mriežku
    char ulozenie;
    std::cout << "Zelate si ulozit vysledne pole do suboru? (A/N):\n";
    std::cin >> ulozenie;

    if (ulozenie == 'a' || ulozenie == 'A') {
        std::cout << "Zadajte nazov suboru pre ulozenie pola (.txt):\n";
        std::string nazovSuboru;
        std::cin >> nazovSuboru;

        ulozeniePolaDoSuboru(nazovSuboru);
        std::cout << "Pole bolo uspesne ulozene. \n" << std::endl;
    } else {
        std::cout << "Pole nebolo ulozene. \n" << std::endl;
    }
    return 0;
}

