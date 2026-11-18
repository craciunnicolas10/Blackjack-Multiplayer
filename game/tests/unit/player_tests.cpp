#include <memory>

#include <gtest/gtest.h>

#include <Backend/IPlayer.h>
#include <Backend/game/player.h>

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;

TEST(PlayerTest, StartsWithNameAndBalance) {
    Player player("Ana", 1000);

    EXPECT_EQ(player.GetName(), "Ana");
    EXPECT_EQ(player.GetBalance(), 1000);
    EXPECT_EQ(player.GetBet(), 0);
    EXPECT_TRUE(player.GetHand().empty());
    EXPECT_TRUE(player.GetFullHand().empty());
    EXPECT_EQ(player.GetScore(), 0);
    EXPECT_FALSE(player.IsBusted());
    EXPECT_FALSE(player.IsInRound());
    EXPECT_FALSE(player.HasBlackjack());
}

TEST(PlayerTest, AddCardUpdatesHandAndScore) {
    Player player("Ana", 1000);

    player.AddCard(V(10));
    player.AddCard(V(7));

    ASSERT_EQ(player.GetHand().size(), 2u);
    EXPECT_EQ(player.GetHand()[0], V(10));
    EXPECT_EQ(player.GetHand()[1], V(7));
    EXPECT_EQ(player.GetFullHand(), player.GetHand());
    EXPECT_EQ(player.GetScore(), 17);
}

TEST(PlayerTest, BustsAboveTwentyOne) {
    Player player("Ana", 1000);

    player.AddCard(V(10));
    player.AddCard(V(10));
    EXPECT_FALSE(player.IsBusted());

    player.AddCard(V(2));
    EXPECT_TRUE(player.IsBusted());
    EXPECT_EQ(player.GetScore(), 22);
}

TEST(PlayerTest, TwentyOneIsNotBusted) {
    Player player("Ana", 1000);

    player.AddCard(V(7));
    player.AddCard(V(7));
    player.AddCard(V(7));

    EXPECT_FALSE(player.IsBusted());
    EXPECT_FALSE(player.HasBlackjack());
}

TEST(PlayerTest, BlackjackOnlyFromFirstTwoCards) {
    Player player("Ana", 1000);

    player.AddCard(V(11));
    player.AddCard(Make(ERank::King));

    EXPECT_TRUE(player.HasBlackjack());
}

TEST(PlayerTest, AddToBetMovesMoneyFromBalance) {
    Player player("Ana", 1000);

    player.AddToBet(100);
    EXPECT_EQ(player.GetBalance(), 900);
    EXPECT_EQ(player.GetBet(), 100);
    EXPECT_TRUE(player.IsInRound());

    player.AddToBet(100);
    EXPECT_EQ(player.GetBalance(), 800);
    EXPECT_EQ(player.GetBet(), 200);
}

TEST(PlayerTest, ReceiveIncreasesBalance) {
    Player player("Ana", 500);

    player.Receive(250);

    EXPECT_EQ(player.GetBalance(), 750);
}

TEST(PlayerTest, ResetRoundClearsHandAndBetButKeepsBalance) {
    Player player("Ana", 1000);
    player.AddToBet(100);
    player.AddCard(V(10));
    player.AddCard(V(9));

    player.ResetRound();

    EXPECT_TRUE(player.GetHand().empty());
    EXPECT_EQ(player.GetBet(), 0);
    EXPECT_FALSE(player.IsInRound());
    EXPECT_EQ(player.GetBalance(), 900);
    EXPECT_EQ(player.GetScore(), 0);
}

TEST(PlayerTest, WorksThroughReadOnlyInterface) {
    std::unique_ptr<IPlayer> player = std::make_unique<Player>("Bob", 321);

    EXPECT_EQ(player->GetName(), "Bob");
    EXPECT_EQ(player->GetBalance(), 321);
    EXPECT_EQ(player->GetBet(), 0);
    EXPECT_FALSE(player->IsBusted());
}
