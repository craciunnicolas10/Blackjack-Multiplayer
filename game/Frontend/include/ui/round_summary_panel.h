#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

#include <game/round_result.h>
#include <ui/button.h>

namespace Frontend {
    // Un rând din rezumatul rundei: cum a ieșit jucătorul și cu câți bani a rămas.
    struct RoundSummaryRow {
        sf::String name;
        RoundResult result;
        int net;    // cât a câștigat (+) sau a pierdut (-) în runda asta
        int money;  // banii după rundă
    };

    // Ecranul de rezultat: un panou peste masă (care se întunecă) cu rezultatul fiecărui
    // jucător, scorul dealerului și butoanele „Runda următoare” și „Meniu”.
    // Taste: Enter = Runda următoare.
    class RoundSummaryPanel {
       public:
        static constexpr float kWidth = 680.f;
        static constexpr float kRowHeight = 40.f;

        // Mărimea panoului depinde de câți jucători sunt la masă, deci se știe de la început.
        RoundSummaryPanel(const sf::Font& font, std::size_t rowCount);

        // `dealerScore` = scorul final al dealerului; nullopt = rândul dealerului nu apare.
        void open(const std::vector<RoundSummaryRow>& rows, std::optional<int> dealerScore);

        void setOnNextRound(std::function<void()> onNextRound);
        void setOnMenu(std::function<void()> onMenu);

        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void draw(sf::RenderTarget& target) const;

       private:
        sf::Vector2f m_position;
        sf::Vector2f m_size;
        std::size_t m_rowCount;
        const sf::Font* m_font;

        sf::RectangleShape m_dim;
        sf::RectangleShape m_background;
        sf::Text m_title;
        sf::Text m_dealerLine;
        std::vector<sf::Text> m_cells;  // câte 4 pe rând: nume, rezultat, câștig, bani
        Button m_nextButton;
        Button m_menuButton;

        std::function<void()> m_onNextRound;
        std::function<void()> m_onMenu;
    };
}  // namespace Frontend
