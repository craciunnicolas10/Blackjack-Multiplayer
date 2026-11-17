#include <ui/table_background.h>

#include <cmath>

#include <app/layout.h>
#include <ui/crisp_text.h>
#include <ui/theme.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        // Arcul e o bucată dintr-un cerc mare cu centrul deasupra ecranului: cel mai jos punct
        // e la mijloc, între cărțile dealerului și ale jucătorilor.
        constexpr sf::Vector2f kArcCenter{kLogicalWidth / 2.f, -420.f};
        constexpr float kArcOuterRadius = 730.f;
        constexpr float kArcInnerRadius = 700.f;
        constexpr float kArcHalfWidth = 470.f;  // cât se întinde arcul la stânga/dreapta
        constexpr int kArcSegments = 96;

        constexpr unsigned kArcTextSize = 16;

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
                    vertices.append(sf::Vertex{p, Theme::kTableLine});
                }
            }
        }
    }  // namespace

    TableBackground::TableBackground(const sf::Font& font) : m_arc(sf::PrimitiveType::Triangles) {
        buildArc();
        buildArcText(font);
    }

    void TableBackground::draw(sf::RenderTarget& target) const {
        m_felt.draw(target);
        target.draw(m_arc);
        for (const sf::Text& letter : m_arcText) {
            drawText(target, letter);
        }
    }

    void TableBackground::buildArc() {
        const float outerHalfAngle = std::asin(kArcHalfWidth / kArcOuterRadius);
        const float innerHalfAngle = std::asin(kArcHalfWidth / kArcInnerRadius);
        appendArcLine(m_arc, kArcOuterRadius, 3.f, outerHalfAngle);
        appendArcLine(m_arc, kArcInnerRadius, 1.5f, innerHalfAngle);
    }

    void TableBackground::buildArcText(const sf::Font& font) {
        const sf::String text = utf8("Dealerul se oprește la 17  ·  Blackjack plătește 3:2");
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
            letter.setFillColor(Theme::kTableText);
            m_arcText.push_back(letter);

            distance += advances[i];
        }
    }
}  // namespace Frontend
