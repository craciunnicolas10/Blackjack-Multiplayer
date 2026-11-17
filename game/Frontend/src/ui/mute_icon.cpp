#include <ui/mute_icon.h>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <ui/theme.h>

namespace Frontend {
    void MuteIcon::setPosition(sf::Vector2f center) { m_position = center; }

    void MuteIcon::draw(sf::RenderTarget& target) const {
        sf::CircleShape background(kRadius);
        background.setOrigin({kRadius, kRadius});
        background.setPosition(m_position);
        background.setFillColor(sf::Color(0, 0, 0, 150));
        background.setOutlineColor(Theme::kGold);
        background.setOutlineThickness(1.5f);
        target.draw(background);

        // Difuzorul: un dreptunghi mic și o pâlnie spre dreapta, puțin la stânga centrului.
        const sf::Vector2f speaker = m_position + sf::Vector2f{-5.f, 0.f};
        sf::ConvexShape body(6);
        body.setPoint(0, {-8.f, -4.f});
        body.setPoint(1, {-3.f, -4.f});
        body.setPoint(2, {3.f, -10.f});
        body.setPoint(3, {3.f, 10.f});
        body.setPoint(4, {-3.f, 4.f});
        body.setPoint(5, {-8.f, 4.f});
        body.setPosition(speaker);
        body.setFillColor(Theme::kTextPrimary);
        target.draw(body);

        // X-ul roșu din dreapta difuzorului.
        const sf::Vector2f cross = m_position + sf::Vector2f{9.f, 0.f};
        for (const float angle : {45.f, -45.f}) {
            sf::RectangleShape bar({12.f, 2.5f});
            bar.setOrigin(bar.getSize() / 2.f);
            bar.setPosition(cross);
            bar.setRotation(sf::degrees(angle));
            bar.setFillColor(Theme::kNegative);
            target.draw(bar);
        }
    }
}  // namespace Frontend
