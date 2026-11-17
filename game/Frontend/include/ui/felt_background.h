#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace Frontend {
    // Postavul verde pe tot ecranul logic 1280x720: un gradient (mai deschis la mijloc, mai
    // închis spre margini) și o textură fină, ca de țesătură. E fundalul comun pentru meniu,
    // lobby și masă (TableBackground îl folosește și adaugă arcul și textul mesei).
    // Se construiește o dată, în constructorul ecranului, și se desenează primul în draw().
    class FeltBackground {
       public:
        FeltBackground();

        void draw(sf::RenderTarget& target) const;

       private:
        void buildGradient();
        void buildGrain();

        sf::VertexArray m_gradient;
        sf::Texture m_grain;
    };
}  // namespace Frontend
