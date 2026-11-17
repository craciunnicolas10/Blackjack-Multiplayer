#pragma once

#include <SFML/Graphics/Color.hpp>

namespace Frontend {
    // Rezultatul unui jucător la sfârșitul rundei. Până se mută ERoundResult în Backend.
    enum class RoundResult { Win, Lose, Push, Blackjack };

    // Textul afișat („Câștig”, „Pierdere”, „Egal”, „Blackjack!”), în UTF-8: treceți-l prin
    // utf8() înainte de sf::Text.
    const char* roundResultLabel(RoundResult result);
    // Culoarea folosită pentru rezultat (eticheta de pe masă, ecranul de rezultat).
    sf::Color roundResultColor(RoundResult result);
}  // namespace Frontend
