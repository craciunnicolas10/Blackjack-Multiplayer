#include <ui/chip_view.h>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

namespace Frontend {
    namespace {
        constexpr int kStripeCount = 8;  // dungile de pe margine

        struct ChipColors {
            sf::Color base;
            sf::Color stripe;
            sf::Color inner;
        };

        ChipColors colorsFor(int value) {
            switch (value) {
                case 10:
                    return {sf::Color(30, 90, 200), sf::Color(235, 235, 235),
                            sf::Color(55, 115, 220)};
                case 50:
                    return {sf::Color(185, 30, 40), sf::Color(235, 235, 235),
                            sf::Color(210, 55, 60)};
                case 100:
                    return {sf::Color(25, 25, 25), sf::Color(240, 210, 120), sf::Color(50, 50, 50)};
                default:
                    return {sf::Color(30, 130, 70), sf::Color(235, 235, 235),
                            sf::Color(50, 155, 90)};
            }
        }
    }  // namespace

    ChipView::ChipView(const sf::Font& font, int value, float radius)
        : m_font(&font), m_value(value), m_radius(radius) {}

    void ChipView::setPosition(sf::Vector2f center) { m_position = center; }

    sf::Vector2f ChipView::getPosition() const { return m_position; }

    float ChipView::getRadius() const { return m_radius; }

    int ChipView::getValue() const { return m_value; }

    void ChipView::setScale(float scale) { m_scale = scale; }

    void ChipView::setAlpha(std::uint8_t alpha) { m_alpha = alpha; }

    void ChipView::setDimmed(bool dimmed) { m_dimmed = dimmed; }

    bool ChipView::contains(sf::Vector2f point) const {
        const sf::Vector2f delta = point - m_position;
        return delta.x * delta.x + delta.y * delta.y <= m_radius * m_radius;
    }

    void ChipView::draw(sf::RenderTarget& target) const {
        const float radius = m_radius * m_scale;
        const ChipColors colors = colorsFor(m_value);

        // Umbra, puțin mai jos, ca jetonul să pară așezat pe masă.
        sf::CircleShape shadow(radius);
        shadow.setOrigin({radius, radius});
        shadow.setPosition(m_position + sf::Vector2f{0.f, radius * 0.12f});
        shadow.setFillColor(adjust(sf::Color(0, 0, 0, 90)));
        target.draw(shadow);

        sf::CircleShape base(radius);
        base.setOrigin({radius, radius});
        base.setPosition(m_position);
        base.setFillColor(adjust(colors.base));
        target.draw(base);

        // Dungile de pe margine.
        const sf::Vector2f stripeSize{radius * 0.36f, radius * 0.24f};
        for (int i = 0; i < kStripeCount; ++i) {
            const float angle = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) /
                                static_cast<float>(kStripeCount);
            sf::RectangleShape stripe(stripeSize);
            stripe.setOrigin({stripeSize.x, stripeSize.y / 2.f});
            stripe.setPosition(m_position +
                               sf::Vector2f{std::cos(angle), std::sin(angle)} * radius);
            stripe.setRotation(sf::radians(angle));
            stripe.setFillColor(adjust(colors.stripe));
            target.draw(stripe);
        }

        // Cercul din mijloc, cu valoarea.
        const float innerRadius = radius * 0.64f;
        sf::CircleShape inner(innerRadius);
        inner.setOrigin({innerRadius, innerRadius});
        inner.setPosition(m_position);
        inner.setFillColor(adjust(colors.inner));
        inner.setOutlineColor(adjust(colors.stripe));
        inner.setOutlineThickness(std::max(1.f, radius * 0.06f));
        target.draw(inner);

        const std::string label = std::to_string(m_value);
        const float sizeFactor = label.size() >= 3 ? 0.46f : 0.6f;
        const auto characterSize = static_cast<unsigned>(std::lround(radius * sizeFactor * 1.4f));
        sf::Text text(*m_font, label, characterSize);
        text.setStyle(sf::Text::Bold);
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.position + bounds.size / 2.f);
        text.setPosition(m_position);
        text.setFillColor(adjust(sf::Color::White));
        target.draw(text);
    }

    sf::Color ChipView::adjust(sf::Color color) const {
        if (m_dimmed) {
            // Spre gri, ca butoanele dezactivate.
            const auto grey = static_cast<std::uint8_t>((color.r + color.g + color.b) / 3 / 2 + 40);
            color = sf::Color(grey, grey, grey, color.a);
        }
        color.a = static_cast<std::uint8_t>(color.a * m_alpha / 255);
        return color;
    }
}  // namespace Frontend
