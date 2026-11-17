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

        // True cât timp utilizatorul scrie text (ex. un nume). Atunci tastele cu litere
        // (ex. M = sunet pornit/oprit) nu mai sunt scurtături globale.
        virtual bool isTypingText() const { return false; }
    };
}  // namespace Frontend
