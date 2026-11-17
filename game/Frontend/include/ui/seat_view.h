#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>

#include <ui/card_view.h>
#include <ui/hand_view.h>

namespace Frontend {
    class CardTextures;

    // Locul unui jucător la masă: mâna lui, iar dedesubt un panou cu numele, banii și pariul.
    // Locul jucătorului care e la rând poate fi evidențiat cu setActive(true).
    class SeatView {
       public:
        static constexpr sf::Vector2f kPanelSize{180.f, 92.f};
        static constexpr float kPanelGap = 12.f;  // între cărți și panou
        // Înălțimea totală: cărțile + spațiu + panoul.
        static constexpr float kHeight = CardView::kSize.y + kPanelGap + kPanelSize.y;

        SeatView(const sf::Font& font, const CardTextures& textures, const sf::String& name);

        // `centerTop` = mijlocul de sus al locului (unde încep cărțile).
        void setPosition(sf::Vector2f centerTop);
        sf::Vector2f getPosition() const;

        void setName(const sf::String& name);
        void setMoney(int money);
        void setBet(int bet);

        void setActive(bool active);
        bool isActive() const;

        HandView& getHand();
        const HandView& getHand() const;

        void update(float dt);
        void draw(sf::RenderTarget& target) const;

       private:
        void layoutTexts();
        void updateColors();

        sf::Vector2f m_position;
        HandView m_hand;
        sf::RectangleShape m_panel;
        sf::Text m_name;
        sf::Text m_money;
        sf::Text m_bet;
        bool m_active = false;
        float m_pulseTime = 0.f;
    };
}  // namespace Frontend
