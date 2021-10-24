#pragma once

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <cstdint>

class Arkanoid;

class Ball : public sf::CircleShape
{
public:

    Ball();

    void update(float dt);

    friend class Arkanoid;

private:

    void moveBall(float dt);

    float x{}, y{};

    const float radius = 16.f,
                speed  = 450.f; // pixels / s

    sf::FloatRect collider{};
    sf::Vector2f  velocity{};

    static inline bool bHitBottom{};
};


class Paddle : public sf::RectangleShape
{
public:

    Paddle();

    void update(float dt);

    friend class Arkanoid;

private:

    void movePaddle(float dt);
    void rotateAngleIndicator(float dt);

    float x{}, y{};

    const float width  = 200.f,
                height = 40.f,
                speed  = 600.f; // pixels / s

    const float angleIndicatorSpeed = 100.f;

    enum EMoveDirection : uint8_t
    {
        EMD_Right,
        EMD_Left,
        EMD_NotMoving
    };

    enum EIndicatorRotation : uint8_t
    {
        EIR_RotateRight,
        EIR_RotateLeft,
        EIR_NotRotating
    };

    EMoveDirection moveDirectionState = EMD_NotMoving;
    EIndicatorRotation indicatorRotationState = EIR_NotRotating;

    sf::FloatRect collider{};
    sf::RectangleShape angleIndicator; // Indicates the angle that the ball will be launched or reflected
};


class Brick : public sf::RectangleShape
{
public:

    Brick(uint32_t row, uint32_t column);

    friend class Arkanoid;

private:

    float x{}, y{};

    bool isDestroyed = false;

    float width  = 150.f,
          height = 40.f;

    sf::FloatRect collider{};
};
