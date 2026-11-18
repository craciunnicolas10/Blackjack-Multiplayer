#pragma once

namespace Backend {
    // Ordinea si valorile sunt folosite si de Frontend (static_cast<int>), nu le schimbati.
    enum class ESuit {
        Clubs,
        Diamonds,
        Hearts,
        Spades
    };

    enum class ERank {
        Two = 2,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        Ten,
        Jack,
        Queen,
        King,
        Ace
    };

    struct Card {
        ERank rank;
        ESuit suit;
    };
}  // namespace Backend
