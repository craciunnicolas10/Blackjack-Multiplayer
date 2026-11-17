#pragma once

#include <cstddef>
#include <random>
#include <vector>

namespace Frontend {
    class TableScreen;

    // Un joc de blackjack simplificat, doar ca să se vadă masa în acțiune până vine Backend-ul.
    // Cere mesei exact ce îi va cere și jocul real (cărți, rând, scoruri, rezultate), deci
    // înlocuirea lui cu adaptorul IGameListener nu schimbă nimic în TableScreen.
    // Simplificări: fără split/asigurare, dealerul nu verifică blackjack la început.
    // Limitele de pariu sunt propuneri până le stabilește echipa.
    class FakeGame {
       public:
        static constexpr int kStartingMoney = 1000;
        static constexpr int kMinBet = 10;
        static constexpr int kMaxBet = 500;
        static constexpr int kDefaultBet = 50;  // propus în prima rundă
        static constexpr int kDealerStandsOn = 17;
        static constexpr float kNextRoundDelay = 3.f;  // secunde după ce apar rezultatele

        explicit FakeGame(TableScreen& table);

        void startRound();
        void update(float dt);

       private:
        struct Card {
            int rank;
            int suit;
        };

        struct Player {
            int money = kStartingMoney;
            int bet = 0;
            int lastBet = kDefaultBet;  // propus din nou în runda următoare
            std::vector<Card> hand;
        };

        static int score(const std::vector<Card>& hand);
        static bool isBlackjack(const std::vector<Card>& hand);

        Card drawCard();
        void dealToPlayer(std::size_t seat);
        void dealToDealer(bool faceDown);

        // Pariurile se pun pe rând; după ultimul jucător se împart cărțile.
        void askBet(std::size_t seat);
        void placeBet(int amount);
        void dealInitialCards();

        void hit();
        void stand();
        void doubleDown();

        // Dă rândul primului jucător de la `seat` încolo care mai poate cere cărți;
        // dacă nu mai e niciunul, joacă dealerul.
        void nextTurn(std::size_t seat);
        void playDealer();
        void settle();

        TableScreen& m_table;
        std::mt19937 m_rng;
        std::vector<Card> m_deck;
        std::vector<Player> m_players;
        std::vector<Card> m_dealer;
        std::size_t m_bettingSeat = 0;
        bool m_dealerRevealed = false;  // s-a întors a doua carte a dealerului
        bool m_roundOver = false;
        float m_nextRoundTimer = 0.f;
    };
}  // namespace Frontend
