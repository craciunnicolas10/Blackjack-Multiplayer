#include <ui/result_badge.h>

#include <algorithm>
#include <cstdint>

#include <SFML/Graphics/ConvexShape.hpp>

#include <audio/sound_player.h>
#include <ui/crisp_text.h>
#include <ui/easing.h>
#include <ui/shapes.h>
#include <ui/text_layout.h>
#include <ui/theme.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        constexpr float kCornerRadius = 10.f;

        constexpr float kAppearOvershoot = 1.9f;

        sf::Color withAlpha(sf::Color color, float alpha) {
            color.a = static_cast<std::uint8_t>(static_cast<float>(color.a) * alpha);
            return color;
        }
    }  // namespace

    ResultBadge::ResultBadge(const sf::Font& font, RoundResult result)
        : m_result(result),
          m_text(font, utf8(roundResultLabel(result)), 20),
          m_color(roundResultColor(result)) {
        m_text.setStyle(sf::Text::Bold);
        centerOrigin(m_text);
    }

    RoundResult ResultBadge::getResult() const { return m_result; }

    void ResultBadge::setPosition(sf::Vector2f center) { m_position = center; }

    sf::Vector2f ResultBadge::getPosition() const { return m_position; }

    void ResultBadge::restart() {
        m_elapsed = 0.f;
        m_started = false;
    }

    bool ResultBadge::isAnimating() const { return !m_started || m_elapsed < kAppearDuration; }

    void ResultBadge::update(float dt) {
        if (!m_started) {
            m_started = true;
            if (m_result == RoundResult::Win || m_result == RoundResult::Blackjack) {
                playSound(SoundId::Win);
            }
            return;  // primul frame: eticheta apare de la mărimea 0
        }
        m_elapsed = std::min(m_elapsed + dt, kAppearDuration);
    }

    void ResultBadge::draw(sf::RenderTarget& target) const {
        const float t = std::clamp(m_elapsed / kAppearDuration, 0.f, 1.f);
        const float scale = std::max(easeOutBack(t, kAppearOvershoot), 0.f);
        const float alpha = std::min(t * 3.f, 1.f);
        if (scale <= 0.f) {
            return;
        }

        sf::ConvexShape badge = makeRoundedRect(kSize, kCornerRadius);
        badge.setPosition(m_position);
        badge.setScale({scale, scale});
        badge.setFillColor(withAlpha(m_color, alpha));
        // Blackjack primește contur auriu; restul, alb.
        const sf::Color outline =
            m_result == RoundResult::Blackjack ? Theme::kGoldBright : Theme::kTextPrimary;
        badge.setOutlineColor(withAlpha(outline, alpha));
        badge.setOutlineThickness(2.f);

        // Umbra, puțin mai jos.
        sf::ConvexShape shadow = badge;
        shadow.setPosition(m_position + sf::Vector2f{0.f, 3.f});
        shadow.setFillColor(withAlpha(Theme::kShadow, alpha));
        shadow.setOutlineThickness(0.f);
        target.draw(shadow);
        target.draw(badge);

        sf::Text text = m_text;
        text.setPosition(m_position);
        text.setScale({scale, scale});
        text.setFillColor(withAlpha(Theme::kTextPrimary, alpha));
        drawText(target, text);
    }
}  // namespace Frontend
