#include "Arkanoid.h"
#include "GameEntities.h"
#include <iostream>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>


Arkanoid::Arkanoid()
{
    auto VideoMode = sf::VideoMode::getDesktopMode();
    screenWidth  = VideoMode.width;
    screenHeight = VideoMode.height;

    m_window = std::make_unique<sf::RenderWindow>();
    m_window->create(VideoMode, "Arkanoid", sf::Style::Fullscreen);
    //m_window->setFramerateLimit(30); // Nice function to test if the game loop behaves well at different FPS

    m_ball = std::make_unique<Ball>();
    m_paddle = std::make_unique<Paddle>();

    newGame();
}

void Arkanoid::newGame()
{
    bGameOver = false;

    m_ball->setPosition(screenWidth / 2.f, screenHeight - 100.f);
    m_ball->velocity = {-1, -1};

    m_paddle->setPosition(screenWidth / 2.f, screenHeight - 25.f);

    for(uint32_t row = 0; row < brickCountX; ++row)
        for(uint32_t column = 0; column < brickCountY; ++column)
            m_bricks.emplace_back(row, column);

    gameLoop();
}

// Both game loop and draw methods were based on two excellent articles on the topic
// https://gameprogrammingpatterns.com/game-loop.html
// https://gafferongames.com/post/fix_your_timestep/
void Arkanoid::gameLoop()
{
    // Fixed time step for updating
    const sf::Time dt = sf::seconds(1.f / 299.f);

    sf::Time accumulator;

    sf::Clock clock;

    while(m_window->isOpen() && !bGameOver)
    {
        sf::Time frameTime = clock.restart(); // How much real time passed since the last frame
        accumulator += frameTime;

        sf::Event event;
        while(m_window->pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                m_window->close();
        }

        getInput();

        while(accumulator >= dt && !bGameOver)
        {
            update(dt.asSeconds());
            accumulator -= dt;
        }

        // The leftover amount in accumulator divided by dt gives us how far into the
        // next frame we are, so we can render accordingly, resulting in smoother motion
        draw(accumulator / dt, dt.asSeconds());
    }

    if(bGameOver)
        handleGameOver();
}

void Arkanoid::getInput()
{
    using Kb = sf::Keyboard;

    if(Kb::isKeyPressed(Kb::Escape))
        m_window->close();

    if(Kb::isKeyPressed(Kb::D) && Kb::isKeyPressed(Kb::A))
    {
        m_paddle->direction = Paddle::EMD_None;
        return;
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

void Arkanoid::update(float dt)
{
    m_ball->update(dt);

    if(bGameOver) // If the ball update caused the game over in this frame, no need to proceed
        return;

    m_paddle->update(dt);

    checkCollision();

    for(auto& brick : m_bricks)
        checkCollision(brick);

    // std::remove_if moves all the elements that satisfy a certain condition (bricks that are destroyed in this case) to the
    // container's back part, and returns an iterator that points to the first destroyed brick that appears from left to right.
    // Now we can properly erase the adjacent destroyed bricks starting at the returned iterator up until the end.
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

    /*m_window->draw(*m_ball);
    m_window->draw(*m_paddle);*/

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

    bool bShouldRestart = false;

    while(m_window->isOpen() && !bShouldRestart)
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
                else
                if(event.key.code == sf::Keyboard::Enter)
                    bShouldRestart = true;
                break;
            }
        }

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
