#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include <game/round_result.h>

namespace Frontend {
    // Eticheta cu rezultatul unui jucător („Câștig”, „Pierdere”, „Egal”, „Blackjack!”).
    // La apariție crește puțin peste mărimea finală și apoi se așază. Animația începe la primul
    // update(), deci trebuie actualizată doar de când eticheta e vizibilă.
    class ResultBadge {
       public:
        static constexpr sf::Vector2f kSize{150.f, 34.f};
        static constexpr float kAppearDuration = 0.45f;  // secunde

        ResultBadge(const sf::Font& font, RoundResult result);

        RoundResult getResult() const;

        // `center` = centrul etichetei.
        void setPosition(sf::Vector2f center);
        sf::Vector2f getPosition() const;

        // Pornește din nou animația de apariție.
        void restart();
        bool isAnimating() const;

        void update(float dt);
        void draw(sf::RenderTarget& target) const;

       private:
        RoundResult m_result;
        sf::Text m_text;
        sf::Color m_color;
        sf::Vector2f m_position;
        float m_elapsed = 0.f;
        bool m_started = false;
    };
}  // namespace Frontend
