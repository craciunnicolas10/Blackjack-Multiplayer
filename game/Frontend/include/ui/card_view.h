#pragma once

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

namespace Frontend {
    class CardTextures;

    // O carte pe masă, cu fața sau cu spatele în sus. Poziția e colțul din stânga-sus, în
    // coordonatele logice 1280x720. Știe să alunece la o poziție nouă și să se întoarcă animat;
    // pentru animații trebuie apelat update(dt) în fiecare frame.
    class CardView {
       public:
        // Imaginile au proporția 250x363.
        static constexpr sf::Vector2f kSize{90.f, 131.f};
        static constexpr float kMoveDuration = 0.35f;  // secunde
        static constexpr float kFlipDuration = 0.3f;

        // Carte cunoscută. Rangul și culoarea ca în card_textures.h.
        CardView(const CardTextures& textures, int rank, int suit, bool faceUp = true);
        // Carte a cărei valoare nu se știe încă (ex. cartea ascunsă a dealerului): se vede
        // doar spatele până la setCard().
        explicit CardView(const CardTextures& textures);

        void setCard(int rank, int suit);
        bool isKnown() const;
        int getRank() const;  // 0 dacă nu e cunoscută
        int getSuit() const;  // -1 dacă nu e cunoscută

        void setFaceUp(bool faceUp);  // instant, oprește o eventuală întoarcere în curs
        bool isFaceUp() const;
        // Întoarce cartea animat (fața devine spate și invers).
        void flip(float duration = kFlipDuration);

        // Mută instant și oprește o eventuală alunecare în curs.
        void setPosition(sf::Vector2f position);
        sf::Vector2f getPosition() const;
        // Alunecă de la poziția curentă la `target`. `delay` = cât stă pe loc înainte să plece
        // (util când se împart mai multe cărți una după alta).
        void moveTo(sf::Vector2f target, float duration = kMoveDuration, float delay = 0.f);
        // Schimbă destinația: dacă e în mișcare, își continuă drumul spre noua țintă (și își
        // păstrează întârzierea); altfel alunecă acolo.
        void retarget(sf::Vector2f target);
        bool isMoving() const;
        // True cât timp cartea e împărțită dar n-a plecat încă din pachet (`delay` din moveTo).
        bool isWaitingInDeck() const;

        bool isAnimating() const;
        void update(float dt);
        // `states` permite, de exemplu, mutarea întregii mâini (scuturarea din HandView).
        void draw(sf::RenderTarget& target,
                  sf::RenderStates states = sf::RenderStates::Default) const;

       private:
        const CardTextures* m_textures;
        int m_rank = 0;
        int m_suit = -1;
        bool m_faceUp = false;

        sf::Vector2f m_position;
        sf::Vector2f m_moveFrom;
        sf::Vector2f m_moveTarget;
        float m_moveElapsed = 0.f;
        float m_moveDuration = 0.f;
        float m_moveDelay = 0.f;
        bool m_moving = false;

        float m_flipElapsed = 0.f;
        float m_flipDuration = 0.f;
        bool m_flipping = false;
        bool m_flipSwapped = false;  // fața s-a schimbat deja la jumătatea întoarcerii
    };
}  // namespace Frontend
