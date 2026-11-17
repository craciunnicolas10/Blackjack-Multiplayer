#include <ui/seat_view.h>

#include <algorithm>
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
        constexpr float kChipsGap = 6.f;  // între panou și teancul de jetoane

        const sf::Color kGold(240, 210, 120);
        const sf::Color kInfoColor(220, 220, 220);
        const sf::Color kMoneyUp(130, 235, 130);
        const sf::Color kMoneyDown(245, 120, 110);

        void centerTextAt(sf::Text& text, sf::Vector2f centerTop) {
            const sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y});
            text.setPosition(centerTop);
        }

        float easeOutCubic(float t) {
            const float inverse = 1.f - t;
            return 1.f - inverse * inverse * inverse;
        }
    }  // namespace

    SeatView::SeatView(const sf::Font& font, const CardTextures& textures, const sf::String& name)
        : m_hand(textures),
          m_chips(font),
          m_panel(kPanelSize),
          m_name(font, name, kNameSize),
          m_money(font, "", kInfoSize),
          m_bet(font, "", kInfoSize) {
        m_panel.setOutlineThickness(2.f);
        refreshMoneyText();
        setBet(0);
        updateColors();
    }

    void SeatView::setPosition(sf::Vector2f centerTop) {
        m_position = centerTop;
        m_hand.setPosition(centerTop);
        m_panel.setPosition(
            {centerTop.x - kPanelSize.x / 2.f, centerTop.y + CardView::kSize.y + kPanelGap});
        // Teancul stă în dreapta panoului, cu jetonul de jos la nivelul marginii de jos.
        const sf::Vector2f panelTop = m_panel.getPosition();
        m_chips.setPosition({panelTop.x + kPanelSize.x + kChipsGap + ChipStack::kChipRadius,
                             panelTop.y + kPanelSize.y - ChipStack::kChipRadius - 4.f});
        layoutTexts();
    }

    sf::Vector2f SeatView::getPosition() const { return m_position; }

    void SeatView::setName(const sf::String& name) {
        m_name.setString(name);
        layoutTexts();
    }

    void SeatView::setMoney(int money) {
        if (!m_moneyKnown) {
            // Prima sumă (ex. banii de start) apare direct, nu numărată de la 0.
            m_moneyKnown = true;
            m_moneyFrom = m_moneyTarget = m_moneyShown = money;
            m_moneyElapsed = kMoneyAnimationDuration;
        } else {
            m_moneyFrom = m_moneyShown;
            m_moneyTarget = money;
            m_moneyElapsed = 0.f;
        }
        refreshMoneyText();
    }

    void SeatView::setBet(int bet) {
        m_bet.setString(utf8("Pariu: " + std::to_string(bet)));
        layoutTexts();
        if (!(bet == 0 && m_chips.isAnimating())) {
            m_chips.setAmount(bet);
        }
    }

    void SeatView::settleBet(ChipsTo destination, sf::Vector2f dealerPosition) {
        // Spre jucător = spre suma lui de bani din panou.
        const sf::Vector2f target =
            destination == ChipsTo::Player ? m_money.getPosition() : dealerPosition;
        m_chips.sendTo(target);
    }

    void SeatView::setActive(bool active) {
        m_active = active;
        m_pulseTime = 0.f;
        updateColors();
    }

    bool SeatView::isActive() const { return m_active; }

    HandView& SeatView::getHand() { return m_hand; }

    const HandView& SeatView::getHand() const { return m_hand; }

    bool SeatView::isAnimating() const {
        return m_hand.isAnimating() || m_chips.isAnimating() ||
               m_moneyElapsed < kMoneyAnimationDuration;
    }

    void SeatView::update(float dt) {
        m_hand.update(dt);
        m_chips.update(dt);

        if (m_moneyElapsed < kMoneyAnimationDuration) {
            m_moneyElapsed = std::min(m_moneyElapsed + dt, kMoneyAnimationDuration);
            const float t = easeOutCubic(m_moneyElapsed / kMoneyAnimationDuration);
            m_moneyShown =
                m_moneyFrom +
                static_cast<int>(std::lround(static_cast<float>(m_moneyTarget - m_moneyFrom) * t));
            refreshMoneyText();
        }

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
        m_chips.draw(target);
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
        m_bet.setFillColor(kInfoColor);
    }

    void SeatView::refreshMoneyText() {
        m_money.setString(utf8("Bani: " + std::to_string(m_moneyShown)));
        // Cât timp suma se schimbă: verde când crește, roșu când scade.
        if (m_moneyElapsed < kMoneyAnimationDuration && m_moneyTarget != m_moneyFrom) {
            m_money.setFillColor(m_moneyTarget > m_moneyFrom ? kMoneyUp : kMoneyDown);
        } else {
            m_money.setFillColor(kInfoColor);
        }
        layoutTexts();
    }
}  // namespace Frontend
