#include <ui/felt_background.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <app/layout.h>
#include <ui/theme.h>

namespace Frontend {
    namespace {
        constexpr sf::Vector2f kCenterPoint{kLogicalWidth / 2.f, kLogicalHeight * 0.42f};
        constexpr int kColumns = 48;
        constexpr int kRows = 27;
        constexpr unsigned kGrainSize = 128;  // textura se repetă pe tot ecranul

        sf::Color lerp(sf::Color a, sf::Color b, float t) {
            const auto mix = [t](std::uint8_t x, std::uint8_t y) {
                return static_cast<std::uint8_t>(
                    static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t);
            };
            return {mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b), mix(a.a, b.a)};
        }

        sf::Color colorAt(sf::Vector2f point) {
            // Distanța normalizată față de centru, puțin turtită pe verticală.
            const sf::Vector2f delta = point - kCenterPoint;
            const float distance = std::sqrt((delta.x / 760.f) * (delta.x / 760.f) +
                                             (delta.y / 520.f) * (delta.y / 520.f));
            const float t = std::clamp(distance, 0.f, 1.f);
            return lerp(Theme::kFeltCenter, Theme::kFeltEdge,
                        t * t * (3.f - 2.f * t));  // smoothstep
        }
    }  // namespace

    FeltBackground::FeltBackground() : m_gradient(sf::PrimitiveType::Triangles) {
        buildGradient();
        buildGrain();
    }

    void FeltBackground::draw(sf::RenderTarget& target) const {
        target.draw(m_gradient);

        sf::Sprite grain(m_grain);
        grain.setTextureRect(sf::IntRect(
            {0, 0}, {static_cast<int>(kLogicalWidth), static_cast<int>(kLogicalHeight)}));
        target.draw(grain);
    }

    void FeltBackground::buildGradient() {
        // O grilă de dreptunghiuri; fiecare colț are culoarea gradientului în acel punct.
        const float cellWidth = kLogicalWidth / kColumns;
        const float cellHeight = kLogicalHeight / kRows;
        for (int row = 0; row < kRows; ++row) {
            for (int column = 0; column < kColumns; ++column) {
                const sf::Vector2f topLeft{static_cast<float>(column) * cellWidth,
                                           static_cast<float>(row) * cellHeight};
                const sf::Vector2f corners[4] = {topLeft, topLeft + sf::Vector2f{cellWidth, 0.f},
                                                 topLeft + sf::Vector2f{0.f, cellHeight},
                                                 topLeft + sf::Vector2f{cellWidth, cellHeight}};
                for (const int index : {0, 1, 2, 1, 3, 2}) {
                    m_gradient.append(sf::Vertex{corners[index], colorAt(corners[index])});
                }
            }
        }
    }

    void FeltBackground::buildGrain() {
        // Zgomot fin, aproape transparent: dă impresia de țesătură peste gradient.
        sf::Image image({kGrainSize, kGrainSize}, sf::Color::Transparent);
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> shade(0, 2);
        std::uniform_int_distribution<int> alpha(0, 14);
        for (unsigned y = 0; y < kGrainSize; ++y) {
            for (unsigned x = 0; x < kGrainSize; ++x) {
                const bool light = shade(rng) == 0;
                const auto a = static_cast<std::uint8_t>(alpha(rng));
                image.setPixel({x, y}, light ? sf::Color(255, 255, 255, a) : sf::Color(0, 0, 0, a));
            }
        }
        if (m_grain.loadFromImage(image)) {
            m_grain.setRepeated(true);
        }
    }
}  // namespace Frontend
