#ifndef MAZE_GENERATOR_H
#define MAZE_GENERATOR_H

#include <vector>
#include <utility>
#include <random>

class MazeGenerator {
public:
    MazeGenerator(int width, int height);
    void generate();
    void highlightPath();
    void printMaze(bool withPath = false) const;
    void postaviPredmeteIMinotaura(int brojPredmeta);

    // geteri
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getRobotX() const { return robotX; }
    int getRobotY() const { return robotY; }
    int getEndX() const { return endX; }
    int getEndY() const { return endY; }
    char getCell(int x, int y) const { return maze[y][x]; }
    void setCell(int x, int y, char c) { maze[y][x] = c; }

private:
    int width, height;
    std::vector<std::vector<char>> maze;
    std::mt19937 rng;
    int startX, startY;
    int endX, endY;
    int robotX, robotY;
    std::vector<std::pair<int, int>> putPolja; // vekror koordinata puta, za postavljanje predmeta

    bool isValidCell(int x, int y) const;
};

#endif
