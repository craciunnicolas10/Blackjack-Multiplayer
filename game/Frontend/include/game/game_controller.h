#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include <SFML/System/String.hpp>

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
    // Jocul ține ascultătorii ca weak_ptr, deci obiectul trebuie să fie într-un shared_ptr:
    // se creează doar cu create().
    class GameController : public Backend::IGameListener,
                           public std::enable_shared_from_this<GameController> {
       public:
        // Pariul propus fiecărui jucător în prima rundă; apoi i se propune ultimul lui pariu.
        static constexpr int kSuggestedFirstBet = 50;

        // Creează jocul, așază jucătorii (în ordinea locurilor de la masă) și pornește prima
        // rundă. Masa trebuie să trăiască mai mult decât obiectul întors.
        static std::shared_ptr<GameController> create(TableScreen& table,
                                                      const std::vector<sf::String>& playerNames);

        GameController(const GameController&) = delete;
        GameController& operator=(const GameController&) = delete;

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
        GameController(TableScreen& table, std::shared_ptr<Backend::IGame> game);

        void connectTable();
        void addPlayers(const std::vector<sf::String>& playerNames);

        // Deschide panoul de pariu pentru primul jucător de la `fromSeat` încolo care poate
        // paria; dacă nu mai e niciunul, închide panoul.
        void askNextBet(std::size_t fromSeat);
        void placeBet(int amount);
        void startNextRound();

        void onBettingStarted();
        void onRoundOver();

        int balanceOf(std::size_t seat) const;
        std::size_t seatCount() const;

        TableScreen& m_table;
        std::shared_ptr<Backend::IGame> m_game;

        std::optional<std::size_t> m_bettingSeat;
        std::vector<int> m_lastBets;  // propuse din nou în runda următoare
        // Banii de la începutul rundei (înainte de pariu), ca să arătăm cât s-a câștigat.
        std::vector<int> m_balancesAtRoundStart;
    };
}  // namespace Frontend
