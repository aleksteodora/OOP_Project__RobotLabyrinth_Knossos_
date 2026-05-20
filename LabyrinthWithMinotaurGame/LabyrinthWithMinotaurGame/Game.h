#ifndef GAME_H
#define GAME_H

#include "MazeGenerator.h"
#include "Item.h"
#include <vector>
#include <memory>
#include <string>

struct ActiveEffect {
    std::string type; // Fog, Sword, Shield, Hammer
    int turnsLeft;
};

class Game {
public:
    Game(MazeGenerator& gen);
    void play();

private:
    MazeGenerator& gen;
    int robotX, robotY;
    int minotaurX, minotaurY;
    std::vector<std::shared_ptr<Item>> items;

    bool hasSword = false;
    bool hasShield = false;
    bool hasHammer = false;
    int fogTurns = 0;

    std::vector<ActiveEffect> activeEffects; // vektor koji cuva aktivne efekte

    void printMaze();
    bool moveRobot(char command);
    void moveMinotaur();
    bool checkGameOver();
    void pickupItem(int x, int y);
    bool isAdjacentToMinotaur();
    void saveGameResult(const std::string& filename, const std::string& finalMessage);

};

#endif
