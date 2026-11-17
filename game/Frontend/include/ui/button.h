#pragma once

#include <functional>

#include <SFML/Graphics.hpp>

namespace Frontend {
    // Buton simplu: dreptunghi + text centrat, cu hover, apăsare și stare dezactivată.
    class Button {
       public:
        Button(const sf::Font& font, const sf::String& label, sf::Vector2f position,
               sf::Vector2f size);

        void setOnClick(std::function<void()> onClick);
        void setEnabled(bool enabled);
        bool isEnabled() const;

        // Un indiciu mic în colțul din dreapta-jos, de obicei tasta care face același lucru
        // (ex. setHint("H") pe „Hit”). Un text gol îl ascunde.
        void setHint(const sf::String& hint);

        // `window` e folosit pentru a transforma pixelii mouse-ului în coordonate logice.
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void draw(sf::RenderTarget& target) const;

       private:
        void updateColors();

        sf::RectangleShape m_shape;
        sf::Text m_text;
        sf::Text m_hint;
        std::function<void()> m_onClick;
        bool m_enabled = true;
        bool m_hovered = false;
        bool m_pressed = false;
    };
}  // namespace Frontend
