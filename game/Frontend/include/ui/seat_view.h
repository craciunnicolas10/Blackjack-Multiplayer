#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>

#include <ui/card_view.h>
#include <ui/chip_stack.h>
#include <ui/hand_view.h>

namespace Frontend {
    class CardTextures;

    // Locul unui jucător la masă: mâna lui, iar dedesubt un panou cu numele, banii și pariul,
    // cu teancul de jetoane al pariului lângă panou.
    // Locul jucătorului care e la rând poate fi evidențiat cu setActive(true).
    class SeatView {
       public:
        static constexpr sf::Vector2f kPanelSize{180.f, 92.f};
        static constexpr float kPanelGap = 12.f;  // între cărți și panou
        // Înălțimea totală: cărțile + spațiu + panoul.
        static constexpr float kHeight = CardView::kSize.y + kPanelGap + kPanelSize.y;
        static constexpr float kMoneyAnimationDuration = 0.7f;  // secunde

        // Unde merg jetoanele pariului la final de rundă.
        enum class ChipsTo {
            Player,  // a câștigat sau egal: pariul (și câștigul) vin la jucător
            Dealer,  // a pierdut: pariul merge la dealer
        };

        SeatView(const sf::Font& font, const CardTextures& textures, const sf::String& name);

        // `centerTop` = mijlocul de sus al locului (unde încep cărțile).
        void setPosition(sf::Vector2f centerTop);
        sf::Vector2f getPosition() const;

        void setName(const sf::String& name);
        // Suma afișată crește sau scade treptat până la noua valoare (prima dată apare direct).
        void setMoney(int money);
        // Schimbă textul și teancul de jetoane. Un pariu de 0 nu întrerupe jetoanele care
        // zboară după settleBet, doar le lasă să termine.
        void setBet(int bet);

        // Final de rundă: jetoanele pariului zboară spre jucător sau spre `dealerPosition`
        // și dispar. Textul „Pariu” rămâne până la următorul setBet.
        void settleBet(ChipsTo destination, sf::Vector2f dealerPosition);

        void setActive(bool active);
        bool isActive() const;

        HandView& getHand();
        const HandView& getHand() const;

        // True cât timp mai e ceva în mișcare: cărți, jetoane sau suma de bani.
        bool isAnimating() const;
        void update(float dt);
        void draw(sf::RenderTarget& target) const;

       private:
        void layoutTexts();
        void updateColors();
        void refreshMoneyText();

        sf::Vector2f m_position;
        HandView m_hand;
        ChipStack m_chips;
        sf::RectangleShape m_panel;
        sf::Text m_name;
        sf::Text m_money;
        sf::Text m_bet;
        bool m_active = false;
        float m_pulseTime = 0.f;

        // Animația banilor: suma afișată merge de la `m_moneyFrom` la `m_moneyTarget`.
        bool m_moneyKnown = false;
        int m_moneyFrom = 0;
        int m_moneyTarget = 0;
        int m_moneyShown = 0;
        float m_moneyElapsed = kMoneyAnimationDuration;
    };
}  // namespace Frontend
