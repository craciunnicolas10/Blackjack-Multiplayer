#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

namespace Frontend {
    // Un ecran al aplicației (meniu, lobby, masă...). Desenează în coordonatele logice 1280x720.
    class Screen {
       public:
        virtual ~Screen() = default;

        virtual void handleEvent(const sf::Event& /*event*/) {}
        virtual void update(float /*dt*/) {}
        virtual void draw(sf::RenderTarget& target) = 0;
    };
}  // namespace Frontend
