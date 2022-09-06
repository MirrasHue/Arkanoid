#include "Arkanoid.h"
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

int main()
{
    Arkanoid& game = Arkanoid::getInstance();
    game.newGame();
    
    return 0;
}
