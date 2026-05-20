#include "Game.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>

Game::Game(MazeGenerator& gen) : gen(gen) { // uzimamo referencu na MazeGenerator objekat
    robotX = gen.getRobotX(); // inicijalne x i z koordinate robota  
    robotY = gen.getRobotY();

    // nadji Minotaura
    minotaurX = minotaurY = -1;
    for (int y = 0; y < gen.getHeight(); ++y) {
        for (int x = 0; x < gen.getWidth(); ++x) {
            if (gen.getCell(x, y) == 'M') {
                minotaurX = x;
                minotaurY = y;
            }
        }
    }

    // kreiranje Item objekata na osnovu oznaka predmeta u lavirintu 
    for (int y = 0; y < gen.getHeight(); ++y) {
        for (int x = 0; x < gen.getWidth(); ++x) {
            if (gen.getCell(x, y) == 'P') {
                int t = rand() % 4; // nasumican odabir tipa predmeta
                if (t == 0) 
                    items.push_back(std::make_shared<Fog>(x, y));
                if (t == 1) 
                    items.push_back(std::make_shared<Sword>(x, y));
                if (t == 2) 
                    items.push_back(std::make_shared<Shield>(x, y));
                if (t == 3) 
                    items.push_back(std::make_shared<Hammer>(x, y));
            }
        }
    }
}

// metoda za upravljanje igrom
void Game::play() {

    char cmd; // unos komande korisnika

    while (true) { 
        printMaze(); // ispis trenutnog stanja lavirinta

        // ispis aktivnih efekata
        if (!activeEffects.empty()) {
            std::cout << "Aktivni efekti: ";
            for (auto& eff : activeEffects)
                std::cout << eff.type << " (" << eff.turnsLeft << ") ";
            std::cout << "\n";
        }

        std::cout << "\nUnesi komandu (W/A/S/D za kretanje, Q za izlaz): ";
        std::cin >> cmd;
        if (cmd == 'Q' || cmd == 'q') 
            break;

        // pomeranje robota ako je potez validan
        if (moveRobot(cmd)) {
            pickupItem(robotX, robotY); // da li je robot pokupio predmet
            if (checkGameOver()) // da li je igra zavrsena
                break;
            moveMinotaur(); // pomeranje minotaura
            if (checkGameOver())  // da li je sada igra zavrsena
                break;
        }

        // azuriranje trajanja aktivnih efekata
        for (int i = 0; i < (int)activeEffects.size(); ++i) {
            activeEffects[i].turnsLeft--; 
            if (activeEffects[i].turnsLeft <= 0) { // ako efekat istice
                if (activeEffects[i].type == "Sword") 
                    hasSword = false;
                if (activeEffects[i].type == "Shield") 
                    hasShield = false;
                if (activeEffects[i].type == "Hammer") 
                    hasHammer = false;
                if (activeEffects[i].type == "Fog") 
                    fogTurns = 0;

                std::cout << "Efekat " << activeEffects[i].type << " je istekao!\n";
                activeEffects.erase(activeEffects.begin() + i); // uklanjamo efekat
                i--; // pomeramo indeks jer je vektor skracen
            } 
        }
    }
   
    // poruka na kraju igre, pamtimo zbog upisa u fajl
    std::string endMessage;
    if (robotX == -1)
        endMessage = "Minotaur je pojeo robota!";
    else if (gen.getCell(robotX, robotY) == 'I')
        endMessage = "Cestitamo! Robot je stigao do izlaza!";
    else
        endMessage = "Igra je prekinuta!";

    std::cout << endMessage << "\n";
    saveGameResult("game_result.txt", endMessage);
}

// ispis lavirinta u konzoli
void Game::printMaze() {
    std::cout << "\n";
    for (int y = 0; y < gen.getHeight(); ++y) {
        for (int x = 0; x < gen.getWidth(); ++x) {
            // ako je magla aktivna, prikazujemo samo 3x3 oko robota
            if (fogTurns > 0 && (std::abs(robotX - x) > 1 || std::abs(robotY - y) > 1)) {
                std::cout << "?";
                continue;
            }

            // prikaz u boji
            if (x == robotX && y == robotY) 
                std::cout << "\033[32mR\033[0m";
            else if (x == minotaurX && y == minotaurY && minotaurX != -1) 
                std::cout << "\033[31mM\033[0m";
            else {
                char c = gen.getCell(x, y);
                if (c == 'P') std::cout << "\033[33mP\033[0m";
                else if (c == 'I' || c == 'U') std::cout << "\033[35m" << c << "\033[0m";
                else if (c == '*') std::cout << '.';
                else std::cout << c;
            }
        }
        std::cout << "\n";
    }
}

