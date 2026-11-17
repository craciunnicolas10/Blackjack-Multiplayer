#pragma once

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/System/Vector2.hpp>

namespace Frontend {
    // Dreptunghi cu colțuri rotunjite, cu originea în centru (setPosition = centrul).
    sf::ConvexShape makeRoundedRect(sf::Vector2f size, float cornerRadius, int pointsPerCorner = 6);
}  // namespace Frontend
