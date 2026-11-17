#include <ui/result_badge.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>

#include <SFML/Graphics/ConvexShape.hpp>

#include <audio/sound_player.h>
#include <screens/table_screen.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        constexpr float kCornerRadius = 10.f;
        constexpr int kCornerPoints = 6;  // puncte pe fiecare colț rotunjit

        const char* labelFor(RoundResult result) {
            switch (result) {
                case RoundResult::Win:
                    return "Câștig";
                case RoundResult::Lose:
                    return "Pierdere";
                case RoundResult::Push:
                    return "Egal";
                case RoundResult::Blackjack:
                    return "Blackjack!";
            }
            return "";
        }

        sf::Color colorFor(RoundResult result) {
            switch (result) {
                case RoundResult::Win:
                    return sf::Color(40, 140, 60);
                case RoundResult::Lose:
                    return sf::Color(170, 40, 40);
                case RoundResult::Push:
                    return sf::Color(100, 100, 100);
                case RoundResult::Blackjack:
                    return sf::Color(190, 140, 20);
            }
            return sf::Color::Black;
        }

        // Depășește puțin 1 și revine, ca o etichetă care „sare” la locul ei.
        float easeOutBack(float t) {
            constexpr float kOvershoot = 1.9f;
            const float u = t - 1.f;
            return 1.f + (kOvershoot + 1.f) * u * u * u + kOvershoot * u * u;
        }

        // Dreptunghi cu colțuri rotunjite, centrat în (0, 0).
        sf::ConvexShape roundedRect(sf::Vector2f size, float radius) {
            sf::ConvexShape shape(4 * kCornerPoints);
            const sf::Vector2f half = size / 2.f;
            const sf::Vector2f centers[4] = {{half.x - radius, -half.y + radius},
                                             {half.x - radius, half.y - radius},
                                             {-half.x + radius, half.y - radius},
                                             {-half.x + radius, -half.y + radius}};
            std::size_t index = 0;
            for (int corner = 0; corner < 4; ++corner) {
                // Fiecare colț acoperă un sfert de cerc, în ordinea acelor de ceasornic.
                const float start = -std::numbers::pi_v<float> / 2.f +
                                    static_cast<float>(corner) * std::numbers::pi_v<float> / 2.f;
                for (int i = 0; i < kCornerPoints; ++i) {
                    const float angle = start + (std::numbers::pi_v<float> / 2.f) *
                                                    static_cast<float>(i) /
                                                    static_cast<float>(kCornerPoints - 1);
                    shape.setPoint(
                        index++,
                        centers[corner] + sf::Vector2f{std::cos(angle), std::sin(angle)} * radius);
                }
            }
            return shape;
        }

        sf::Color withAlpha(sf::Color color, float alpha) {
            color.a = static_cast<std::uint8_t>(static_cast<float>(color.a) * alpha);
            return color;
        }
    }  // namespace

    ResultBadge::ResultBadge(const sf::Font& font, RoundResult result)
        : m_result(result), m_text(font, utf8(labelFor(result)), 20), m_color(colorFor(result)) {
        m_text.setStyle(sf::Text::Bold);
        const sf::FloatRect bounds = m_text.getLocalBounds();
        m_text.setOrigin(bounds.position + bounds.size / 2.f);
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
        const float scale = std::max(easeOutBack(t), 0.f);
        const float alpha = std::min(t * 3.f, 1.f);
        if (scale <= 0.f) {
            return;
        }

        sf::ConvexShape badge = roundedRect(kSize, kCornerRadius);
        badge.setPosition(m_position);
        badge.setScale({scale, scale});
        badge.setFillColor(withAlpha(m_color, alpha));
        // Blackjack primește contur auriu; restul, alb.
        const sf::Color outline =
            m_result == RoundResult::Blackjack ? sf::Color(255, 225, 130) : sf::Color::White;
        badge.setOutlineColor(withAlpha(outline, alpha));
        badge.setOutlineThickness(2.f);

        // Umbra, puțin mai jos.
        sf::ConvexShape shadow = badge;
        shadow.setPosition(m_position + sf::Vector2f{0.f, 3.f});
        shadow.setFillColor(withAlpha(sf::Color(0, 0, 0, 90), alpha));
        shadow.setOutlineThickness(0.f);
        target.draw(shadow);
        target.draw(badge);

        sf::Text text = m_text;
        text.setPosition(m_position);
        text.setScale({scale, scale});
        text.setFillColor(withAlpha(sf::Color::White, alpha));
        target.draw(text);
    }
}  // namespace Frontend
