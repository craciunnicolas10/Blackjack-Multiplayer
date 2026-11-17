#include <ui/text_input.h>

#include <cmath>
#include <utility>

#include <ui/crisp_text.h>

namespace Frontend {
    namespace {
        constexpr unsigned kCharacterSize = 26;
        constexpr float kPadding = 14.f;
        constexpr float kBlinkPeriod = 1.f;  // secunde: jumătate vizibil, jumătate ascuns
    }  // namespace

    TextInput::TextInput(const sf::Font& font, sf::Vector2f position, sf::Vector2f size,
                         std::size_t maxLength)
        : m_box(size),
          m_text(font, "", kCharacterSize),
          m_placeholder(font, "", kCharacterSize),
          m_caret({2.f, static_cast<float>(kCharacterSize)}),
          m_maxLength(maxLength) {
        m_box.setPosition(position);
        m_box.setFillColor(sf::Color(245, 245, 235));
        m_box.setOutlineThickness(2.f);

        // Centrare verticală după un text de referință, ca rândul să nu sară când scrii.
        const sf::Text reference(font, "Ag", kCharacterSize);
        const sf::FloatRect bounds = reference.getLocalBounds();
        const float originY = bounds.position.y + bounds.size.y / 2.f;
        const sf::Vector2f textPosition{position.x + kPadding, position.y + size.y / 2.f};

        m_text.setOrigin({0.f, originY});
        m_text.setPosition(textPosition);
        m_text.setFillColor(sf::Color(20, 20, 20));

        m_placeholder.setOrigin({0.f, originY});
        m_placeholder.setPosition(textPosition);
        m_placeholder.setFillColor(sf::Color(140, 140, 140));

        m_caret.setFillColor(sf::Color(20, 20, 20));
        m_caret.setOrigin({0.f, kCharacterSize / 2.f});

        setFocused(false);
        refreshText();
    }

    void TextInput::setPlaceholder(const sf::String& placeholder) {
        m_placeholder.setString(placeholder);
    }

    void TextInput::setOnSubmit(std::function<void()> onSubmit) {
        m_onSubmit = std::move(onSubmit);
    }

    void TextInput::setFocused(bool focused) {
        m_focused = focused;
        m_blinkTimer = 0.f;
        m_box.setOutlineColor(m_focused ? sf::Color(240, 210, 120) : sf::Color(90, 90, 90));
    }

    bool TextInput::isFocused() const { return m_focused; }

    const sf::String& TextInput::getText() const { return m_value; }

    void TextInput::clear() {
        m_value.clear();
        refreshText();
    }

    void TextInput::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (pressed->button == sf::Mouse::Button::Left) {
                setFocused(
                    m_box.getGlobalBounds().contains(window.mapPixelToCoords(pressed->position)));
            }
            return;
        }

        if (!m_focused) {
            return;
        }

        if (const auto* entered = event.getIf<sf::Event::TextEntered>()) {
            // Sub 32 sunt caractere de control (Backspace, Enter, Tab); le tratăm la KeyPressed.
            if (entered->unicode >= 32 && entered->unicode != 127 &&
                m_value.getSize() < m_maxLength) {
                m_value += sf::String(entered->unicode);
                refreshText();
            }
        } else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Backspace && !m_value.isEmpty()) {
                m_value.erase(m_value.getSize() - 1);
                refreshText();
            } else if (key->code == sf::Keyboard::Key::Enter && m_onSubmit) {
                m_onSubmit();
            }
        }
    }

    void TextInput::update(float dt) { m_blinkTimer = std::fmod(m_blinkTimer + dt, kBlinkPeriod); }

    void TextInput::draw(sf::RenderTarget& target) const {
        target.draw(m_box);
        if (m_value.isEmpty() && !m_focused) {
            drawText(target, m_placeholder);
        } else {
            drawText(target, m_text);
        }

        if (m_focused && m_blinkTimer < kBlinkPeriod / 2.f) {
            target.draw(m_caret);
        }
    }

    void TextInput::refreshText() {
        m_text.setString(m_value);
        m_blinkTimer = 0.f;  // cursorul rămâne vizibil cât timp scrii

        const sf::Vector2f end = m_text.findCharacterPos(m_value.getSize());
        m_caret.setPosition({end.x + 1.f, m_box.getPosition().y + m_box.getSize().y / 2.f});
    }
}  // namespace Frontend
