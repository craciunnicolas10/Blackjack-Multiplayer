#include <ui/text_layout.h>

#include <SFML/Graphics/Rect.hpp>

#include <app/layout.h>

namespace Frontend {
    void centerOrigin(sf::Text& text) {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.position + bounds.size / 2.f);
    }

    void centerTextAt(sf::Text& text, sf::Vector2f center) {
        centerOrigin(text);
        text.setPosition(center);
    }

    void centerTextTopAt(sf::Text& text, sf::Vector2f centerTop) {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y});
        text.setPosition(centerTop);
    }

    void centerHorizontally(sf::Text& text, float top) {
        centerTextTopAt(text, {kLogicalWidth / 2.f, top});
    }
}  // namespace Frontend
