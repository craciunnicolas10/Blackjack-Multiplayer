#include <ui/seat_view.h>

#include <cmath>
#include <cstdint>
#include <string>

#include <ui/utf8.h>

namespace Frontend {
    namespace {
        constexpr unsigned kNameSize = 22;
        constexpr unsigned kInfoSize = 18;
        constexpr float kPanelPadding = 8.f;
        constexpr float kLineGap = 4.f;

        const sf::Color kGold(240, 210, 120);

        void centerTextAt(sf::Text& text, sf::Vector2f centerTop) {
            const sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y});
            text.setPosition(centerTop);
        }
    }  // namespace

    SeatView::SeatView(const sf::Font& font, const CardTextures& textures, const sf::String& name)
        : m_hand(textures),
          m_panel(kPanelSize),
          m_name(font, name, kNameSize),
          m_money(font, "", kInfoSize),
          m_bet(font, "", kInfoSize) {
        m_panel.setOutlineThickness(2.f);
        setMoney(0);
        setBet(0);
        updateColors();
    }

    void SeatView::setPosition(sf::Vector2f centerTop) {
        m_position = centerTop;
        m_hand.setPosition(centerTop);
        m_panel.setPosition(
            {centerTop.x - kPanelSize.x / 2.f, centerTop.y + CardView::kSize.y + kPanelGap});
        layoutTexts();
    }

    sf::Vector2f SeatView::getPosition() const { return m_position; }

    void SeatView::setName(const sf::String& name) {
        m_name.setString(name);
        layoutTexts();
    }

    void SeatView::setMoney(int money) {
        m_money.setString(utf8("Bani: " + std::to_string(money)));
        layoutTexts();
    }

    void SeatView::setBet(int bet) {
        m_bet.setString(utf8("Pariu: " + std::to_string(bet)));
        layoutTexts();
    }

    void SeatView::setActive(bool active) {
        m_active = active;
        m_pulseTime = 0.f;
        updateColors();
    }

    bool SeatView::isActive() const { return m_active; }

    HandView& SeatView::getHand() { return m_hand; }

    const HandView& SeatView::getHand() const { return m_hand; }

    void SeatView::update(float dt) {
        m_hand.update(dt);
        if (m_active) {
            m_pulseTime += dt;
            updateColors();
        }
    }

    void SeatView::draw(sf::RenderTarget& target) const {
        target.draw(m_panel);
        target.draw(m_name);
        target.draw(m_money);
        target.draw(m_bet);
        m_hand.draw(target);
    }

    void SeatView::layoutTexts() {
        const sf::Vector2f panelTop = m_panel.getPosition();
        const float centerX = panelTop.x + kPanelSize.x / 2.f;

        float y = panelTop.y + kPanelPadding;
        centerTextAt(m_name, {centerX, y});
        y += static_cast<float>(kNameSize) + kLineGap + 2.f;
        centerTextAt(m_money, {centerX, y});
        y += static_cast<float>(kInfoSize) + kLineGap;
        centerTextAt(m_bet, {centerX, y});
    }

    void SeatView::updateColors() {
        if (m_active) {
            // Conturul auriu „respiră” ușor, ca să se vadă al cui e rândul.
            const float pulse = (std::sin(m_pulseTime * 4.f) + 1.f) / 2.f;
            sf::Color outline = kGold;
            outline.a = static_cast<std::uint8_t>(170.f + 85.f * pulse);
            m_panel.setFillColor(sf::Color(90, 70, 15, 220));
            m_panel.setOutlineColor(outline);
            m_panel.setOutlineThickness(3.f);
            m_name.setFillColor(kGold);
        } else {
            m_panel.setFillColor(sf::Color(0, 0, 0, 110));
            m_panel.setOutlineColor(sf::Color(255, 255, 255, 50));
            m_panel.setOutlineThickness(2.f);
            m_name.setFillColor(sf::Color::White);
        }
        m_money.setFillColor(sf::Color(220, 220, 220));
        m_bet.setFillColor(sf::Color(220, 220, 220));
    }
}  // namespace Frontend
