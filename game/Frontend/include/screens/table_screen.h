#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include <app/app_context.h>
#include <screens/screen.h>
#include <ui/button.h>

namespace Frontend {
    // Masa de joc. Deocamdată arată doar layout-ul: zona dealerului, 4 locuri și butoanele.
    // Cărțile sunt dreptunghiuri gri până vine CardView (persoana C).
    class TableScreen : public Screen {
       public:
        explicit TableScreen(AppContext context);

        void handleEvent(const sf::Event& event) override;
        void draw(sf::RenderTarget& target) override;

       private:
        void drawCardPlaceholder(sf::RenderTarget& target, sf::Vector2f position,
                                 bool faceDown) const;
        void drawLabel(sf::RenderTarget& target, const sf::String& text,
                       sf::Vector2f centerTop) const;
        void goToMenu();

        AppContext m_context;
        std::vector<Button> m_actionButtons;  // Hit, Stand, Double
        std::vector<Button> m_otherButtons;   // Meniu
    };
}  // namespace Frontend
