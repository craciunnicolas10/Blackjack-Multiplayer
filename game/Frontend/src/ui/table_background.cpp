#include <ui/table_background.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <app/layout.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        // Postavul: mai deschis în centru, mai închis spre margini.
        const sf::Color kFeltCenter(30, 125, 72);
        const sf::Color kFeltEdge(6, 52, 28);
        constexpr sf::Vector2f kFeltCenterPoint{kLogicalWidth / 2.f, kLogicalHeight * 0.42f};
        constexpr int kFeltColumns = 48;
        constexpr int kFeltRows = 27;

        constexpr unsigned kGrainSize = 128;  // textura se repetă pe tot ecranul

        // Arcul e o bucată dintr-un cerc mare cu centrul deasupra ecranului: cel mai jos punct
        // e la mijloc, între cărțile dealerului și ale jucătorilor.
        constexpr sf::Vector2f kArcCenter{kLogicalWidth / 2.f, -420.f};
        constexpr float kArcOuterRadius = 730.f;
        constexpr float kArcInnerRadius = 700.f;
        constexpr float kArcHalfWidth = 470.f;  // cât se întinde arcul la stânga/dreapta
        constexpr int kArcSegments = 96;
        const sf::Color kArcColor(240, 210, 120, 150);

        constexpr unsigned kArcTextSize = 16;
        const sf::Color kArcTextColor(240, 210, 120, 190);

        sf::Color lerp(sf::Color a, sf::Color b, float t) {
            const auto mix = [t](std::uint8_t x, std::uint8_t y) {
                return static_cast<std::uint8_t>(
                    static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t);
            };
            return {mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b), mix(a.a, b.a)};
        }

        sf::Color feltColorAt(sf::Vector2f point) {
            // Distanța normalizată față de centru, puțin turtită pe verticală.
            const sf::Vector2f delta = point - kFeltCenterPoint;
            const float distance = std::sqrt((delta.x / 760.f) * (delta.x / 760.f) +
                                             (delta.y / 520.f) * (delta.y / 520.f));
            const float t = std::clamp(distance, 0.f, 1.f);
            return lerp(kFeltCenter, kFeltEdge, t * t * (3.f - 2.f * t));  // smoothstep
        }

        // Punctul de pe cercul arcului, la unghiul `angle` (0 = drept în jos).
        sf::Vector2f arcPoint(float radius, float angle) {
            return kArcCenter + sf::Vector2f{radius * std::sin(angle), radius * std::cos(angle)};
        }

        // O linie curbă subțire, ca bandă de triunghiuri între două raze apropiate.
        void appendArcLine(sf::VertexArray& vertices, float radius, float thickness,
                           float halfAngle) {
            for (int i = 0; i < kArcSegments; ++i) {
                const float a0 =
                    -halfAngle + 2.f * halfAngle * static_cast<float>(i) / kArcSegments;
                const float a1 =
                    -halfAngle + 2.f * halfAngle * static_cast<float>(i + 1) / kArcSegments;
                const sf::Vector2f p0 = arcPoint(radius - thickness / 2.f, a0);
                const sf::Vector2f p1 = arcPoint(radius + thickness / 2.f, a0);
                const sf::Vector2f p2 = arcPoint(radius - thickness / 2.f, a1);
                const sf::Vector2f p3 = arcPoint(radius + thickness / 2.f, a1);
                for (const sf::Vector2f p : {p0, p1, p2, p1, p3, p2}) {
                    vertices.append(sf::Vertex{p, kArcColor});
                }
            }
        }
    }  // namespace

    TableBackground::TableBackground(const sf::Font& font)
        : m_felt(sf::PrimitiveType::Triangles), m_arc(sf::PrimitiveType::Triangles) {
        buildFelt();
        buildGrain();
        buildArc();
        buildArcText(font);
    }

    void TableBackground::draw(sf::RenderTarget& target) const {
        target.draw(m_felt);

        sf::Sprite grain(m_grain);
        grain.setTextureRect(sf::IntRect(
            {0, 0}, {static_cast<int>(kLogicalWidth), static_cast<int>(kLogicalHeight)}));
        target.draw(grain);

        target.draw(m_arc);
        for (const sf::Text& letter : m_arcText) {
            target.draw(letter);
        }
    }

    void TableBackground::buildFelt() {
        // O grilă de dreptunghiuri; fiecare colț are culoarea gradientului în acel punct.
        const float cellWidth = kLogicalWidth / kFeltColumns;
        const float cellHeight = kLogicalHeight / kFeltRows;
        for (int row = 0; row < kFeltRows; ++row) {
            for (int column = 0; column < kFeltColumns; ++column) {
                const sf::Vector2f topLeft{static_cast<float>(column) * cellWidth,
                                           static_cast<float>(row) * cellHeight};
                const sf::Vector2f corners[4] = {topLeft, topLeft + sf::Vector2f{cellWidth, 0.f},
                                                 topLeft + sf::Vector2f{0.f, cellHeight},
                                                 topLeft + sf::Vector2f{cellWidth, cellHeight}};
                for (const int index : {0, 1, 2, 1, 3, 2}) {
                    m_felt.append(sf::Vertex{corners[index], feltColorAt(corners[index])});
                }
            }
        }
    }

    void TableBackground::buildGrain() {
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

    void TableBackground::buildArc() {
        const float outerHalfAngle = std::asin(kArcHalfWidth / kArcOuterRadius);
        const float innerHalfAngle = std::asin(kArcHalfWidth / kArcInnerRadius);
        appendArcLine(m_arc, kArcOuterRadius, 3.f, outerHalfAngle);
        appendArcLine(m_arc, kArcInnerRadius, 1.5f, innerHalfAngle);
    }

    void TableBackground::buildArcText(const sf::Font& font) {
        const sf::String text = utf8("Dealer stands on 17  ·  Blackjack plătește 3:2");
        const float radius = (kArcOuterRadius + kArcInnerRadius) / 2.f;

        // Lățimea fiecărei litere, ca să le așezăm una după alta de-a lungul arcului.
        std::vector<float> advances;
        float totalWidth = 0.f;
        for (std::size_t i = 0; i < text.getSize(); ++i) {
            const float advance = font.getGlyph(text[i], kArcTextSize, false).advance;
            advances.push_back(advance);
            totalWidth += advance;
        }

        // Toate literele se centrează pe verticală după mijlocul unei majuscule, ca să stea pe
        // aceeași linie (altfel „g” sau „,” ar sări din rând).
        const sf::FloatRect capital = sf::Text(font, "H", kArcTextSize).getLocalBounds();
        const float capitalMiddle = capital.position.y + capital.size.y / 2.f;

        // Textul e centrat pe mijlocul arcului (unghiul 0). Spre dreapta unghiul crește și arcul
        // urcă, așa că litera se rotește invers acelor de ceasornic, după tangentă.
        float distance = -totalWidth / 2.f;
        for (std::size_t i = 0; i < text.getSize(); ++i) {
            const float middle = distance + advances[i] / 2.f;
            const float angle = middle / radius;

            sf::Text letter(font, sf::String(text[i]), kArcTextSize);
            letter.setOrigin({advances[i] / 2.f, capitalMiddle});
            letter.setPosition(arcPoint(radius, angle));
            letter.setRotation(sf::radians(-angle));
            letter.setFillColor(kArcTextColor);
            m_arcText.push_back(letter);

            distance += advances[i];
        }
    }
}  // namespace Frontend
