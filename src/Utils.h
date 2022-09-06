#pragma once

#include <cmath>
#include <iostream>
#include <filesystem>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

namespace fs = std::filesystem;

struct Math
{
    static sf::Vector2f GetUnitVector(sf::Vector2f vec)
    {
        float length = std::sqrt(vec.x * vec.x + vec.y * vec.y);
        return vec /= length;
    }
};

inline fs::path pathToFile(const char* fileName)
{
    auto currentPath = fs::current_path();
    auto filePath = currentPath / fileName;
    int max_n_iter = 10;

    for(int i = 0; i < max_n_iter; ++i)
    {
        // We don't look into sub folders while going back towards the root
        if(fs::exists(filePath))
            return filePath;
        else
        {
            currentPath = currentPath.parent_path();
            filePath = currentPath / fileName;
        }
    }
    return {};
}

struct TextHandler
{
    TextHandler()
    {
        auto filePath = pathToFile("bahnschrift.ttf");

        if(filePath.empty())
            throw "Could not find font file!";

        if(!font.loadFromFile(filePath.string().c_str()))
            throw "Not able to load font from file!";

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