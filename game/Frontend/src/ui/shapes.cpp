#include <ui/shapes.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Frontend {
    sf::ConvexShape makeRoundedRect(sf::Vector2f size, float cornerRadius, int pointsPerCorner) {
        pointsPerCorner = std::max(pointsPerCorner, 2);
        const float radius = std::min({cornerRadius, size.x / 2.f, size.y / 2.f});
        const sf::Vector2f half = size / 2.f;
        // Centrele colțurilor, în ordinea acelor de ceasornic începând din dreapta-sus.
        const sf::Vector2f centers[4] = {{half.x - radius, -half.y + radius},
                                         {half.x - radius, half.y - radius},
                                         {-half.x + radius, half.y - radius},
                                         {-half.x + radius, -half.y + radius}};
        constexpr float kQuarter = std::numbers::pi_v<float> / 2.f;

        sf::ConvexShape shape(static_cast<std::size_t>(4 * pointsPerCorner));
        std::size_t index = 0;
        for (int corner = 0; corner < 4; ++corner) {
            const float start = -kQuarter + static_cast<float>(corner) * kQuarter;
            for (int i = 0; i < pointsPerCorner; ++i) {
                const float angle = start + kQuarter * static_cast<float>(i) /
                                                static_cast<float>(pointsPerCorner - 1);
                shape.setPoint(
                    index++,
                    centers[corner] + sf::Vector2f{std::cos(angle), std::sin(angle)} * radius);
            }
        }
        return shape;
    }
}  // namespace Frontend
