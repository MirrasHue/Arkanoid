#pragma once

#include <cmath>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

struct Math
{
    static sf::Vector2f GetUnitVector(sf::Vector2f vec)
    {
        float length = std::sqrt(vec.x * vec.x + vec.y * vec.y);
        return vec /= length;
    }
};

struct TextHandler
{
    TextHandler()
    {
        if(!font.loadFromFile("../bahnschrift.ttf"))
        {
            //std::cerr<<"Error when loading font\n";
            return;
        }
        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
        text.setPosition({20, 10});
    }

    void setText(const sf::String& string)
    {
        text.setString(string);
    }

    void setPosition(const sf::Vector2f& position)
    {
        text.setPosition(position);
    }

    sf::Font font;
    sf::Text text;
};