#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <SFML/Network/Packet.hpp>

#include <Backend/net/game_events.h>
#include <Backend/net/game_snapshot.h>
#include <Backend/net/protocol.h>
#include <Backend/net/remote_game.h>

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;
using ::testing::NiceMock;

namespace {
    PlayerSnapshot MakePlayer(const std::string& name, std::vector<Card> hand, int score,
                              int balance, int bet, bool canPlaceBet) {
        PlayerSnapshot player;
        player.name = name;
        player.hand = std::move(hand);
        player.score = score;
        player.balance = balance;
        player.bet = bet;
        player.canPlaceBet = canPlaceBet;
        return player;
    }

    GameSnapshot MakeSnapshot() {
        GameSnapshot snapshot;
        snapshot.state = EGameState::PlayersTurn;
        snapshot.currentPlayer = 1;
        snapshot.canDoubleDown = true;
        snapshot.players.push_back(MakePlayer("Ana", {V(10), V(7)}, 17, 900, 100, true));
        snapshot.players.push_back(MakePlayer("Bob", {V(10), V(6)}, 16, 400, 500, false));
        snapshot.dealer = MakePlayer("Dealer", {V(9)}, 9, 0, 0, false);
        return snapshot;
    }

    DecodedEvent MakeEvent(std::function<void(IGameListener&)> notify) {
        DecodedEvent event;
        event.notify = std::move(notify);
        event.snapshot = MakeSnapshot();
        return event;
    }
}  // namespace

class RemoteGameTest : public ::testing::Test {
   protected:
    RemoteGameTest()
        : remote(std::make_shared<RemoteGame>([this](sf::Packet& packet) {
              const auto type = ReadMessageType(packet);
              if (type) {
                  sent.push_back(*type);
              }
          })) {}

    std::vector<EMessageType> sent;
    std::shared_ptr<RemoteGame> remote;
};

TEST_F(RemoteGameTest, StartsWithAnEmptyTable) {
    EXPECT_EQ(remote->GetState(), EGameState::WaitingForPlayers);
    EXPECT_EQ(remote->GetPlayerCount(), 0);
    EXPECT_EQ(remote->GetCurrentPlayerIndex(), -1);
    EXPECT_EQ(remote->GetPlayer(0), nullptr);
    EXPECT_NE(remote->GetDealer(), nullptr);
    EXPECT_FALSE(remote->CanPlaceBet(0));
    EXPECT_FALSE(remote->CanDoubleDown());
}

TEST_F(RemoteGameTest, PlayersCannotBeAddedLocally) {
    EXPECT_FALSE(remote->AddPlayer("Ana"));
    EXPECT_TRUE(sent.empty());
}

TEST_F(RemoteGameTest, ActionsAreSentToTheHost) {
    EXPECT_TRUE(remote->PlaceBet(1, 100));
    EXPECT_TRUE(remote->Hit());
    EXPECT_TRUE(remote->Stand());
    EXPECT_TRUE(remote->DoubleDown());
    EXPECT_TRUE(remote->StartRound());

    const std::vector<EMessageType> expected = {EMessageType::PlaceBet, EMessageType::Hit,
                                                EMessageType::Stand, EMessageType::DoubleDown,
                                                EMessageType::NextRound};
    EXPECT_EQ(sent, expected);
}

TEST_F(RemoteGameTest, ApplyEventReplacesTheSnapshot) {
    remote->ApplyEvent(MakeEvent([](IGameListener&) {}));

    EXPECT_EQ(remote->GetState(), EGameState::PlayersTurn);
    EXPECT_EQ(remote->GetPlayerCount(), 2);
    EXPECT_EQ(remote->GetCurrentPlayerIndex(), 1);
    EXPECT_TRUE(remote->CanDoubleDown());
}

TEST_F(RemoteGameTest, PlayersAreReadFromTheSnapshot) {
    remote->ApplyEvent(MakeEvent([](IGameListener&) {}));

    const IPlayer* ana = remote->GetPlayer(0);
    ASSERT_NE(ana, nullptr);
    EXPECT_EQ(ana->GetName(), "Ana");
    EXPECT_EQ(ana->GetBalance(), 900);
    EXPECT_EQ(ana->GetBet(), 100);
    EXPECT_EQ(ana->GetScore(), 17);
    EXPECT_EQ(ana->GetHand().size(), 2u);
    ASSERT_NE(remote->GetPlayer(1), nullptr);
    EXPECT_EQ(remote->GetPlayer(1)->GetName(), "Bob");
}

TEST_F(RemoteGameTest, InvalidPlayerIndexGivesNull) {
    remote->ApplyEvent(MakeEvent([](IGameListener&) {}));

    EXPECT_EQ(remote->GetPlayer(-1), nullptr);
    EXPECT_EQ(remote->GetPlayer(2), nullptr);
}

TEST_F(RemoteGameTest, DealerIsReadFromTheSnapshot) {
    remote->ApplyEvent(MakeEvent([](IGameListener&) {}));

    const IPlayer* dealer = remote->GetDealer();
    ASSERT_NE(dealer, nullptr);
    EXPECT_EQ(dealer->GetHand().size(), 1u);
    EXPECT_EQ(dealer->GetScore(), 9);
}

TEST_F(RemoteGameTest, CanPlaceBetComesFromTheSnapshot) {
    remote->ApplyEvent(MakeEvent([](IGameListener&) {}));

    EXPECT_TRUE(remote->CanPlaceBet(0));
    EXPECT_FALSE(remote->CanPlaceBet(1));
    EXPECT_FALSE(remote->CanPlaceBet(-1));
    EXPECT_FALSE(remote->CanPlaceBet(2));
}

TEST_F(RemoteGameTest, ListenersAreNotifiedAfterTheStateIsUpdated) {
    auto listener = std::make_shared<NiceMock<MockGameListener>>();
    remote->AddListener(listener);
    EGameState seenState = EGameState::WaitingForPlayers;
    EXPECT_CALL(*listener, OnTurnChanged(1)).WillOnce([&](int) { seenState = remote->GetState(); });

    remote->ApplyEvent(MakeEvent([](IGameListener& target) { target.OnTurnChanged(1); }));

    EXPECT_EQ(seenState, EGameState::PlayersTurn);
}

TEST_F(RemoteGameTest, RemovedListenerIsNotNotified) {
    auto listener = std::make_shared<NiceMock<MockGameListener>>();
    remote->AddListener(listener);
    remote->RemoveListener(listener.get());
    EXPECT_CALL(*listener, OnTurnChanged(testing::_)).Times(0);

    remote->ApplyEvent(MakeEvent([](IGameListener& target) { target.OnTurnChanged(1); }));
}

TEST_F(RemoteGameTest, AllListenersReceiveTheEvent) {
    auto first = std::make_shared<NiceMock<MockGameListener>>();
    auto second = std::make_shared<NiceMock<MockGameListener>>();
    remote->AddListener(first);
    remote->AddListener(second);
    EXPECT_CALL(*first, OnBetPlaced(0, 100)).Times(1);
    EXPECT_CALL(*second, OnBetPlaced(0, 100)).Times(1);

    remote->ApplyEvent(MakeEvent([](IGameListener& target) { target.OnBetPlaced(0, 100); }));
}
