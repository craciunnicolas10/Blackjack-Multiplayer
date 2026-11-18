#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <Backend/net/hosted_game.h>

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;
using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

class HostedGameTest : public ::testing::Test {
   protected:
    HostedGameTest()
        : inner(std::make_shared<StrictMock<MockGame>>()),
          hosted(inner, [this]() { ++refusedCount; }) {}

    std::shared_ptr<StrictMock<MockGame>> inner;
    int refusedCount = 0;
    HostedGame hosted;
};

TEST_F(HostedGameTest, ForwardsListenerManagement) {
    auto listener = std::make_shared<MockGameListener>();
    EXPECT_CALL(*inner, AddListener(_)).Times(1);
    EXPECT_CALL(*inner, RemoveListener(listener.get())).Times(1);

    hosted.AddListener(listener);
    hosted.RemoveListener(listener.get());
}

TEST_F(HostedGameTest, ForwardsPlayerActions) {
    EXPECT_CALL(*inner, AddPlayer("Ana")).WillOnce(Return(true));
    EXPECT_CALL(*inner, PlaceBet(2, 150)).WillOnce(Return(true));
    EXPECT_CALL(*inner, Hit()).WillOnce(Return(false));
    EXPECT_CALL(*inner, Stand()).WillOnce(Return(true));
    EXPECT_CALL(*inner, DoubleDown()).WillOnce(Return(false));

    EXPECT_TRUE(hosted.AddPlayer("Ana"));
    EXPECT_TRUE(hosted.PlaceBet(2, 150));
    EXPECT_FALSE(hosted.Hit());
    EXPECT_TRUE(hosted.Stand());
    EXPECT_FALSE(hosted.DoubleDown());
    EXPECT_EQ(refusedCount, 0);
}

TEST_F(HostedGameTest, ForwardsQueries) {
    EXPECT_CALL(*inner, GetState()).WillOnce(Return(EGameState::Betting));
    EXPECT_CALL(*inner, GetPlayerCount()).WillOnce(Return(3));
    EXPECT_CALL(*inner, GetCurrentPlayerIndex()).WillOnce(Return(1));
    EXPECT_CALL(*inner, GetPlayer(1)).WillOnce(Return(nullptr));
    EXPECT_CALL(*inner, GetDealer()).WillOnce(Return(nullptr));
    EXPECT_CALL(*inner, CanPlaceBet(2)).WillOnce(Return(true));
    EXPECT_CALL(*inner, CanDoubleDown()).WillOnce(Return(true));

    EXPECT_EQ(hosted.GetState(), EGameState::Betting);
    EXPECT_EQ(hosted.GetPlayerCount(), 3);
    EXPECT_EQ(hosted.GetCurrentPlayerIndex(), 1);
    EXPECT_EQ(hosted.GetPlayer(1), nullptr);
    EXPECT_EQ(hosted.GetDealer(), nullptr);
    EXPECT_TRUE(hosted.CanPlaceBet(2));
    EXPECT_TRUE(hosted.CanDoubleDown());
}

TEST_F(HostedGameTest, AcceptedStartRoundDoesNotNotify) {
    EXPECT_CALL(*inner, StartRound()).WillOnce(Return(true));

    EXPECT_TRUE(hosted.StartRound());
    EXPECT_EQ(refusedCount, 0);
}
