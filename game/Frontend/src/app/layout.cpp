#include <app/layout.h>

#include <SFML/Graphics/Rect.hpp>

namespace Frontend {
    sf::View makeLetterboxView(sf::Vector2u windowSize) {
        sf::View view(sf::FloatRect({0.f, 0.f}, {kLogicalWidth, kLogicalHeight}));

        // Fereastra minimizată pe Windows raportează dimensiunea 0.
        if (windowSize.x == 0 || windowSize.y == 0) {
            return view;
        }

        const float windowRatio =
            static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
        const float logicalRatio = kLogicalWidth / kLogicalHeight;

        sf::FloatRect viewport({0.f, 0.f}, {1.f, 1.f});
        if (windowRatio > logicalRatio) {
            viewport.size.x = logicalRatio / windowRatio;
            viewport.position.x = (1.f - viewport.size.x) / 2.f;
        } else {
            viewport.size.y = windowRatio / logicalRatio;
            viewport.position.y = (1.f - viewport.size.y) / 2.f;
        }

        view.setViewport(viewport);
        return view;
    }
}  // namespace Frontend
