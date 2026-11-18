#include <ui/title_banner.h>

#include <cmath>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <ui/card_textures.h>
#include <ui/card_view.h>
#include <ui/chip_view.h>
#include <ui/crisp_text.h>
#include <ui/text_layout.h>
#include <ui/theme.h>

namespace Frontend {
    namespace {
        // Pozițiile decorului față de centrul titlului, la scara 1.
        constexpr sf::Vector2f kCardsOffset{-300.f, 0.f};
        constexpr sf::Vector2f kChipsOffset{300.f, 30.f};
        constexpr float kChipRadius = 26.f;
        constexpr float kSwayDegrees = 2.5f;  // cât se leagănă cărțile
        constexpr float kSwaySpeed = 1.6f;

        const sf::Color kTitleOutline(70, 45, 5);
    }  // namespace

    TitleBanner::TitleBanner(const sf::Font& font, const CardTextures& cards,
                             const sf::String& title)
        : m_font(&font), m_cards(&cards), m_title(font, title, kTitleSize) {
        m_title.setStyle(sf::Text::Bold);
        m_title.setFillColor(Theme::kGold);
        m_title.setOutlineColor(kTitleOutline);
        m_title.setOutlineThickness(3.f);
        centerOrigin(m_title);
    }

    void TitleBanner::setPosition(sf::Vector2f center) { m_position = center; }

    sf::Vector2f TitleBanner::getPosition() const { return m_position; }

    void TitleBanner::setScale(float scale) { m_scale = scale; }

    void TitleBanner::update(float dt) { m_time += dt; }

    void TitleBanner::draw(sf::RenderTarget& target) const {
        const float sway = kSwayDegrees * std::sin(m_time * kSwaySpeed);

        // Evantaiul de cărți: K♥ în spate, A♠ în față.
        drawCard(target, 13, kSuitHearts, kCardsOffset + sf::Vector2f{22.f, 6.f}, 14.f - sway);
        drawCard(target, kMaxRank, kSuitSpades, kCardsOffset + sf::Vector2f{-22.f, 0.f},
                 -10.f + sway);

        drawChips(target, kChipsOffset);

        // Titlul, cu o umbră moale dedesubt.
        sf::Text shadow = m_title;
        shadow.setPosition(m_position + sf::Vector2f{4.f, 6.f} * m_scale);
        shadow.setScale({m_scale, m_scale});
        shadow.setFillColor(sf::Color(0, 0, 0, 110));
        shadow.setOutlineColor(sf::Color(0, 0, 0, 60));
        drawText(target, shadow);

        sf::Text title = m_title;
        title.setPosition(m_position);
        title.setScale({m_scale, m_scale});
        drawText(target, title);
    }

    void TitleBanner::drawCard(sf::RenderTarget& target, int rank, int suit, sf::Vector2f offset,
                               float degrees) const {
        const sf::Texture& texture = m_cards->get(rank, suit);
        const sf::Vector2f textureSize(texture.getSize());
        if (textureSize.x <= 0.f) {
            return;
        }
        // Puțin mai mari decât pe masă.
        const sf::Vector2f size = CardView::kSize * 1.25f * m_scale;
        const sf::Vector2f center = m_position + offset * m_scale;

        sf::RectangleShape shadow(size - sf::Vector2f{4.f, 4.f});
        shadow.setOrigin(shadow.getSize() / 2.f);
        shadow.setPosition(center + sf::Vector2f{5.f, 7.f} * m_scale);
        shadow.setRotation(sf::degrees(degrees));
        shadow.setFillColor(Theme::kShadow);
        target.draw(shadow);

        sf::Sprite sprite(texture);
        sprite.setOrigin(textureSize / 2.f);
        sprite.setScale({size.x / textureSize.x, size.y / textureSize.y});
        sprite.setPosition(center);
        sprite.setRotation(sf::degrees(degrees));
        target.draw(sprite);
    }

    void TitleBanner::drawChips(sf::RenderTarget& target, sf::Vector2f offset) const {
        const sf::Vector2f base = m_position + offset * m_scale;
        const float radius = kChipRadius * m_scale;

        // Un teanc înalt și unul mic lângă el.
        const int tall[] = {100, 100, 50, 50, 10};
        for (int i = 0; i < 5; ++i) {
            ChipView chip(*m_font, tall[i], radius);
            chip.setPosition(base + sf::Vector2f{0.f, -7.f * static_cast<float>(i)} * m_scale);
            chip.draw(target);
        }
        const int small[] = {10, 50};
        for (int i = 0; i < 2; ++i) {
            ChipView chip(*m_font, small[i], radius);
            chip.setPosition(base +
                             sf::Vector2f{48.f, 22.f - 7.f * static_cast<float>(i)} * m_scale);
            chip.draw(target);
        }
    }
}  // namespace Frontend
