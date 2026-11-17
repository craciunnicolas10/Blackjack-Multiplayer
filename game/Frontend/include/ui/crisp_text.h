#pragma once

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

namespace Frontend {
    // Desenează un text clar la orice mărime a ferestrei. Folosiți-o în loc de
    // `target.draw(text)`.
    //
    // De ce: totul se desenează în spațiul logic 1280x720 și apoi se mărește. Un sf::Text de
    // 22 px are literele generate la 22 px, iar la ecran complet 1080p ele sunt întinse la
    // 33 px și ies încețoșate. Aici literele se generează direct la mărimea reală de pe ecran
    // (22 x 1,5 = 33 px) și textul se micșorează înapoi în spațiul logic, deci pe ecran ajunge
    // pixel cu pixel. Poziția, originea și restul rămân cum le-a setat apelantul.
    void drawText(sf::RenderTarget& target, const sf::Text& text,
                  sf::RenderStates states = sf::RenderStates::Default);
}  // namespace Frontend
