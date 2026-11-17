#pragma once

#include <vector>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace Frontend {
    // Fundalul mesei, pe tot ecranul logic 1280x720: postav verde cu gradient (mai deschis la
    // mijloc) și o textură fină, un arc auriu între dealer și jucători și, de-a lungul arcului,
    // textul „Dealer stands on 17 · Blackjack plătește 3:2”.
    // Se construiește o dată (în constructorul ecranului) și se desenează primul în draw().
    class TableBackground {
       public:
        explicit TableBackground(const sf::Font& font);

        void draw(sf::RenderTarget& target) const;

       private:
        void buildFelt();
        void buildGrain();
        void buildArc();
        void buildArcText(const sf::Font& font);

        sf::VertexArray m_felt;
        sf::Texture m_grain;
        sf::VertexArray m_arc;
        std::vector<sf::Text> m_arcText;  // câte o literă, rotită după arc
    };
}  // namespace Frontend
