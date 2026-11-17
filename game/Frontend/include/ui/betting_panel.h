#pragma once

#include <array>
#include <functional>
#include <vector>

#include <SFML/Graphics.hpp>

#include <ui/button.h>
#include <ui/chip_button.h>

namespace Frontend {
    // Panoul în care un jucător își alege pariul: jetoane care adună la sumă, „Șterge” și
    // „Confirmă”. Nu știe regulile: limitele și banii jucătorului vin din joc prin open().
    // Taste: Enter = Confirmă, Backspace = Șterge.
    class BettingPanel {
       public:
        static constexpr sf::Vector2f kSize{600.f, 290.f};
        static constexpr std::array<int, 3> kChipValues{10, 50, 100};

        // `position` = colțul din stânga-sus al panoului.
        BettingPanel(const sf::Font& font, sf::Vector2f position);

        // Pregătește panoul pentru un jucător. Pariul maxim e cel mai mic dintre `maxBet` și
        // banii jucătorului; `initialBet` (ex. pariul din runda trecută) e ajustat la limite.
        void open(const sf::String& playerName, int money, int minBet, int maxBet, int initialBet);
        int getBet() const;

        // Primește pariul ales când jucătorul apasă „Confirmă”.
        void setOnConfirm(std::function<void(int)> onConfirm);

        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void draw(sf::RenderTarget& target) const;

       private:
        int effectiveMax() const;
        bool canConfirm() const;
        void addChip(int value);
        void clearBet();
        void confirm();
        void refresh();

        sf::Vector2f m_position;
        sf::RectangleShape m_background;
        sf::Text m_title;
        sf::Text m_amount;
        sf::Text m_info;
        std::vector<ChipButton> m_chipButtons;
        Button m_clearButton;
        Button m_confirmButton;

        int m_money = 0;
        int m_minBet = 0;
        int m_maxBet = 0;
        int m_bet = 0;
        std::function<void(int)> m_onConfirm;
    };
}  // namespace Frontend
