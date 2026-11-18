#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include <Backend/IGame.h>
#include <Backend/IGameListener.h>

namespace Frontend {
    class TableScreen;

    // Legătura dintre joc (Backend) și masă:
    //  - ascultă jocul (Observer: IGameListener) și traduce fiecare anunț într-o funcție a
    //    mesei (OnPlayerCardDealt → dealPlayerCard etc.);
    //  - trimite jocului ce a ales jucătorul pe masă (Hit → IGame::Hit etc.).
    // Regulile sunt toate în Backend; clasa asta doar mută informația dintr-o parte în alta.
    //
    // Jocul poate fi cel local (IGame::Produce) sau, în rețea, Net::RemoteGame: pentru
    // GameController e același IGame. În rețea, doar locurile de pe calculatorul ăsta primesc
    // panoul de pariu și butoanele; la celelalte masa arată pe cine așteaptă.
    //
    // Jocul ține ascultătorii ca weak_ptr, deci obiectul trebuie să fie într-un shared_ptr:
    // se creează doar cu create().
    class GameController : public Backend::IGameListener {
       public:
        // Pariul propus fiecărui jucător în prima rundă; apoi i se propune ultimul lui pariu.
        static constexpr int kSuggestedFirstBet = 50;

        // Leagă jocul de masă. `localSeats[i]` = locul i e jucat de pe calculatorul ăsta.
        // Jucătorii și prima rundă îi pornește cine a creat jocul (vezi GameSession).
        // Masa trebuie să trăiască mai mult decât obiectul întors.
        static std::shared_ptr<GameController> create(TableScreen& table,
                                                      std::shared_ptr<Backend::IGame> game,
                                                      std::vector<bool> localSeats);

        GameController(const GameController&) = delete;
        GameController& operator=(const GameController&) = delete;

        // Jocul nu mai pornește nicio rundă (în rețea, aflat de la gazdă).
        void showGameOver();

        // --- IGameListener ---
        void OnGameStateChanged(Backend::EGameState newState) override;
        void OnPlayerAdded(int playerIndex) override;
        void OnBetPlaced(int playerIndex, int amount) override;
        void OnPlayerCardDealt(int playerIndex, Backend::Card card) override;
        void OnDealerCardDealt(Backend::Card card, bool isFaceDown) override;
        void OnDealerCardRevealed(Backend::Card card) override;
        void OnTurnChanged(int playerIndex) override;
        void OnRoundEnded(int playerIndex, Backend::ERoundResult result) override;

       private:
        GameController(TableScreen& table, std::shared_ptr<Backend::IGame> game,
                       std::vector<bool> localSeats);

        void connectTable();
        bool isLocal(std::size_t seat) const;

        // Deschide panoul de pariu pentru primul jucător care mai are de pariat (îl întreabă
        // pe joc: IGame::CanPlaceBet); dacă nu mai e niciunul, închide panoul. Merge după
        // starea jocului, nu după ordinea anunțurilor, deci e la fel pe toate calculatoarele
        // dintr-un joc în rețea.
        void askNextBet();
        void placeBet(int amount);
        void requestNextRound();

        void onBettingStarted();
        void onRoundOver();

        void showDealerScore();
        int balanceOf(std::size_t seat) const;
        std::size_t seatCount() const;

        TableScreen& m_table;
        std::shared_ptr<Backend::IGame> m_game;
        std::vector<bool> m_localSeats;

        std::optional<std::size_t> m_bettingSeat;
        std::vector<int> m_lastBets;  // propuse din nou în runda următoare
        // Banii de la începutul rundei (înainte de pariu), ca să arătăm cât s-a câștigat.
        std::vector<int> m_balancesAtRoundStart;
    };
}  // namespace Frontend
