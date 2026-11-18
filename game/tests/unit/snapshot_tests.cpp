#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <SFML/Network/Packet.hpp>

#include <Backend/IGame.h>
#include <Backend/net/game_snapshot.h>
#include <Backend/net/protocol.h>

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;

namespace {
    PlayerSnapshot MakePlayer(const std::string& name, std::vector<Card> hand, int score,
                              int balance, int bet, bool busted, bool canPlaceBet) {
        PlayerSnapshot player;
        player.name = name;
        player.hand = std::move(hand);
        player.score = score;
        player.balance = balance;
        player.bet = bet;
        player.busted = busted;
        player.canPlaceBet = canPlaceBet;
        return player;
    }

    GameSnapshot MakeSnapshot() {
        GameSnapshot snapshot;
        snapshot.state = EGameState::PlayersTurn;
        snapshot.currentPlayer = 1;
        snapshot.canDoubleDown = true;
        snapshot.players.push_back(MakePlayer("Ana", {V(10), V(7)}, 17, 900, 100, false, false));
        snapshot.players.push_back(
            MakePlayer("Bob", {V(10), V(6), V(9)}, 25, 400, 500, true, true));
        snapshot.dealer = MakePlayer("Dealer", {V(9)}, 9, 0, 0, false, false);
        return snapshot;
    }

    void ExpectSamePlayer(const PlayerSnapshot& actual, const PlayerSnapshot& expected) {
        EXPECT_EQ(actual.name, expected.name);
        EXPECT_EQ(actual.hand, expected.hand);
        EXPECT_EQ(actual.score, expected.score);
        EXPECT_EQ(actual.balance, expected.balance);
        EXPECT_EQ(actual.bet, expected.bet);
        EXPECT_EQ(actual.busted, expected.busted);
        EXPECT_EQ(actual.canPlaceBet, expected.canPlaceBet);
    }
}  // namespace

TEST(PlayerSnapshotTest, ExposesFieldsThroughReadOnlyInterface) {
    const PlayerSnapshot snapshot = MakePlayer("Ana", {V(10), V(2)}, 12, 800, 200, true, false);
    const IPlayer& player = snapshot;

    EXPECT_EQ(player.GetName(), "Ana");
    EXPECT_EQ(player.GetHand(), snapshot.hand);
    EXPECT_EQ(player.GetScore(), 12);
    EXPECT_EQ(player.GetBalance(), 800);
    EXPECT_EQ(player.GetBet(), 200);
    EXPECT_TRUE(player.IsBusted());
}

TEST(SnapshotSerializationTest, RoundTripsAFullSnapshot) {
    const GameSnapshot original = MakeSnapshot();
    sf::Packet packet;
    WriteSnapshot(packet, original);
    GameSnapshot read;

    ASSERT_TRUE(ReadSnapshot(packet, read));

    EXPECT_EQ(read.state, original.state);
    EXPECT_EQ(read.currentPlayer, original.currentPlayer);
    EXPECT_EQ(read.canDoubleDown, original.canDoubleDown);
    ASSERT_EQ(read.players.size(), original.players.size());
    for (std::size_t i = 0; i < original.players.size(); ++i) {
        ExpectSamePlayer(read.players[i], original.players[i]);
    }
    ExpectSamePlayer(read.dealer, original.dealer);
}

TEST(SnapshotSerializationTest, RoundTripsAnEmptySnapshot) {
    const GameSnapshot original;
    sf::Packet packet;
    WriteSnapshot(packet, original);
    GameSnapshot read = MakeSnapshot();

    ASSERT_TRUE(ReadSnapshot(packet, read));

    EXPECT_EQ(read.state, EGameState::WaitingForPlayers);
    EXPECT_EQ(read.currentPlayer, -1);
    EXPECT_FALSE(read.canDoubleDown);
    EXPECT_TRUE(read.players.empty());
    EXPECT_TRUE(read.dealer.hand.empty());
}

TEST(SnapshotSerializationTest, RoundTripsUtf8Names) {
    GameSnapshot original = MakeSnapshot();
    original.players[0].name = "\xC8\x98tefan";
    sf::Packet packet;
    WriteSnapshot(packet, original);
    GameSnapshot read;

    ASSERT_TRUE(ReadSnapshot(packet, read));

    EXPECT_EQ(read.players[0].name, original.players[0].name);
}

TEST(SnapshotSerializationTest, EveryTruncatedPrefixIsRejected) {
    sf::Packet full;
    WriteSnapshot(full, MakeSnapshot());

    for (std::size_t size = 0; size < full.getDataSize(); ++size) {
        sf::Packet cut = Truncate(full, size);
        GameSnapshot read;

        EXPECT_FALSE(ReadSnapshot(cut, read)) << "size=" << size;
    }
}

