#pragma once

#include <vector>

#include <Backend/Card.h>

namespace Backend {
    inline constexpr int kBlackjackScore = 21;

    // 2-10 dupa numar, figurile 10, asul 11.
    int CardValue(ERank rank);

    // Asul valoreaza 11, sau 1 daca altfel mana ar trece de 21.
    int ScoreHand(const std::vector<Card>& hand);

    // 21 din primele doua carti.
    bool IsBlackjack(const std::vector<Card>& hand);
}  // namespace Backend
