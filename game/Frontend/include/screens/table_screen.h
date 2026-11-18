#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

#include <app/app_context.h>
#include <game/round_result.h>
#include <screens/screen.h>
#include <ui/betting_panel.h>
#include <ui/button.h>
#include <ui/hand_view.h>
#include <ui/result_badge.h>
#include <ui/round_summary_panel.h>
#include <ui/seat_view.h>
#include <ui/table_background.h>

namespace Frontend {
    class GameController;

    // Masa de joc. Nu știe regulile: jocul (Backend, prin GameController) îi spune ce să
    // arate prin funcțiile publice de mai jos, iar masa anunță prin
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

        // Dacă e vreo carte pe drum, suma se schimbă după ce ajunge (ex. câștigul apare după
        // ultima carte a dealerului, nu înainte).
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

        // Pariuri: panoul apare pentru un jucător (locul lui e evidențiat), iar pariul ales
        // ajunge la joc prin setOnBet. Limitele vin din joc; `initialBet` e propus din start
        // (ex. pariul din runda trecută). Cât timp se pariază, Hit/Stand/Double sunt inactive.
        void startBetting(std::size_t seat, int money, int minBet, int maxBet, int initialBet);
        void endBetting();
        std::optional<std::size_t> getBettingSeat() const;

        // Eticheta apare deasupra mâinii după ce s-au terminat animațiile cărților; atunci
        // jetoanele pariului pleacă spre jucător (câștig, egal) sau spre dealer (pierdere).
        void showResult(std::size_t seat, Backend::ERoundResult result);

        // Ecranul de rezultat: apare peste masă după ce s-au terminat toate animațiile
        // (cărți, jetoane, etichete), cu „Runda următoare” și „Meniu”. Se apelează după
        // showResult și setPlayerMoney; `netWinnings[seat]` = cât a câștigat (+) sau a pierdut
        // (-) jucătorul în runda asta. Locurile fără rezultat nu apar.
        void showRoundSummary(std::vector<int> netWinnings);
        // Nu mai urmează nicio rundă (nimeni nu mai are bani): ecranul de rezultat rămâne,
        // doar cu „Meniu”.
        void showGameOver();

        // Strânge cărțile, scorurile și rezultatele și închide panoul de pariu și ecranul de
        // rezultat, pentru o rundă nouă. Banii și pariurile afișate rămân.
        void clearRound();

        // True cât timp se mișcă ceva: cărți, jetoane, bani sau etichetele cu rezultatul.
        bool isAnimating() const;

        // --- Ce a ales jucătorul la rând ---
        void setOnHit(std::function<void()> onHit);
        void setOnStand(std::function<void()> onStand);
        void setOnDoubleDown(std::function<void()> onDoubleDown);
        void setOnBet(std::function<void(int)> onBet);
        // „Runda următoare” de pe ecranul de rezultat.
        void setOnNextRound(std::function<void()> onNextRound);

       private:
        enum class Action { Hit, Stand, DoubleDown };

        sf::Vector2f seatPosition(std::size_t seat) const;
        // Cât așteaptă următoarea carte până pleacă din pachet; mută coada mai departe.
        float takeDealDelay();
        // Cărți pe drum, în pachet sau în curs de întoarcere.
        bool cardsInMotion() const;
        // Când nu mai e nicio carte pe drum: aplică banii amânați și arată rezultatele.
        void applyWhenCardsSettled();
        void openRoundSummary();
        void refreshActionButtons();
        // Evidențiază locul jucătorului la rând sau al celui care pariază.
        void refreshSeatHighlight();
        void requestAction(Action action);
        void applyPendingActions();
        void goToMenu();

        void drawDeck(sf::RenderTarget& target) const;
        void drawLabel(sf::RenderTarget& target, const sf::String& text,
                       sf::Vector2f centerTop) const;
        void drawScore(sf::RenderTarget& target, const HandView& hand, int score) const;

        AppContext m_context;
        TableBackground m_background;
        std::vector<sf::String> m_playerNames;
        std::vector<SeatView> m_seats;
        std::vector<std::optional<int>> m_scores;
        std::vector<std::optional<ResultBadge>> m_results;
        std::vector<std::optional<int>> m_pendingMoney;
        std::vector<int> m_money;  // ultima sumă primită de la joc, pentru ecranul de rezultat
        bool m_resultsShown = false;  // etichetele sunt pe ecran și jetoanele au plecat
        HandView m_dealerHand;
        std::optional<int> m_dealerScore;
        std::optional<std::size_t> m_currentSeat;
        std::optional<std::size_t> m_bettingSeat;

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
        BettingPanel m_bettingPanel;
        RoundSummaryPanel m_summaryPanel;

        std::function<void()> m_onHit;
        std::function<void()> m_onStand;
        std::function<void()> m_onDoubleDown;
        std::function<void(int)> m_onBet;
        std::function<void()> m_onNextRound;

        // Ca în lobby: butoanele doar marchează ce s-a cerut, acțiunea rulează după ce s-au
        // terminat de parcurs evenimentele.
        std::optional<Action> m_pendingAction;
        std::optional<int> m_pendingBet;
        bool m_nextRoundRequested = false;

        // Ecranul de rezultat: cerut de joc, apare după animații și o mică pauză.
        std::optional<std::vector<int>> m_summaryNet;
        float m_summaryDelay = 0.f;
        bool m_summaryVisible = false;
        bool m_menuRequested = false;

        // Legătura cu jocul. Ultimul membru, ca să fie distrus primul: până atunci masa,
        // pe care o folosește, există încă.
        std::shared_ptr<GameController> m_game;
    };
}  // namespace Frontend
