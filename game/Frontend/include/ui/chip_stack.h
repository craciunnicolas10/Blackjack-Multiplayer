#pragma once

#include <vector>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

#include <ui/chip_view.h>

namespace Frontend {
    // Un teanc de jetoane pentru o sumă (ex. pariul unui jucător): suma e împărțită în jetoane
    // de 100, 50 și 10, cele mari jos. La final de rundă teancul poate „zbura” spre jucător sau
    // spre dealer și dispare acolo.
    class ChipStack {
       public:
        static constexpr float kChipRadius = 15.f;
        static constexpr float kLayerOffset = 6.f;  // cât urcă fiecare jeton față de cel de sub el
        static constexpr int kMaxChips = 12;        // peste atât nu mai crește teancul
        static constexpr float kFlightDuration = 0.45f;
        static constexpr float kFlightStagger = 0.05f;  // jetoanele pleacă unul după altul

        explicit ChipStack(const sf::Font& font);

        // `bottomCenter` = centrul jetonului de jos.
        void setPosition(sf::Vector2f bottomCenter);
        sf::Vector2f getPosition() const;

        // Refă teancul instant (oprește un zbor în curs). 0 = teanc gol.
        void setAmount(int amount);
        int getAmount() const;

        // Jetoanele zboară spre `target` și dispar acolo; după asta teancul e gol.
        void sendTo(sf::Vector2f target);
        bool isAnimating() const;

        void update(float dt);
        void draw(sf::RenderTarget& target) const;

       private:
        sf::Vector2f restingPosition(std::size_t index) const;
        void rebuild();
        void placeChips();

        const sf::Font* m_font;
        sf::Vector2f m_position;
        int m_amount = 0;
        std::vector<ChipView> m_chips;

        bool m_flying = false;
        float m_flightTime = 0.f;
        sf::Vector2f m_flightTarget;
    };
}  // namespace Frontend
