#pragma once

namespace Backend {
    // Ordinea și valorile sunt folosite și de Frontend (static_cast<int>), nu le schimbați.
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
