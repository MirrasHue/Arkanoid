#include "Arkanoid.h"
#include "GameEntities.h"
#include <iostream>
#include <thread>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>


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
    Arkanoid::bGameOver = false;
    bShouldRestart = false;

    m_ball->setPosition(screenWidth / 2.f, screenHeight - 100.f);
    m_ball->velocity = {-1, -1};

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

            if(Arkanoid::bGameOver)
                if(event.key.code == sf::Keyboard::Enter)
                    bShouldRestart = true;
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
    const sf::Time dt = sf::seconds(1.f / 299.f);

    sf::Time accumulator;

    sf::Clock clock;
    
    // Launch a new thread to get the player input
    std::thread inputThread(&Arkanoid::getInput, this);
    inputThread.detach();

    while(m_window->isOpen() && !Arkanoid::bGameOver)
    {
        sf::Time frameTime = clock.restart(); // How much real time passed since the last frame
        accumulator += frameTime;

        handleEvents();

        while(accumulator >= dt && !Arkanoid::bGameOver)
        {
            update(dt.asSeconds());
            accumulator -= dt;
        }

        // The leftover amount in accumulator divided by dt gives us how far into the
        // next frame we are, so we can render accordingly, resulting in smoother motion
        draw(accumulator / dt, dt.asSeconds());
    }

    if(Arkanoid::bGameOver)
        handleGameOver();
}

// Runs on a separate thread
void Arkanoid::getInput()
{
    using Kb = sf::Keyboard;

    while(!Arkanoid::bGameOver)
    {
        if(Kb::isKeyPressed(Kb::D) && Kb::isKeyPressed(Kb::A))
        {
            m_paddle->direction = Paddle::EMD_None;
            continue;
        }

        if(Kb::isKeyPressed(Kb::D))
        {
            m_paddle->direction = Paddle::EMD_Right;
        }
        else
        if(Kb::isKeyPressed(Kb::A))
        {
            m_paddle->direction = Paddle::EMD_Left;
        }
        else
        {
            m_paddle->direction = Paddle::EMD_None;
        }
    }
}

void Arkanoid::update(float dt)
{
    m_ball->update(dt);

    if(Arkanoid::bGameOver) // If the ball update caused the game over in this frame, no need to proceed
        return;

    m_paddle->update(dt);

    checkCollision();

    for(auto& brick : m_bricks)
        checkCollision(brick);

    // std::remove_if moves the elements in the given range in such a way that the elements that aren't to
    // be removed appear at the vector's front, and returns an iterator past the last non removed element. 
    // Now we can properly erase the adjacent dummies, starting at the returned iterator up to the end.
    m_bricks.erase(std::remove_if(m_bricks.begin(), m_bricks.end(),
                                  [](const auto& brick){return brick.isDestroyed;}),
                   m_bricks.end());
}

void Arkanoid::draw(float nextFramePrediction, float dt) const
{
    m_window->clear();

    Ball tempBall = *m_ball;
    Paddle tempPaddle = *m_paddle;

    tempBall.moveBall((1.f + nextFramePrediction) * dt);
    //tempBall.setFillColor(sf::Color::Green);

    tempPaddle.movePaddle((1.f + nextFramePrediction) * dt);
    //tempPaddle.setFillColor(sf::Color::Magenta);

    m_window->draw(tempBall);
    m_window->draw(tempPaddle);

    //m_window->draw(*m_ball);
    //m_window->draw(*m_paddle);

    for(const auto& brick : m_bricks)
        m_window->draw(brick);

    m_window->display();
}

void Arkanoid::handleGameOver()
{
    sf::Font font;
    if(!font.loadFromFile("bahnschrift.ttf"))
    {
        std::cerr<<"Error when loading font\n";
        return;
    }

    sf::Text text;
    text.setFont(font);
    text.setString("Press Enter to play again...");
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition({20, 10});

    while(m_window->isOpen() && !bShouldRestart)
    {
        handleEvents();

        m_window->clear();
        m_window->draw(text);
        m_window->display();
    }

    if(bShouldRestart)
        newGame();
}

void Arkanoid::checkCollision()
{
    if(! m_ball->collider.intersects(m_paddle->collider))
        return;

    m_ball->velocity.y = -1;

    // Only reverse the ball trajectory if it hits 1 / 4 of the paddle's left or right part
    if(m_ball->x > m_paddle->x + m_paddle->width / 4.f)
        m_ball->velocity.x = 1;
    else
    if(m_ball->x < m_paddle->x - m_paddle->width / 4.f)
        m_ball->velocity.x = -1;
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
        if(m_ball->y > brick.y)
            m_ball->velocity.y = 1;
        else
            m_ball->velocity.y = -1;
    }

    if(distanceY <= brick.height / 2.f + 10.f) // Check if collide with the right or left side of the brick
    {
        if(m_ball->x > brick.x)
            m_ball->velocity.x = 1;
        else
            m_ball->velocity.x = -1;
    }
}

Arkanoid::~Arkanoid() = default;
