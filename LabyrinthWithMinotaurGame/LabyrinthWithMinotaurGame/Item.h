#ifndef ITEM_H
#define ITEM_H
#include <string>
#include <iostream>
#include <memory>

class Item {
public:
    Item(int x, int y) : x(x), y(y) {}
    virtual ~Item() {}
    int getX() const { return x; }
    int getY() const { return y; }
    virtual void applyEffect() = 0;
protected:
    int x, y;
};

class Fog : public Item {
public:
    using Item::Item;
    void applyEffect() override {
        std::cout << "Efekat: Magla rata! Vidljivost je ogranicena na 3x3 polja.\n";
    }
};

class Sword : public Item {
public:
    using Item::Item;
    void applyEffect() override {
        std::cout << "Efekat: Mac! Mozes da unistis Minotaura.\n";
    }
};

class Shield : public Item {
public:
    using Item::Item;
    void applyEffect() override {
        std::cout << "Efekat: Stit! Mozes da se odbranis od Minotaura.\n";
    }
};

class Hammer : public Item {
public:
    using Item::Item;
    void applyEffect() override {
        std::cout << "Efekat: Cekic! Mozes da prodjes kroz zid.\n";
    }
};

#endif