TEST(SnapshotSerializationTest, OversizedHandIsRejected) {
    GameSnapshot original = MakeSnapshot();
    original.players[0].hand.assign(static_cast<std::size_t>(kMaxHandSize) + 1, V(2));
    sf::Packet packet;
    WriteSnapshot(packet, original);
    GameSnapshot read;

    EXPECT_FALSE(ReadSnapshot(packet, read));
}

TEST(SnapshotSerializationTest, InvalidStateByteIsRejected) {
    sf::Packet packet;
    packet << static_cast<std::uint8_t>(200);
    GameSnapshot read;

    EXPECT_FALSE(ReadSnapshot(packet, read));
}

class TakeSnapshotTest : public ::testing::Test {
   protected:
    void SetUp() override {
        deck = std::make_shared<ScriptedDeck>(
            ArrangeDeck({{V(10), V(10)}, {V(10), V(6)}}, {V(10), V(7)}));
        game = IGame::Produce(deck);
        game->AddPlayer("Ana");
        game->AddPlayer("Bob");
    }

    std::shared_ptr<ScriptedDeck> deck;
    std::shared_ptr<IGame> game;
};

TEST_F(TakeSnapshotTest, CapturesLobbyState) {
    const GameSnapshot snapshot = TakeSnapshot(*game);

    EXPECT_EQ(snapshot.state, EGameState::WaitingForPlayers);
    EXPECT_EQ(snapshot.currentPlayer, -1);
    ASSERT_EQ(snapshot.players.size(), 2u);
    EXPECT_EQ(snapshot.players[0].name, "Ana");
    EXPECT_EQ(snapshot.players[1].name, "Bob");
    EXPECT_EQ(snapshot.players[0].balance, kStartingBalance);
    EXPECT_TRUE(snapshot.players[0].hand.empty());
}

TEST_F(TakeSnapshotTest, CapturesBettingPermissions) {
    game->StartRound();
    game->PlaceBet(0, 100);

    const GameSnapshot snapshot = TakeSnapshot(*game);

    EXPECT_EQ(snapshot.state, EGameState::Betting);
    EXPECT_FALSE(snapshot.players[0].canPlaceBet);
    EXPECT_TRUE(snapshot.players[1].canPlaceBet);
    EXPECT_EQ(snapshot.players[0].bet, 100);
    EXPECT_EQ(snapshot.players[0].balance, kStartingBalance - 100);
}

TEST_F(TakeSnapshotTest, HidesTheDealerHoleCard) {
    game->StartRound();
    game->PlaceBet(0, 100);
    game->PlaceBet(1, 100);

    const GameSnapshot snapshot = TakeSnapshot(*game);

    EXPECT_EQ(snapshot.state, EGameState::PlayersTurn);
    EXPECT_EQ(snapshot.currentPlayer, 0);
    ASSERT_EQ(snapshot.dealer.hand.size(), 1u);
    EXPECT_EQ(snapshot.dealer.hand[0], V(10));
    EXPECT_EQ(snapshot.dealer.score, 10);
    EXPECT_EQ(snapshot.players[0].hand.size(), 2u);
    EXPECT_EQ(snapshot.players[0].score, 20);
    EXPECT_EQ(snapshot.players[1].score, 16);
    EXPECT_EQ(snapshot.canDoubleDown, game->CanDoubleDown());
}

TEST_F(TakeSnapshotTest, ShowsFullDealerHandOnceRoundIsOver) {
    game->StartRound();
    game->PlaceBet(0, 100);
    game->PlaceBet(1, 100);
    game->Stand();
    game->Stand();

    const GameSnapshot snapshot = TakeSnapshot(*game);

    EXPECT_EQ(snapshot.state, EGameState::RoundOver);
    EXPECT_EQ(snapshot.dealer.hand.size(), 2u);
    EXPECT_EQ(snapshot.dealer.score, 17);
    EXPECT_EQ(snapshot.currentPlayer, -1);
}

TEST_F(TakeSnapshotTest, SerializedSnapshotMatchesTheGame) {
    game->StartRound();
    game->PlaceBet(0, 100);
    game->PlaceBet(1, 250);
    sf::Packet packet;
    WriteSnapshot(packet, TakeSnapshot(*game));
    GameSnapshot read;

    ASSERT_TRUE(ReadSnapshot(packet, read));

    for (int i = 0; i < game->GetPlayerCount(); ++i) {
        const IPlayer* expected = game->GetPlayer(i);
        const auto index = static_cast<std::size_t>(i);
        EXPECT_EQ(read.players[index].name, expected->GetName());
        EXPECT_EQ(read.players[index].balance, expected->GetBalance());
        EXPECT_EQ(read.players[index].bet, expected->GetBet());
        EXPECT_EQ(read.players[index].hand, expected->GetHand());
    }
}
