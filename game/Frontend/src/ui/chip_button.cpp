#include <ui/chip_button.h>

#include <utility>

namespace Frontend {
    namespace {
        constexpr float kHoverScale = 1.08f;
        constexpr float kPressedScale = 0.94f;
    }  // namespace

    ChipButton::ChipButton(const sf::Font& font, int value, sf::Vector2f center, float radius)
        : m_chip(font, value, radius) {
        m_chip.setPosition(center);
        refresh();
    }

    void ChipButton::setOnClick(std::function<void()> onClick) { m_onClick = std::move(onClick); }

    void ChipButton::setEnabled(bool enabled) {
        m_enabled = enabled;
        if (!m_enabled) {
            m_pressed = false;
        }
        refresh();
    }

    bool ChipButton::isEnabled() const { return m_enabled; }

    int ChipButton::getValue() const { return m_chip.getValue(); }

    void ChipButton::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
            m_hovered = m_chip.contains(window.mapPixelToCoords(moved->position));
        } else if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (pressed->button == sf::Mouse::Button::Left && m_enabled) {
                m_hovered = m_chip.contains(window.mapPixelToCoords(pressed->position));
                m_pressed = m_hovered;
            }
        } else if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (released->button == sf::Mouse::Button::Left) {
                const bool clicked = m_pressed && m_enabled &&
                                     m_chip.contains(window.mapPixelToCoords(released->position));
                m_pressed = false;
                if (clicked && m_onClick) {
                    m_onClick();
                }
            }
        }
        refresh();
    }

    void ChipButton::draw(sf::RenderTarget& target) const { m_chip.draw(target); }

    void ChipButton::refresh() {
        m_chip.setDimmed(!m_enabled);
        if (!m_enabled) {
            m_chip.setScale(1.f);
        } else if (m_pressed) {
            m_chip.setScale(kPressedScale);
        } else if (m_hovered) {
            m_chip.setScale(kHoverScale);
        } else {
            m_chip.setScale(1.f);
        }
    }
}  // namespace Frontend
