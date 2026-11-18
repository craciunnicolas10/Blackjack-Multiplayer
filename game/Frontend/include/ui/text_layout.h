#pragma once

#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

// Așezarea textelor. Originea unui sf::Text e colțul de sus al primei litere, așa că pentru
// centrare trebuie mutată după mărimea reală a textului; funcțiile de aici fac asta.
namespace Frontend {
    // Originea în mijlocul textului: după asta, setPosition(p) pune mijlocul textului în p.
    void centerOrigin(sf::Text& text);

    // Pune mijlocul textului în `center`.
    void centerTextAt(sf::Text& text, sf::Vector2f center);

    // Centrează textul pe orizontală în jurul lui `centerTop.x`, cu partea de sus la `centerTop.y`.
    void centerTextTopAt(sf::Text& text, sf::Vector2f centerTop);

    // Centrează textul pe orizontală în fereastră (1280 logic), cu partea de sus la `top`.
    void centerHorizontally(sf::Text& text, float top);
}  // namespace Frontend