// pomeranje robota prema unetoj komandi
bool Game::moveRobot(char command) {
    int nx = robotX, ny = robotY;
    if (command == 'W' || command == 'w') 
        ny--;
    if (command == 'S' || command == 's') 
        ny++;
    if (command == 'A' || command == 'a') 
        nx--;
    if (command == 'D' || command == 'd') 
        nx++;

    // provera granica lavirinta
    if (nx < 0 || ny < 0 || nx >= gen.getWidth() || ny >= gen.getHeight()) 
        return false;
    // da li je zid i da li robot ima cekic
    if (gen.getCell(nx, ny) == '#' && !hasHammer) 
        return false;

    gen.setCell(robotX, robotY, '.');

    // azuriranje koordinata robota
    robotX = nx; robotY = ny;
    return true;
}

// provera je l robot pokupio predmet zbog aktiviranja efekta
void Game::pickupItem(int x, int y) {
    for (auto it = items.begin(); it != items.end(); ++it) {
        if ((*it)->getX() == x && (*it)->getY() == y) { // ako robot stoji na predmetu
            (*it)->applyEffect(); // prikaz efekta

            // odredjivanje tipa predmeta
            std::string type;
            if (dynamic_cast<Sword*>(it->get())) 
                type = "Sword";
            if (dynamic_cast<Shield*>(it->get())) 
                type = "Shield";
            if (dynamic_cast<Hammer*>(it->get()))
                type = "Hammer";
            if (dynamic_cast<Fog*>(it->get())) 
                type = "Fog";

            // osvezi ili dodaj novi efekat
            bool found = false;
            for (auto& eff : activeEffects) {
                if (eff.type == type) {
                    eff.turnsLeft = 4 ;
                    found = true;
                    break;
                }
            }
            if (!found) 
                activeEffects.push_back({ type, 4 }); // dodaj novi

            // odmah aktiviraj efekat
            if (type == "Sword") 
                hasSword = true;
            if (type == "Shield") 
                hasShield = true;
            if (type == "Hammer") 
                hasHammer = true;
            if (type == "Fog") 
                fogTurns = 3;

            gen.setCell(x, y, '.'); // ukloni predmet
            items.erase(it);
            break;
        }
    }
}

// provera da li je robot pored minotaura
bool Game::isAdjacentToMinotaur() {
    return std::abs(robotX - minotaurX) + std::abs(robotY - minotaurY) == 1;
}

// pomeranje Minotaura
void Game::moveMinotaur() {
    if (minotaurX == -1) 
        return; // minotaur unisten

    if (isAdjacentToMinotaur()) {
        if (hasShield) {
            std::cout << "Minotaur te napada, ali si zaštićen štitom!\n";
            return;
        }
        if (hasSword) {
            std::cout << "Ubio si Minotaura mačem!\n";
            minotaurX = minotaurY = -1;
            return;
        }

        std::cout << "Minotaur te pojede!\n";
        robotX = robotY = -1;
        return;
    }

    // normalno kretanje minotaura
    int dirs[4][2] = { {0,-1},{0,1},{-1,0},{1,0} }; 
    std::vector<int> valid; // validni potezi
    for (int i = 0; i < 4; ++i) {
        int nx = minotaurX + dirs[i][0];
        int ny = minotaurY + dirs[i][1];
        if (nx >= 0 && ny >= 0 && nx < gen.getWidth() && ny < gen.getHeight()) {
            char c = gen.getCell(nx, ny);
            if (c != '#' && !(nx == robotX && ny == robotY)) valid.push_back(i);
        }
    }
    if (!valid.empty()) {
        int dir = valid[rand() % valid.size()];
        gen.setCell(minotaurX, minotaurY, '.');

        minotaurX += dirs[dir][0];
        minotaurY += dirs[dir][1];

        // ako minotaur stane na predmet, unistava ga
        if (gen.getCell(minotaurX, minotaurY) == 'P') {
            std::cout << "Minotaur je unistio predmet!\n";
            gen.setCell(minotaurX, minotaurY, '.');
        }
    }
}

// cuvanje rezultata u fajl
void Game::saveGameResult(const std::string& filename, const std::string& finalMessage) {
    std::ofstream fout(filename);
    if (!fout.is_open()) 
        return;

    for (int y = 0; y < gen.getHeight(); ++y) {
        for (int x = 0; x < gen.getWidth(); ++x) {
            char c = gen.getCell(x, y);

            if (x == robotX && y == robotY) 
                fout << 'R';
            else if (x == minotaurX && y == minotaurY && minotaurX != -1) 
                fout << 'M';
            else fout << c;
        }
        fout << "\n";
    }

    fout << "\n" << finalMessage << "\n";
    fout.close();
}

// provera kraja igre
bool Game::checkGameOver() {
    if (robotX == -1)  // robot pojeden
        return true;
    if (gen.getCell(robotX, robotY) == 'I')  // robot je stigao do izlaza
        return true;
    return false; // nije gotova igra
}
