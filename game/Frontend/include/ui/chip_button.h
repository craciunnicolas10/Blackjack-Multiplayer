#pragma once

#include <functional>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <ui/chip_view.h>

namespace Frontend {
    // Jeton apăsabil: ca Button (hover, apăsare, dezactivare), dar click-ul contează doar
    // în interiorul cercului, nu în tot dreptunghiul din jur.
    class ChipButton {
       public:
        // `center` = centrul jetonului.
        ChipButton(const sf::Font& font, int value, sf::Vector2f center, float radius);

        void setOnClick(std::function<void()> onClick);
        void setEnabled(bool enabled);
        bool isEnabled() const;
        int getValue() const;

        // `window` e folosit pentru a transforma pixelii mouse-ului în coordonate logice.
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void draw(sf::RenderTarget& target) const;

       private:
        void refresh();

        ChipView m_chip;
        std::function<void()> m_onClick;
        bool m_enabled = true;
        bool m_hovered = false;
        bool m_pressed = false;
    };
}  // namespace Frontend
