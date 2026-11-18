#pragma once

#include <functional>

#include <SFML/Graphics.hpp>

#include <ui/button.h>

namespace Frontend {
    // Un mesaj peste tot ecranul (care se întunecă), cu un titlu, un text și butonul „Meniu”.
    // Folosit când jocul nu mai poate continua (ex. cineva a ieșit din jocul în rețea).
    class NoticePanel {
       public:
        explicit NoticePanel(const sf::Font& font);

        void open(const sf::String& title, const sf::String& message);
        void setOnMenu(std::function<void()> onMenu);

        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void draw(sf::RenderTarget& target) const;

       private:
        sf::RectangleShape m_dim;
        sf::RectangleShape m_background;
        sf::Text m_title;
        sf::Text m_message;
        Button m_menuButton;
        std::function<void()> m_onMenu;
    };
}  // namespace Frontend
