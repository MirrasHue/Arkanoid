#include "Arkanoid.h"
#include "GameEntities.h"
#include <iostream>
#include <thread>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>


Arkanoid::Arkanoid()
{
    auto videoMode = sf::VideoMode::getDesktopMode();
    screenWidth  = videoMode.width;
    screenHeight = videoMode.height;

    m_window = std::make_unique<sf::RenderWindow>();
    m_window->create(videoMode, "Arkanoid", sf::Style::Fullscreen);
    //m_window->setFramerateLimit(30); // Nice function to test if the game loop behaves well at different FPS

    m_ball = std::make_unique<Ball>();
    m_paddle = std::make_unique<Paddle>();

    newGame();
}

void Arkanoid::newGame()
{
    Ball::bHitBottom = false;
    bIsBallDetached = false;
    bShouldRestart = false;

    m_paddle->setPosition(screenWidth / 2.f, screenHeight - 25.f);

    for(uint32_t row = 0; row < brickCountX; ++row)
        for(uint32_t column = 0; column < brickCountY; ++column)
            m_bricks.emplace_back(row, column);

    gameLoop();
}

void Arkanoid::handleEvents()
{
    sf::Event event;
    while(m_window->pollEvent(event))
    {
        switch(event.type)
        {
        case sf::Event::Closed:
            m_window->close();
            break;

        case sf::Event::KeyPressed:
            if(event.key.code == sf::Keyboard::Escape)
                m_window->close();

            if(Ball::bHitBottom)
                if(event.key.code == sf::Keyboard::Enter)
                    bShouldRestart = true;

            if(event.key.code == sf::Keyboard::Space && !bIsBallDetached)
            {
                bIsBallDetached = true;
                m_ball->velocity.y = -std::sin((m_paddle->angleIndicator.getRotation() + 90.f) * 3.14159265f / 180.f);
                m_ball->velocity.x = -std::cos((m_paddle->angleIndicator.getRotation() + 90.f) * 3.14159265f / 180.f);
            }
            break;
        }
    }
}

// Both game loop and draw methods were based on two excellent articles on the topic
// https://gameprogrammingpatterns.com/game-loop.html
// https://gafferongames.com/post/fix_your_timestep/
void Arkanoid::gameLoop()
{
    // Fixed time step for updating the game
    const sf::Time dt = sf::seconds(1.f / 250.f);

    sf::Time accumulator;

    sf::Clock clock;

    // Launch a new thread to get the player input
    std::thread inputThread(&Arkanoid::getInput, this);
    inputThread.detach();

    while(m_window->isOpen() && !bShouldRestart)
    {
        sf::Time frameTime = clock.restart(); // How much real time passed since the last frame
        accumulator += frameTime;

        handleEvents();

        while(accumulator >= dt && !Ball::bHitBottom)
        {
            update(dt.asSeconds());
            accumulator -= dt;
        }

        // The leftover amount in accumulator divided by dt gives us how far into the
        // next frame we are, so we can render accordingly, resulting in smoother motion
        draw(accumulator / dt, dt.asSeconds());
    }

    if(Arkanoid::bShouldRestart)
        newGame();
}

// Runs on a separate thread
void Arkanoid::getInput()
{
    using Kb = sf::Keyboard;

    while(!Ball::bHitBottom)
    {
        // Paddle
        if(Kb::isKeyPressed(Kb::D) && Kb::isKeyPressed(Kb::A))
            m_paddle->moveDirectionState = Paddle::EMD_NotMoving;
        else
        if(Kb::isKeyPressed(Kb::D))
            m_paddle->moveDirectionState = Paddle::EMD_Right;
        else
        if(Kb::isKeyPressed(Kb::A))
            m_paddle->moveDirectionState = Paddle::EMD_Left;
        else
            m_paddle->moveDirectionState = Paddle::EMD_NotMoving;

        // Angle Indicator
        if(Kb::isKeyPressed(Kb::Left) && Kb::isKeyPressed(Kb::Right))
            m_paddle->indicatorRotationState = Paddle::EIR_NotRotating;
        else
        if(Kb::isKeyPressed(Kb::Right))
            m_paddle->indicatorRotationState = Paddle::EIR_RotateRight;
        else
        if(Kb::isKeyPressed(Kb::Left))
            m_paddle->indicatorRotationState = Paddle::EIR_RotateLeft;
        else
            m_paddle->indicatorRotationState = Paddle::EIR_NotRotating;
    }
}

