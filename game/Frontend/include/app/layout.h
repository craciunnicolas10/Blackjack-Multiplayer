#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>

namespace Frontend {
    // Rezoluția logică: toate ecranele poziționează elementele în acest spațiu.
    inline constexpr float kLogicalWidth = 1280.f;
    inline constexpr float kLogicalHeight = 720.f;

    // View care scalează spațiul logic în fereastră, păstrând proporția (benzi negre pe margini).
    sf::View makeLetterboxView(sf::Vector2u windowSize);
}  // namespace Frontend
