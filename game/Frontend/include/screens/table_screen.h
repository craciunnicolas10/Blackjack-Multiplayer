#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

#include <app/app_context.h>
#include <screens/screen.h>
#include <ui/button.h>
#include <ui/hand_view.h>
#include <ui/seat_view.h>

namespace Frontend {
    class FakeGame;

    // Rezultatul unui jucător la sfârșitul rundei. Până se mută ERoundResult în Backend.
    enum class RoundResult { Win, Lose, Push, Blackjack };

    // Masa de joc. Nu știe regulile: jocul (deocamdată FakeGame, apoi adaptorul IGameListener)
    // îi spune ce să arate prin funcțiile publice de mai jos, iar masa anunță prin
    // setOnHit / setOnStand / setOnDoubleDown ce a ales jucătorul.
    // Cărțile se împart pe rând: dacă mai multe vin una după alta, fiecare pleacă din pachet
    // puțin după cea dinainte, chiar dacă au fost cerute în același frame.
    class TableScreen : public Screen {
       public:
        TableScreen(AppContext context, std::vector<sf::String> playerNames);
        ~TableScreen() override;

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void draw(sf::RenderTarget& target) override;

        // --- Apelate de joc ---
        std::size_t getSeatCount() const;

        void setPlayerMoney(std::size_t seat, int money);
        void setPlayerBet(std::size_t seat, int bet);
        // Scorul se vede într-o bulină lângă mână; nullopt o ascunde.
        void setPlayerScore(std::size_t seat, std::optional<int> score);
        void setDealerScore(std::optional<int> score);

        // Rangul și culoarea ca în ui/card_textures.h.
        void dealPlayerCard(std::size_t seat, int rank, int suit);
        // Pentru o carte cu fața în jos valoarea e ignorată, ca să nu apară pe ecran.
        void dealDealerCard(int rank, int suit, bool faceDown = false);
        // Întoarce prima carte ascunsă a dealerului, după ce au ajuns cărțile împărțite înainte.
        void revealDealerCard(int rank, int suit);

        // Jucătorul la rând: locul lui e evidențiat și butoanele Hit/Stand/Double sunt active.
        // nullopt = nu e rândul nimănui (pariuri, rândul dealerului, sfârșit de rundă).
        void setCurrentSeat(std::optional<std::size_t> seat);
        std::optional<std::size_t> getCurrentSeat() const;

        // Eticheta apare deasupra mâinii după ce s-au terminat animațiile cărților.
        void showResult(std::size_t seat, RoundResult result);

        // Strânge cărțile, scorurile și rezultatele, pentru o rundă nouă. Banii rămân.
        void clearRound();

        // True cât timp mai e vreo carte pe drum sau în curs de întoarcere.
        bool isAnimating() const;

        // --- Ce a ales jucătorul la rând ---
        void setOnHit(std::function<void()> onHit);
        void setOnStand(std::function<void()> onStand);
        void setOnDoubleDown(std::function<void()> onDoubleDown);

       private:
        enum class Action { Hit, Stand, DoubleDown };

        sf::Vector2f seatPosition(std::size_t seat) const;
        // Cât așteaptă următoarea carte până pleacă din pachet; mută coada mai departe.
        float takeDealDelay();
        void refreshActionButtons();
        void requestAction(Action action);
        void applyPendingActions();
        void goToMenu();

        void drawDeck(sf::RenderTarget& target) const;
        void drawLabel(sf::RenderTarget& target, const sf::String& text,
                       sf::Vector2f centerTop) const;
        void drawScore(sf::RenderTarget& target, const HandView& hand, int score) const;
        void drawResult(sf::RenderTarget& target, const HandView& hand,
                        RoundResult result) const;

        AppContext m_context;
        std::vector<SeatView> m_seats;
        std::vector<std::optional<int>> m_scores;
        std::vector<std::optional<RoundResult>> m_results;
        HandView m_dealerHand;
        std::optional<int> m_dealerScore;
        std::optional<std::size_t> m_currentSeat;

        float m_dealQueue = 0.f;  // secunde până pleacă următoarea carte din pachet
        // Cartea dealerului care se întoarce după ce se termină coada de împărțit.
        struct PendingReveal {
            int rank;
            int suit;
            float delay;
        };
        std::optional<PendingReveal> m_pendingReveal;

        Button m_hitButton;
        Button m_standButton;
        Button m_doubleButton;
        Button m_menuButton;

        std::function<void()> m_onHit;
        std::function<void()> m_onStand;
        std::function<void()> m_onDoubleDown;

        // Ca în lobby: butoanele doar marchează ce s-a cerut, acțiunea rulează după ce s-au
        // terminat de parcurs evenimentele.
        std::optional<Action> m_pendingAction;
        bool m_menuRequested = false;

        // Temporar, până vine Backend-ul: o rundă jucată cu cărți aleatoare.
        std::unique_ptr<FakeGame> m_game;
    };
}  // namespace Frontend
