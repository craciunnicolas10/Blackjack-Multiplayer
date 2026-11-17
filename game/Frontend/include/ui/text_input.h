#pragma once

#include <cstddef>
#include <functional>

#include <SFML/Graphics.hpp>

namespace Frontend {
    // Câmp de text pe un singur rând: click pentru focus, scriere, Backspace, Enter pentru trimitere.
    class TextInput {
       public:
        TextInput(const sf::Font& font, sf::Vector2f position, sf::Vector2f size,
                  std::size_t maxLength);

        void setPlaceholder(const sf::String& placeholder);
        void setOnSubmit(std::function<void()> onSubmit);

        void setFocused(bool focused);
        bool isFocused() const;

        const sf::String& getText() const;
        void clear();

        // `window` e folosit pentru a transforma pixelii mouse-ului în coordonate logice.
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
        void update(float dt);
        void draw(sf::RenderTarget& target) const;

       private:
        void refreshText();

        sf::RectangleShape m_box;
        sf::Text m_text;
        sf::Text m_placeholder;
        sf::RectangleShape m_caret;
        sf::String m_value;
        std::size_t m_maxLength;
        std::function<void()> m_onSubmit;
        bool m_focused = false;
        float m_blinkTimer = 0.f;
    };
}  // namespace Frontend
