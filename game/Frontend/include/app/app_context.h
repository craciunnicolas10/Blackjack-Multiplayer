#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace Frontend {
    class CardTextures;
    class ScreenManager;

    // Ce primește fiecare ecran de la aplicație. Obiectele sunt deținute de Application.
    struct AppContext {
        sf::RenderWindow& window;
        const sf::Font& font;
        ScreenManager& screens;
        const CardTextures& cards;
    };
}  // namespace Frontend
