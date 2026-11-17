#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

namespace Frontend {
    // Iconița „fără sunet”: un difuzor tăiat cu un X, pe un cerc închis la culoare.
    // Application o desenează peste orice ecran, în colțul din dreapta-jos, cât timp
    // sunetul e oprit (tasta M).
    class MuteIcon {
       public:
        static constexpr float kRadius = 20.f;

        // `center` = centrul cercului.
        void setPosition(sf::Vector2f center);
        void draw(sf::RenderTarget& target) const;

       private:
        sf::Vector2f m_position;
    };
}  // namespace Frontend
