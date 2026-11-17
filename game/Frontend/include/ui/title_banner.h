#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>

namespace Frontend {
    class CardTextures;

    // Titlul mare din meniu: textul auriu cu umbră, un evantai de două cărți (A♠, K♥) în
    // stânga și un teanc de jetoane în dreapta. Cărțile se leagănă ușor (update(dt)).
    // Ocupă cam 760 x 200 în jurul centrului dat.
    class TitleBanner {
       public:
        static constexpr unsigned kTitleSize = 80;

        TitleBanner(const sf::Font& font, const CardTextures& cards,
                    const sf::String& title = "Blackjack");

        // `center` = centrul textului titlului.
        void setPosition(sf::Vector2f center);
        sf::Vector2f getPosition() const;
        // Pentru o variantă mai mică (ex. 0,6 în lobby). 1 = mărimea normală.
        void setScale(float scale);

        void update(float dt);
        void draw(sf::RenderTarget& target) const;

       private:
        void drawCard(sf::RenderTarget& target, int rank, int suit, sf::Vector2f offset,
                      float degrees) const;
        void drawChips(sf::RenderTarget& target, sf::Vector2f offset) const;

        const sf::Font* m_font;
        const CardTextures* m_cards;
        sf::Text m_title;
        sf::Vector2f m_position;
        float m_scale = 1.f;
        float m_time = 0.f;
    };
}  // namespace Frontend
