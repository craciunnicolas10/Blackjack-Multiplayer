#pragma once

#include <vector>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <ui/felt_background.h>

namespace Frontend {
    // Fundalul mesei: postavul (FeltBackground), un arc auriu între dealer și jucători și,
    // de-a lungul arcului, textul „Dealerul se oprește la 17 · Blackjack plătește 3:2”.
    // Se construiește o dată (în constructorul ecranului) și se desenează primul în draw().
    class TableBackground {
       public:
        explicit TableBackground(const sf::Font& font);

        void draw(sf::RenderTarget& target) const;

       private:
        void buildArc();
        void buildArcText(const sf::Font& font);

        FeltBackground m_felt;
        sf::VertexArray m_arc;
        std::vector<sf::Text> m_arcText;  // câte o literă, rotită după arc
    };
}  // namespace Frontend
