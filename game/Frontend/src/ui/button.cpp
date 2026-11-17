#include <ui/button.h>

#include <utility>

namespace Frontend {
    Button::Button(const sf::Font& font, const sf::String& label, sf::Vector2f position,
                   sf::Vector2f size)
        : m_shape(size), m_text(font, label, 26) {
        m_shape.setPosition(position);
        m_shape.setOutlineThickness(2.f);

        const sf::FloatRect bounds = m_text.getLocalBounds();
        m_text.setOrigin(bounds.position + bounds.size / 2.f);
        m_text.setPosition(position + size / 2.f);

        updateColors();
    }

    void Button::setOnClick(std::function<void()> onClick) { m_onClick = std::move(onClick); }

    void Button::setEnabled(bool enabled) {
        m_enabled = enabled;
        if (!m_enabled) {
            m_pressed = false;
        }
        updateColors();
    }

    bool Button::isEnabled() const { return m_enabled; }

    void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
            m_hovered = m_shape.getGlobalBounds().contains(window.mapPixelToCoords(moved->position));
        } else if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (pressed->button == sf::Mouse::Button::Left && m_enabled) {
                m_hovered =
                    m_shape.getGlobalBounds().contains(window.mapPixelToCoords(pressed->position));
                m_pressed = m_hovered;
            }
        } else if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (released->button == sf::Mouse::Button::Left) {
                const bool clicked = m_pressed && m_enabled &&
                                     m_shape.getGlobalBounds().contains(
                                         window.mapPixelToCoords(released->position));
                m_pressed = false;
                if (clicked && m_onClick) {
                    m_onClick();
                }
            }
        }
        updateColors();
    }

    void Button::draw(sf::RenderTarget& target) const {
        target.draw(m_shape);
        target.draw(m_text);
    }

    void Button::updateColors() {
        if (!m_enabled) {
            m_shape.setFillColor(sf::Color(60, 60, 60));
            m_shape.setOutlineColor(sf::Color(90, 90, 90));
            m_text.setFillColor(sf::Color(130, 130, 130));
            return;
        }

        if (m_pressed) {
            m_shape.setFillColor(sf::Color(150, 110, 20));
        } else if (m_hovered) {
            m_shape.setFillColor(sf::Color(200, 150, 30));
        } else {
            m_shape.setFillColor(sf::Color(170, 125, 25));
        }
        m_shape.setOutlineColor(sf::Color(240, 210, 120));
        m_text.setFillColor(sf::Color::White);
    }
}  // namespace Frontend
