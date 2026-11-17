#pragma once

#include <cstdint>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

namespace Frontend {
    // Un jeton rotund, văzut de sus, cu valoarea scrisă pe el. Culoarea depinde de valoare:
    // 10 albastru, 50 roșu, 100 negru cu auriu (alte valori: verde).
    class ChipView {
       public:
        static constexpr float kDefaultRadius = 16.f;

        ChipView(const sf::Font& font, int value, float radius = kDefaultRadius);

        // Poziția e centrul jetonului.
        void setPosition(sf::Vector2f center);
        sf::Vector2f getPosition() const;
        float getRadius() const;
        int getValue() const;

        void setScale(float scale);  // ex. mărit la hover
        void setAlpha(std::uint8_t alpha);
        void setDimmed(bool dimmed);  // aspect de jeton dezactivat (gri)

        // True dacă punctul e în cercul jetonului (fără scalare).
        bool contains(sf::Vector2f point) const;

        void draw(sf::RenderTarget& target) const;

       private:
        sf::Color adjust(sf::Color color) const;  // aplică transparența și aspectul dezactivat

        const sf::Font* m_font;
        int m_value;
        float m_radius;
        sf::Vector2f m_position;
        float m_scale = 1.f;
        std::uint8_t m_alpha = 255;
        bool m_dimmed = false;
    };
}  // namespace Frontend