void Arkanoid::update(float dt)
{
    if(bIsBallDetached)
        m_ball->update(dt);
    else // If it isn't detached, move the ball alongside the paddle
        m_ball->setPosition(m_paddle->x, m_paddle->y - 50.f);

    if(Ball::bHitBottom) // If the ball update caused the game over in this frame, no need to proceed
        return;

    m_paddle->update(dt);

    checkCollision();

    for(auto& brick : m_bricks)
        checkCollision(brick);

    // std::remove_if moves the elements in the given range in such a way that the elements that aren't to
    // be removed appear at the front, and returns an iterator past the last not to be removed element.
    // Now we can properly erase the adjacent dummies, starting at the returned iterator up to the end.
    m_bricks.erase(std::remove_if(m_bricks.begin(), m_bricks.end(),
                                  [](const auto& brick){return brick.isDestroyed;}),
                   m_bricks.end());
}

void Arkanoid::draw(float nextFramePrediction, float dt) const
{
    m_window->clear();

    if(!Ball::bHitBottom)
    {
        /*Ball tempBall = *m_ball;
        Paddle tempPaddle = *m_paddle;

        //tempBall.moveBall((1.f + nextFramePrediction) * dt);
        //tempBall.setFillColor(sf::Color::Green);

        //tempPaddle.movePaddle((1.f + nextFramePrediction) * dt);
        //tempPaddle.setFillColor(sf::Color::Magenta);

        //m_window->draw(tempBall);
        m_window->draw(tempPaddle);
        m_window->draw(tempPaddle.angleIndicator);*/

        m_window->draw(*m_ball);
        m_window->draw(*m_paddle);
        m_window->draw(m_paddle->angleIndicator);

        for (const auto &brick: m_bricks)
            m_window->draw(brick);
    }
    else
    {
        textHandler.setText("Press Enter to play again...");
        m_window->draw(textHandler.text);
    }

    m_window->display();
}

void Arkanoid::checkCollision()
{
    if(! m_ball->collider.intersects(m_paddle->collider))
        return;

    // Only reflect the ball in the desired direction if it hits the middle part of the paddle
    if(m_ball->x > m_paddle->x - m_paddle->width / 4.f && m_ball->x < m_paddle->x + m_paddle->width / 4.f)
    {
        m_ball->velocity.y = -std::sin((m_paddle->angleIndicator.getRotation() + 90.f) * 3.14159265f / 180.f);
        m_ball->velocity.x = -std::cos((m_paddle->angleIndicator.getRotation() + 90.f) * 3.14159265f / 180.f);
    }
    else
    {
        m_ball->velocity.y = -m_ball->velocity.y;

        // Reverse the ball if it hits the very corner of the paddle's right or left side
        if(m_ball->x > m_paddle->x + (m_paddle->width / 2.f - m_paddle->width / 16.f) && m_ball->velocity.x < 0)
            m_ball->velocity.x *= -1;
        else
        if(m_ball->x < m_paddle->x - (m_paddle->width / 2.f - m_paddle->width / 16.f) && m_ball->velocity.x > 0)
            m_ball->velocity.x *= -1;
    }
}

void Arkanoid::checkCollision(Brick& brick)
{
    if(! m_ball->collider.intersects(brick.collider))
        return;

    brick.isDestroyed = true;

    float distanceX = std::abs(m_ball->x - brick.x);
    float distanceY = std::abs(m_ball->y - brick.y);

    // Add a little bit of extra room to catch corners hits (works better for high update rate), same for the sides
    if(distanceX <= brick.width / 2.f + 10.f) // Check if collide with the bottom or top of the brick
    {
        if((m_ball->y > brick.y && m_ball->velocity.y < 0)
        || (m_ball->y < brick.y && m_ball->velocity.y > 0))
            m_ball->velocity.y *= -1;
    }

    if(distanceY <= brick.height / 2.f + 10.f) // Check if collide with the right or left side of the brick
    {
        if((m_ball->x > brick.x && m_ball->velocity.x < 0)
        || (m_ball->x < brick.x && m_ball->velocity.x > 0))
            m_ball->velocity.x *= -1;
    }
}

Arkanoid::~Arkanoid() = default;
