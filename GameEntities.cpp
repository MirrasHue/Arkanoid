#include "GameEntities.h"
#include "MathUtils.h"
#include "Arkanoid.h"


Ball::Ball()
{
    setRadius(radius);
    setOrigin(radius, radius);
    setFillColor(sf::Color::Cyan);

    collider = getLocalBounds();
    collider.width  *= 0.75f; // Shrink the collision volume a bit to
    collider.height *= 0.75f; // make it adjust better with the ball
}

void Ball::moveBall(float dt)
{
    move(Math::GetUnitVector(velocity) * speed * dt); // Normalize so that the ball doesn't go faster when moving diagonally
}

void Ball::update(float dt)
{
    moveBall(dt);

    x = getPosition().x;
    y = getPosition().y;

    if(y + radius >= Arkanoid::screenHeight) // The ball hit the bottom, game over
    {
        Arkanoid::bGameOver = true;
        return;
    }

    collider.left = x - collider.width / 2.f;  // Update pos of ball's collider to ensure
    collider.top  = y - collider.height / 2.f; // that it is at the same spot as the ball

    // Bounce the ball if it hits the screen's edges
    if(x + radius >= Arkanoid::screenWidth || x - radius <= 0)
        velocity.x *= -1;

    if(y - radius <= 0)
        velocity.y *= -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Paddle::Paddle()
{
    setSize({width, height});
    setOrigin(width / 2.f, height / 2.f);
    setFillColor(sf::Color::Red);
}

void Paddle::movePaddle(float dt)
{
    if(direction == EMD_Right)
        move(speed * dt, 0.f);
    else
    if(direction == EMD_Left)
        move(-speed * dt, 0.f);
}

void Paddle::update(float dt)
{
    movePaddle(dt);

    x = getPosition().x;
    y = getPosition().y;

    // Keep the paddle inside the screen
    if(x + width / 2.f > Arkanoid::screenWidth)
        setPosition(Arkanoid::screenWidth - width / 2.f, y);
    else
    if(x - width / 2.f < 0)
        setPosition(width / 2.f, y);

    // Get the paddle's bounds each frame to later check collision with the ball
    collider = getGlobalBounds();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Brick::Brick(uint32_t row, uint32_t column)
{
    setSize({width, height});
    setOrigin(width / 2.f, height / 2.f);
    setPosition((row + 1) * (width + 4) - 5, // Set the bricks in a grid pattern
                (column + 2) * (height + 4));
    setFillColor(sf::Color::White);

    // Get the brick's bounds to later check collision with the ball
    collider = getGlobalBounds();

    x = getPosition().x;
    y = getPosition().y;
}
