#pragma once

#include <SFML/System/String.hpp>

namespace Frontend {
    // Textul fără spațiile de la început și de la sfârșit (ex. un nume scris de jucător).
    sf::String trimmed(const sf::String& text);
}  // namespace Frontend
