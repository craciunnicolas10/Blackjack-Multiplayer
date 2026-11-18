#pragma once

#include <SFML/Graphics/Color.hpp>

#include <Backend/GameEnums.h>

namespace Frontend {
    // Cum arată pe ecran rezultatul unui jucător (ERoundResult vine din Backend).
    // Textul afișat („Câștig”, „Pierdere”, „Egal”, „Blackjack!”), în UTF-8: treceți-l prin
    // utf8() înainte de sf::Text.
    const char* roundResultLabel(Backend::ERoundResult result);
    // Culoarea folosită pentru rezultat (eticheta de pe masă, ecranul de rezultat).
    sf::Color roundResultColor(Backend::ERoundResult result);
}  // namespace Frontend
