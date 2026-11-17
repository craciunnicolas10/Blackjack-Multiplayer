#pragma once

#include <array>

#include <SFML/Graphics/Texture.hpp>

namespace Frontend {
    // Până se mută Card / ERank / ESuit în Backend, rangul și culoarea sunt simple numere.
    // Rang: 2–10, 11 = jack, 12 = queen, 13 = king, 14 = ace.
    inline constexpr int kMinRank = 2;
    inline constexpr int kMaxRank = 14;

    // Culoare: în ordinea asta.
    inline constexpr int kSuitClubs = 0;
    inline constexpr int kSuitDiamonds = 1;
    inline constexpr int kSuitHearts = 2;
    inline constexpr int kSuitSpades = 3;
    inline constexpr int kSuitCount = 4;

    // Cele 52 de fețe + spatele, încărcate o singură dată din assets/cards/.
    // Obiectul e deținut de Application și ajunge la ecrane prin AppContext.
    class CardTextures {
       public:
        // Încarcă toate texturile. Pentru fiecare fișier lipsă scrie o eroare în std::cerr și
        // întoarce false la final (aplicația se oprește, ca la font).
        bool load();

        // Pentru rang sau culoare invalide întoarce spatele cărții.
        const sf::Texture& get(int rank, int suit) const;
        const sf::Texture& back() const;

       private:
        static constexpr int kRankCount = kMaxRank - kMinRank + 1;

        std::array<sf::Texture, kRankCount * kSuitCount> m_faces;
        sf::Texture m_back;
    };
}  // namespace Frontend
