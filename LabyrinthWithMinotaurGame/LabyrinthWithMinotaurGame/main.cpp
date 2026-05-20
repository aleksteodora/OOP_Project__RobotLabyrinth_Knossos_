#include "MazeGenerator.h"
#include "Game.h"
#include <ctime>
#include <iostream>
#include <chrono>

int main() {
    srand(time(nullptr));

    int width, height;
    do {
        std::cout << "Unesite sirinu (>15): ";
        std::cin >> width;
        std::cout << "Unesite visinu (>15): ";
        std::cin >> height;
    } while (width <= 15 || height <= 15);

    MazeGenerator gen(width, height);
    auto start = std::chrono::high_resolution_clock::now(); //d
    gen.generate();
    auto end = std::chrono::high_resolution_clock::now(); //d
    std::chrono::duration<double> duration = end - start; //d
    gen.highlightPath();

    int brojPredmeta;
    do {
        std::cout << "Unesite broj predmeta (mora biti veci od 3): ";
        std::cin >> brojPredmeta;
    } while (brojPredmeta <= 3);

    gen.postaviPredmeteIMinotaura(brojPredmeta);

    std::cout << "\n--- Lavirint ---\n";
    gen.printMaze(false);
    std::cout << "\n--- Lavirint sa putem ---\n";
    gen.printMaze(true);

    std::cout << "\nVreme generisanja lavirinta: " << duration.count() << " sekundi\n"; //d

    std::cout << "\n--- Interaktivna igra ---";
    Game game(gen);
    game.play();

    return 0;
}
