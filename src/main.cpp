#include "Arkanoid.h"

int main()
{
    Arkanoid& game = Arkanoid::getInstance();
    game.newGame();
    
    return 0;
}
