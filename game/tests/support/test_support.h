#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <SFML/Network/Packet.hpp>

#include <Backend/Card.h>
#include <Backend/GameEnums.h>
#include <Backend/IDeck.h>
#include <Backend/IGame.h>
#include <Backend/IGameListener.h>
#include <Backend/INetworkListener.h>
#include <Backend/IPlayer.h>
#include <Backend/NetworkEnums.h>

namespace Backend {
    int CardValue(ERank rank);
    int ScoreHand(const std::vector<Card>& hand);
    bool IsBlackjack(const std::vector<Card>& hand);

    inline bool operator==(const Card& left, const Card& right) {
        return left.rank == right.rank && left.suit == right.suit;
    }

    inline void PrintTo(const Card& card, std::ostream* stream) {
        *stream << "Card{rank=" << static_cast<int>(card.rank)
                << ", suit=" << static_cast<int>(card.suit) << "}";
    }
}  // namespace Backend

namespace TestSupport {
    inline Backend::Card Make(Backend::ERank rank, Backend::ESuit suit = Backend::ESuit::Spades) {
        return Backend::Card{rank, suit};
    }

    inline Backend::Card V(int value) {
        if (value == 11) {
            return Make(Backend::ERank::Ace, Backend::ESuit::Hearts);
        }
        return Make(static_cast<Backend::ERank>(value), Backend::ESuit::Clubs);
    }

    inline std::vector<Backend::Card> ArrangeDeck(
        const std::vector<std::vector<Backend::Card>>& players,
        const std::vector<Backend::Card>& dealer,
        const std::vector<Backend::Card>& extra = {}) {
        std::vector<Backend::Card> cards;
        for (std::size_t round = 0; round < 2; ++round) {
            for (const auto& hand : players) {
                cards.push_back(hand.at(round));
            }
            cards.push_back(dealer.at(round));
        }
        cards.insert(cards.end(), extra.begin(), extra.end());
        return cards;
    }

    inline std::vector<Backend::Card> Concat(std::vector<Backend::Card> first,
                                             const std::vector<Backend::Card>& second) {
        first.insert(first.end(), second.begin(), second.end());
        return first;
    }

    class ScriptedDeck : public Backend::IDeck {
       public:
        explicit ScriptedDeck(std::vector<Backend::Card> cards) : m_cards(std::move(cards)) {}

        void Shuffle() override {
            ++shuffleCount;
        }

        Backend::Card Draw() override {
            if (m_next < m_cards.size()) {
                return m_cards[m_next++];
            }
            ++overdraws;
            return Backend::Card{Backend::ERank::Two, Backend::ESuit::Clubs};
        }

        int shuffleCount = 0;
        int overdraws = 0;

       private:
        std::vector<Backend::Card> m_cards;
        std::size_t m_next = 0;
    };

    class MockDeck : public Backend::IDeck {
       public:
        MOCK_METHOD(void, Shuffle, (), (override));
        MOCK_METHOD(Backend::Card, Draw, (), (override));
    };

    class MockGameListener : public Backend::IGameListener {
       public:
        MOCK_METHOD(void, OnGameStateChanged, (Backend::EGameState), (override));
        MOCK_METHOD(void, OnPlayerAdded, (int), (override));
        MOCK_METHOD(void, OnBetPlaced, (int, int), (override));
        MOCK_METHOD(void, OnPlayerCardDealt, (int, Backend::Card), (override));
        MOCK_METHOD(void, OnDealerCardDealt, (Backend::Card, bool), (override));
        MOCK_METHOD(void, OnDealerCardRevealed, (Backend::Card), (override));
        MOCK_METHOD(void, OnTurnChanged, (int), (override));
        MOCK_METHOD(void, OnRoundEnded, (int, Backend::ERoundResult), (override));
    };

    class MockNetworkListener : public Backend::INetworkListener {
       public:
        MOCK_METHOD(void, OnPlayersChanged, (const std::vector<std::string>&), (override));
        MOCK_METHOD(void, OnJoinRejected, (Backend::EJoinRejection), (override));
        MOCK_METHOD(void, OnGameStarted, (), (override));
        MOCK_METHOD(void, OnGameOver, (), (override));
        MOCK_METHOD(void, OnPlayerLeft, (const std::string&), (override));
        MOCK_METHOD(void, OnHostLeft, (), (override));
    };

    class MockGame : public Backend::IGame {
       public:
        MOCK_METHOD(void, AddListener, (std::weak_ptr<Backend::IGameListener>), (override));
        MOCK_METHOD(void, RemoveListener, (Backend::IGameListener*), (override));
        MOCK_METHOD(bool, AddPlayer, (const std::string&), (override));
        MOCK_METHOD(bool, PlaceBet, (int, int), (override));
        MOCK_METHOD(bool, StartRound, (), (override));
        MOCK_METHOD(bool, Hit, (), (override));
        MOCK_METHOD(bool, Stand, (), (override));
        MOCK_METHOD(bool, DoubleDown, (), (override));
        MOCK_METHOD(Backend::EGameState, GetState, (), (const, override));
        MOCK_METHOD(int, GetPlayerCount, (), (const, override));
        MOCK_METHOD(int, GetCurrentPlayerIndex, (), (const, override));
        MOCK_METHOD(const Backend::IPlayer*, GetPlayer, (int), (const, override));
        MOCK_METHOD(const Backend::IPlayer*, GetDealer, (), (const, override));
        MOCK_METHOD(bool, CanPlaceBet, (int), (const, override));
        MOCK_METHOD(bool, CanDoubleDown, (), (const, override));
    };

    inline bool PumpUntil(const std::function<void()>& update,
                          const std::function<bool()>& done,
                          std::chrono::milliseconds timeout = std::chrono::milliseconds(3000)) {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (true) {
            update();
            if (done()) {
                return true;
            }
            if (std::chrono::steady_clock::now() >= deadline) {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }

    inline sf::Packet Truncate(const sf::Packet& packet, std::size_t size) {
        sf::Packet result;
        if (size > 0) {
            result.append(packet.getData(), size);
        }
        return result;
    }
}  // namespace TestSupport
