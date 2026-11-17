#include <ui/button.h>

#include <utility>

#include <ui/crisp_text.h>
#include <ui/theme.h>

namespace Frontend {
    namespace {
        constexpr float kHintMargin = 6.f;  // între indiciu și colțul butonului
    }  // namespace

    Button::Button(const sf::Font& font, const sf::String& label, sf::Vector2f position,
                   sf::Vector2f size)
        : m_shape(size),
          m_text(font, label, Theme::kTextButton),
          m_hint(font, "", Theme::kTextTiny) {
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

    void Button::setHint(const sf::String& hint) {
        m_hint.setString(hint);
        // Aliniat la colțul din dreapta-jos.
        const sf::FloatRect bounds = m_hint.getLocalBounds();
        m_hint.setOrigin(bounds.position + bounds.size);
        m_hint.setPosition(m_shape.getPosition() + m_shape.getSize() -
                           sf::Vector2f{kHintMargin, kHintMargin});
        updateColors();
    }

    void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
            m_hovered =
                m_shape.getGlobalBounds().contains(window.mapPixelToCoords(moved->position));
        } else if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (pressed->button == sf::Mouse::Button::Left && m_enabled) {
                m_hovered =
                    m_shape.getGlobalBounds().contains(window.mapPixelToCoords(pressed->position));
                m_pressed = m_hovered;
            }
        } else if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (released->button == sf::Mouse::Button::Left) {
                const bool clicked =
                    m_pressed && m_enabled &&
                    m_shape.getGlobalBounds().contains(window.mapPixelToCoords(released->position));
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
        drawText(target, m_text);
        if (!m_hint.getString().isEmpty()) {
            drawText(target, m_hint);
        }
    }

    void Button::updateColors() {
        if (!m_enabled) {
            m_shape.setFillColor(Theme::kButtonDisabledFill);
            m_shape.setOutlineColor(Theme::kButtonDisabledOutline);
            m_text.setFillColor(Theme::kButtonDisabledText);
            m_hint.setFillColor(Theme::kButtonDisabledText);
            return;
        }

        if (m_pressed) {
            m_shape.setFillColor(Theme::kButtonPressed);
        } else if (m_hovered) {
            m_shape.setFillColor(Theme::kButtonHover);
        } else {
            m_shape.setFillColor(Theme::kButtonFill);
        }
        m_shape.setOutlineColor(Theme::kGold);
        m_text.setFillColor(Theme::kTextPrimary);
        // Indiciul e mai discret decât textul principal.
        sf::Color hint = Theme::kTextPrimary;
        hint.a = 170;
        m_hint.setFillColor(hint);
    }
}  // namespace Frontend
