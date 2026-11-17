#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include <app/app_context.h>
#include <screens/screen.h>
#include <ui/button.h>

namespace Frontend {
    // Masa de joc. Deocamdată arată doar layout-ul: zona dealerului, câte un loc pentru fiecare
    // jucător din lobby și butoanele. Cărțile sunt de test până vine jocul real.
    class TableScreen : public Screen {
       public:
        TableScreen(AppContext context, std::vector<sf::String> playerNames);

        void handleEvent(const sf::Event& event) override;
        void draw(sf::RenderTarget& target) override;

       private:
        sf::Vector2f seatPosition(std::size_t seat) const;
        void drawCard(sf::RenderTarget& target, sf::Vector2f position, int rank, int suit,
                      bool faceUp) const;
        void drawLabel(sf::RenderTarget& target, const sf::String& text,
                       sf::Vector2f centerTop) const;
        void goToMenu();

        AppContext m_context;
        std::vector<sf::String> m_playerNames;
        std::vector<Button> m_actionButtons;  // Hit, Stand, Double
        std::vector<Button> m_otherButtons;   // Meniu
    };
}  // namespace Frontend
