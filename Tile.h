#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include <vector>
using namespace std;

class Tile {
    bool isMine;
    bool revealed;
    bool flagged;
    bool leaderboardOpen;
    int value = 0;

    const sf::Texture* empty;
    const sf::Texture* hidden;
    const sf::Texture* flag;

    vector<Tile*> adjacent; //initially uninitialized
    vector<sf::Texture> faces;

    sf::Sprite hiddenSprite;
    sf::Sprite valueSprite;
    sf::Sprite emptySprite;
    sf::Sprite mineSprite;
    sf::Sprite flagSprite;

public:
    //parameterized constructor
    Tile(vector<sf::Texture> &faces, sf::Texture *hidden, sf::Texture *shown, sf::Texture *flag, sf::Texture *mine, bool isMine) :
    hiddenSprite(*hidden), valueSprite(*hidden), emptySprite(*shown), mineSprite(*mine), flagSprite(*flag) {
        this->faces = faces; // nums 1-8
        this->hidden = hidden; // hidden tile texture
        this->flag = flag; //flag texture
        this->isMine = isMine; // is this tile a mine?
        this->empty = shown; //shown tile texture

        //default
        this->revealed = false;
        this->flagged = false;
        this->leaderboardOpen = false;
    }

    // takes in vector or neighboring tiles and adds to adjacent
    void setAdjacent(vector<Tile*> &neighbors) {
        adjacent = neighbors;
    }

    // checks if click is on current tile
    bool contains(sf::Vector2f point) {
        return hiddenSprite.getGlobalBounds().contains(point);
    }

    //sets inital tile positions for all sprites
    void setPosition(float x, float y) {
        hiddenSprite.setPosition(sf::Vector2f(x, y));
        valueSprite.setPosition(sf::Vector2f(x, y));
        emptySprite.setPosition(sf::Vector2f(x, y));
        mineSprite.setPosition(sf::Vector2f(x, y));
        flagSprite.setPosition(sf::Vector2f(x, y));
    }

    //draws actual tile
    void draw(sf::RenderWindow& window) {
        if (leaderboardOpen) {
            window.draw(emptySprite);
        }
        else if (!revealed) {
            window.draw(hiddenSprite);
            if (flagged) {
                window.draw(flagSprite);
            }
        } else {
            window.draw(emptySprite);
            if (flagged) {
                window.draw(flagSprite);
            }
            if (isMine) {
                window.draw(mineSprite);
            } else if (value > 0) {
                valueSprite.setTexture(faces[value-1]);
                window.draw(valueSprite);
            }
        }
    }

    // is current tile revealed?
    bool isRevealed() {
        return revealed;
    }

    // reveal current tile (used for when tile is clicked) and checks if current tile is a mine
    void reveal() {
        revealed = true;
    }

    //hides all tiles for reset
    void hide() {
        revealed = false;
        flagged = false;
        value = 0;
    }

    void floodReveal(){
        if (revealed) {
            return;
        }
        revealed = true;

        if (value > 0) {
            valueSprite.setTexture(faces[value-1]);
            return;
        }

        for (Tile* neighbor: adjacent) {
            if (!neighbor->checkMine()) {
                neighbor->floodReveal();
            }
        }
    }

    // sets flag on right click (draws flag on top of hidden)
    void setFlag() {
        flagged = true;
    }

    void removeFlag() {
        flagged = false;
    }

    bool hasFlag() {
        return flagged;
    }

    // getter function for isMine bool
    bool checkMine() {
        return isMine;
    }

    void setVal() {
        for (int i = 0; i < adjacent.size(); i++) {
            if (adjacent[i]->checkMine()) {
                value++;
            }
        }
    }

    int getVal() {
        return value;
    }

    void leaderboardState() {
        leaderboardOpen = (!leaderboardOpen);
        if (leaderboardOpen) {
        } else {
        }
    }
};

#ifndef MINESWEEPER_CARD_H
#define MINESWEEPER_CARD_H

#endif //MINESWEEPER_CARD_H