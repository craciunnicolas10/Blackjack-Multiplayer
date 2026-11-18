#include <ui/seat_view.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

#include <SFML/Graphics/ConvexShape.hpp>

#include <ui/crisp_text.h>
#include <ui/easing.h>
#include <ui/shapes.h>
#include <ui/text_layout.h>
#include <ui/theme.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        constexpr unsigned kNameSize = Theme::kTextBody;
        constexpr unsigned kInfoSize = Theme::kTextSmall;
        constexpr float kPanelPadding = 8.f;
        constexpr float kLineGap = 4.f;
        constexpr float kChipsGap = 6.f;  // între panou și teancul de jetoane

        // Eticheta „Peste 21!”, ștampilată peste cărți.
        constexpr sf::Vector2f kBustSize{122.f, 32.f};
        constexpr float kBustTopOffset = 46.f;  // față de marginea de sus a cărților
        constexpr float kBustAngle = -8.f;      // grade
        constexpr float kBustAppearDuration = 0.3f;

        constexpr float kBustOvershoot = 2.2f;
    }  // namespace

    SeatView::SeatView(const sf::Font& font, const CardTextures& textures, const sf::String& name)
        : m_hand(textures),
          m_chips(font),
          m_panel(kPanelSize),
          m_name(font, name, kNameSize),
          m_money(font, "", kInfoSize),
          m_bet(font, "", kInfoSize),
          m_bustText(font, utf8("Peste 21!"), Theme::kTextSmall) {
        m_panel.setOutlineThickness(2.f);
        m_bustText.setStyle(sf::Text::Bold);
        m_bustText.setFillColor(Theme::kTextPrimary);
        centerOrigin(m_bustText);
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

    void SeatView::showBust() {
        if (m_bustShown) {
            return;
        }
        m_bustPending = true;
    }

    bool SeatView::isBustShown() const { return m_bustShown; }

    void SeatView::setActive(bool active) {
        m_active = active;
        m_pulseTime = 0.f;
        updateColors();
    }

    bool SeatView::isActive() const { return m_active; }

    HandView& SeatView::getHand() { return m_hand; }

    const HandView& SeatView::getHand() const { return m_hand; }

    bool SeatView::isAnimating() const {
        return m_hand.isAnimating() || m_chips.isAnimating() || m_bustPending ||
               m_moneyElapsed < kMoneyAnimationDuration;
    }

    void SeatView::update(float dt) {
        m_hand.update(dt);
        m_chips.update(dt);

        if (m_hand.getCardCount() == 0) {
            // Cărțile s-au strâns (rundă nouă): eticheta dispare odată cu ele.
            m_bustPending = false;
            m_bustShown = false;
        } else if (m_bustPending && !m_hand.isAnimating()) {
            // Abia după ce a ajuns cartea care a trecut de 21.
            m_bustPending = false;
            m_bustShown = true;
            m_bustElapsed = 0.f;
            m_hand.shake();
        } else if (m_bustShown) {
            m_bustElapsed = std::min(m_bustElapsed + dt, kBustAppearDuration);
        }

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
        drawText(target, m_name);
        drawText(target, m_money);
        drawText(target, m_bet);
        m_chips.draw(target);
        m_hand.draw(target);
        if (m_bustShown) {
            drawBust(target);
        }
    }

    void SeatView::drawBust(sf::RenderTarget& target) const {
        const sf::FloatRect hand = m_hand.getBounds();
        const sf::Vector2f center{hand.position.x + hand.size.x / 2.f,
                                  hand.position.y + kBustTopOffset};
        const float t = m_bustElapsed / kBustAppearDuration;
        const float scale = std::max(easeOutBack(t, kBustOvershoot), 0.f);
        if (scale <= 0.f) {
            return;
        }

        sf::ConvexShape tag = makeRoundedRect(kBustSize, 8.f);
        tag.setPosition(center);
        tag.setRotation(sf::degrees(kBustAngle));
        tag.setScale({scale, scale});
        tag.setFillColor(Theme::kDanger);
        tag.setOutlineColor(Theme::kTextPrimary);
        tag.setOutlineThickness(2.f);

        sf::ConvexShape shadow = tag;
        shadow.move({2.f, 4.f});
        shadow.setFillColor(Theme::kShadow);
        shadow.setOutlineThickness(0.f);
        target.draw(shadow);
        target.draw(tag);

        sf::Text text = m_bustText;
        text.setPosition(center);
        text.setRotation(sf::degrees(kBustAngle));
        text.setScale({scale, scale});
        drawText(target, text);
    }

    void SeatView::layoutTexts() {
        const sf::Vector2f panelTop = m_panel.getPosition();
        const float centerX = panelTop.x + kPanelSize.x / 2.f;

        float y = panelTop.y + kPanelPadding;
        centerTextTopAt(m_name, {centerX, y});
        y += static_cast<float>(kNameSize) + kLineGap + 2.f;
        centerTextTopAt(m_money, {centerX, y});
        y += static_cast<float>(kInfoSize) + kLineGap;
        centerTextTopAt(m_bet, {centerX, y});
    }

    void SeatView::updateColors() {
        if (m_active) {
            // Conturul auriu „respiră” ușor, ca să se vadă al cui e rândul.
            const float pulse = (std::sin(m_pulseTime * 4.f) + 1.f) / 2.f;
            sf::Color outline = Theme::kGold;
            outline.a = static_cast<std::uint8_t>(170.f + 85.f * pulse);
            m_panel.setFillColor(Theme::kSeatActiveFill);
            m_panel.setOutlineColor(outline);
            m_panel.setOutlineThickness(3.f);
            m_name.setFillColor(Theme::kGold);
        } else {
            m_panel.setFillColor(Theme::kSeatFill);
            m_panel.setOutlineColor(Theme::kSeatOutline);
            m_panel.setOutlineThickness(2.f);
            m_name.setFillColor(Theme::kTextPrimary);
        }
        m_bet.setFillColor(Theme::kTextSecondary);
    }

    void SeatView::refreshMoneyText() {
        m_money.setString(utf8("Bani: " + std::to_string(m_moneyShown)));
        // Cât timp suma se schimbă: verde când crește, roșu când scade.
        if (m_moneyElapsed < kMoneyAnimationDuration && m_moneyTarget != m_moneyFrom) {
            m_money.setFillColor(m_moneyTarget > m_moneyFrom ? Theme::kPositive : Theme::kNegative);
        } else {
            m_money.setFillColor(Theme::kTextSecondary);
        }
        layoutTexts();
    }
}  // namespace Frontend
