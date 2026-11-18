#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

namespace Frontend {
    // Înălțimea unui rând din lista de jucători (cu tot cu spațiul de sub el).
    inline constexpr float kPlayerRowHeight = 64.f;

    // Lista de jucători din lobby: câte un rând numerotat („1. Ana”) pentru fiecare text,
    // începând din `topLeft`, pe lățimea `width`.
    void drawPlayerRows(sf::RenderTarget& target, const sf::Font& font,
                        const std::vector<sf::String>& names, sf::Vector2f topLeft, float width);
}  // namespace Frontend
