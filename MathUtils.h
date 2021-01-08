#include <cmath>

struct Math
{
    static sf::Vector2f GetUnitVector(sf::Vector2f vec)
    {
        float length = std::sqrt(vec.x * vec.x + vec.y * vec.y);
        return vec /= length;
    }
};
