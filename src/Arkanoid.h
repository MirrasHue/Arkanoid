#pragma once

#include <memory>
//#include <vector>
#include "Utils.h"
//#include <iostream>

namespace sf
{
    class RenderWindow;
}

class Ball;
class Paddle;
class Brick;

class Arkanoid
{
public:
    // There will be only one instance of the game, just studying the Singleton pattern
    static Arkanoid& getInstance()
    {
        static Arkanoid instance;
        //std::cout<<&instance<<"\n";
        return instance;
    }

    Arkanoid(const Arkanoid&) = delete;
    Arkanoid& operator= (const Arkanoid&) = delete;

    void newGame();

private:
                // We have to prevent the compiler from generating an
    Arkanoid(); // inlined destructor in order to make unique_ptr work
   ~Arkanoid(); // with forward declarations, so the dtor must be in a
                // .cpp file where the actual types are complete.

    void handleEvents();
    void gameLoop();
    void getInput();
    void update(float dt);
    void draw(float nextFramePrediction, float dt) const;

    void checkCollision(); // Ball x Paddle
    void checkCollision(Brick&); // Ball x Brick

private:

    std::unique_ptr<sf::RenderWindow> m_window;
    std::unique_ptr<Ball>             m_ball;
    std::unique_ptr<Paddle>           m_paddle;
    std::vector<Brick>                m_bricks;

    uint32_t brickCountX = 8,
             brickCountY = 5;

    bool bIsBallDetached{};
    bool bShouldRestart{true};

    mutable TextHandler textHandler;

public:

    static inline uint32_t screenWidth{}, screenHeight{};
};

