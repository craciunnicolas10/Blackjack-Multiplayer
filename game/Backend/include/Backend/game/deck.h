#pragma once

#include <random>
#include <vector>

#include <Backend/Card.h>
#include <Backend/IDeck.h>

namespace Backend {
    // Un pachet de 52 de carti. Shuffle() il reface complet si il amesteca; daca se termina
    // in timpul rundei, se reface singur.
    class Deck : public IDeck {
       public:
        Deck();

        void Shuffle() override;
        Card Draw() override;

       private:
        std::vector<Card> m_cards;
        std::mt19937 m_rng;
    };
}  // namespace Backend
