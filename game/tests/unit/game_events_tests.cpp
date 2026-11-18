#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include <SFML/Network/Packet.hpp>

#include <Backend/IGame.h>
#include <Backend/net/game_events.h>
#include <Backend/net/protocol.h>

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;
using ::testing::NiceMock;

class GameEventsTest : public ::testing::Test {
   protected:
    void SetUp() override {
        game = IGame::Produce(
            std::make_shared<ScriptedDeck>(ArrangeDeck({{V(10), V(9)}}, {V(10), V(7)})));
        game->AddPlayer("Ana");
        encoder = std::make_unique<EventEncoder>(*game, [this](sf::Packet& packet) {
            packets.push_back(packet);
        });
    }

    std::optional<DecodedEvent> DecodeSent(std::size_t index) {
        if (index >= packets.size()) {
            return std::nullopt;
        }
        sf::Packet packet = packets[index];
        const auto type = ReadMessageType(packet);
        if (!type || *type != EMessageType::GameEvent) {
            return std::nullopt;
        }
        return DecodeEvent(packet);
    }

    void ExpectSnapshotOfTheGame(const DecodedEvent& decoded) {
        ASSERT_EQ(decoded.snapshot.players.size(), 1u);
        EXPECT_EQ(decoded.snapshot.players[0].name, "Ana");
        EXPECT_EQ(decoded.snapshot.players[0].balance, kStartingBalance);
        EXPECT_EQ(decoded.snapshot.state, EGameState::WaitingForPlayers);
    }

    std::shared_ptr<IGame> game;
    std::unique_ptr<EventEncoder> encoder;
    std::vector<sf::Packet> packets;
    NiceMock<MockGameListener> listener;
};

TEST_F(GameEventsTest, StateChangedRoundTrip) {
    encoder->OnGameStateChanged(EGameState::Betting);
    ASSERT_EQ(packets.size(), 1u);
    auto decoded = DecodeSent(0);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_CALL(listener, OnGameStateChanged(EGameState::Betting)).Times(1);

    decoded->notify(listener);

    ExpectSnapshotOfTheGame(*decoded);
}

TEST_F(GameEventsTest, PlayerAddedRoundTrip) {
    encoder->OnPlayerAdded(0);
    auto decoded = DecodeSent(0);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_CALL(listener, OnPlayerAdded(0)).Times(1);

    decoded->notify(listener);

    ExpectSnapshotOfTheGame(*decoded);
}

TEST_F(GameEventsTest, BetPlacedRoundTrip) {
    encoder->OnBetPlaced(0, 250);
    auto decoded = DecodeSent(0);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_CALL(listener, OnBetPlaced(0, 250)).Times(1);

    decoded->notify(listener);
}

TEST_F(GameEventsTest, PlayerCardDealtRoundTrip) {
    encoder->OnPlayerCardDealt(0, Make(ERank::Queen, ESuit::Diamonds));
    auto decoded = DecodeSent(0);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_CALL(listener, OnPlayerCardDealt(0, Make(ERank::Queen, ESuit::Diamonds))).Times(1);

    decoded->notify(listener);
}

TEST_F(GameEventsTest, DealerCardDealtFaceUpRoundTrip) {
    encoder->OnDealerCardDealt(V(8), false);
    auto decoded = DecodeSent(0);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_CALL(listener, OnDealerCardDealt(V(8), false)).Times(1);

    decoded->notify(listener);
}

TEST_F(GameEventsTest, DealerCardRevealedRoundTrip) {
    encoder->OnDealerCardRevealed(V(11));
    auto decoded = DecodeSent(0);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_CALL(listener, OnDealerCardRevealed(V(11))).Times(1);

    decoded->notify(listener);
}

TEST_F(GameEventsTest, TurnChangedRoundTrip) {
    encoder->OnTurnChanged(0);
    auto decoded = DecodeSent(0);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_CALL(listener, OnTurnChanged(0)).Times(1);

    decoded->notify(listener);
}

TEST_F(GameEventsTest, RoundEndedRoundTrip) {
    for (const auto result : {ERoundResult::Win, ERoundResult::Lose, ERoundResult::Push,
                              ERoundResult::Blackjack}) {
        packets.clear();
        encoder->OnRoundEnded(0, result);
        auto decoded = DecodeSent(0);
        ASSERT_TRUE(decoded.has_value());
        EXPECT_CALL(listener, OnRoundEnded(0, result)).Times(1);

        decoded->notify(listener);
    }
}

TEST_F(GameEventsTest, EachAnnouncementIsSentAsOneMessage) {
    encoder->OnPlayerAdded(0);
    encoder->OnTurnChanged(0);
    encoder->OnRoundEnded(0, ERoundResult::Win);

    EXPECT_EQ(packets.size(), 3u);
}

TEST_F(GameEventsTest, SnapshotReflectsTheGameAtAnnouncementTime) {
    game->StartRound();
    game->PlaceBet(0, 100);
    packets.clear();

    encoder->OnTurnChanged(0);
    auto decoded = DecodeSent(0);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->snapshot.state, EGameState::PlayersTurn);
    EXPECT_EQ(decoded->snapshot.currentPlayer, 0);
    ASSERT_EQ(decoded->snapshot.dealer.hand.size(), 1u);
    EXPECT_EQ(decoded->snapshot.players[0].bet, 100);
}

TEST_F(GameEventsTest, EmptyPayloadIsRejected) {
    sf::Packet packet;

    EXPECT_FALSE(DecodeEvent(packet).has_value());
}

TEST_F(GameEventsTest, UnknownEventTypeIsRejected) {
    sf::Packet packet;
    packet << static_cast<std::uint8_t>(200);

    EXPECT_FALSE(DecodeEvent(packet).has_value());
}

TEST_F(GameEventsTest, InvalidStateInPayloadIsRejected) {
    sf::Packet packet;
    packet << static_cast<std::uint8_t>(EEventType::StateChanged)
           << static_cast<std::uint8_t>(200);

    EXPECT_FALSE(DecodeEvent(packet).has_value());
}

TEST_F(GameEventsTest, EveryTruncatedMessageIsRejected) {
    encoder->OnPlayerCardDealt(0, V(10));
    ASSERT_EQ(packets.size(), 1u);
    const sf::Packet& full = packets[0];

    for (std::size_t size = 1; size < full.getDataSize(); ++size) {
        sf::Packet cut = Truncate(full, size);
        ASSERT_TRUE(ReadMessageType(cut).has_value());

        EXPECT_FALSE(DecodeEvent(cut).has_value()) << "size=" << size;
    }
}
