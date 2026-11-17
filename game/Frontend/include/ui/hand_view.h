#pragma once

#include <cstddef>
#include <vector>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

#include <ui/card_view.h>

namespace Frontend {
    class CardTextures;

    // Cărțile dintr-o mână, suprapuse: fiecare e mutată cu kCardOffset la dreapta față de cea
    // dinainte. Mâna e centrată orizontal pe poziția dată, așa că la fiecare carte nouă cele
    // existente alunecă puțin spre stânga.
    class HandView {
       public:
        static constexpr float kCardOffset = 40.f;

        explicit HandView(const CardTextures& textures);

        // `centerTop` = mijlocul marginii de sus a mâinii.
        void setPosition(sf::Vector2f centerTop);
        sf::Vector2f getPosition() const;

        // Adaugă instant, fără animație.
        void addCard(int rank, int suit, bool faceUp = true);
        void addHiddenCard();

        // Adaugă cu animație: cartea pleacă din `from` (colțul stânga-sus al pachetului) și
        // alunecă la locul ei. `delay` = după cât timp pleacă (pentru împărțit pe rând).
        void dealCard(int rank, int suit, bool faceUp, sf::Vector2f from, float delay = 0.f);
        void dealHiddenCard(sf::Vector2f from, float delay = 0.f);

        // Află valoarea cărții `index` și o întoarce cu fața în sus, animat
        // (ex. cartea ascunsă a dealerului la sfârșitul rundei).
        void revealCard(std::size_t index, int rank, int suit);

        void clear();
        std::size_t getCardCount() const;
        const CardView& getCard(std::size_t index) const;
        CardView& getCard(std::size_t index);

        // Dreptunghiul ocupat de cărți după ce își termină animațiile.
        sf::FloatRect getBounds() const;

        bool isAnimating() const;
        void update(float dt);
        void draw(sf::RenderTarget& target) const;

       private:
        sf::Vector2f slotPosition(std::size_t index) const;
        void placeCard(CardView card, sf::Vector2f from, float delay);
        // Mută cărțile la locurile lor (după ce s-a schimbat numărul sau poziția mâinii).
        void relayout(bool animate);

        const CardTextures* m_textures;
        sf::Vector2f m_position;
        std::vector<CardView> m_cards;
    };
}  // namespace Frontend
