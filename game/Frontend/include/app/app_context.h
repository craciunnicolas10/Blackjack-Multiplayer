#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace Frontend {
    class ScreenManager;

    // Ce primește fiecare ecran de la aplicație. Obiectele sunt deținute de Application.
    struct AppContext {
        sf::RenderWindow& window;
        const sf::Font& font;
        ScreenManager& screens;
    };
}  // namespace Frontend
