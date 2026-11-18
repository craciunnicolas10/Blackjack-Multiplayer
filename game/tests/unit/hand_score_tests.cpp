#include <vector>

#include <gtest/gtest.h>

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;

TEST(HandScoreTest, NumberCardsAreWorthTheirNumber) {
    for (int value = 2; value <= 10; ++value) {
        EXPECT_EQ(CardValue(static_cast<ERank>(value)), value);
    }
}

TEST(HandScoreTest, FaceCardsAreWorthTen) {
    EXPECT_EQ(CardValue(ERank::Jack), 10);
    EXPECT_EQ(CardValue(ERank::Queen), 10);
    EXPECT_EQ(CardValue(ERank::King), 10);
}

TEST(HandScoreTest, AceIsWorthEleven) {
    EXPECT_EQ(CardValue(ERank::Ace), 11);
}

TEST(HandScoreTest, EmptyHandScoresZero) {
    EXPECT_EQ(ScoreHand({}), 0);
}

TEST(HandScoreTest, SumsPlainCards) {
    EXPECT_EQ(ScoreHand({V(10), V(7)}), 17);
    EXPECT_EQ(ScoreHand({Make(ERank::King), Make(ERank::Queen)}), 20);
}

TEST(HandScoreTest, AceCountsElevenWhenItFits) {
    EXPECT_EQ(ScoreHand({V(11), V(6)}), 17);
    EXPECT_EQ(ScoreHand({V(11), Make(ERank::King)}), 21);
}

TEST(HandScoreTest, AceDropsToOneWhenHandWouldBust) {
    EXPECT_EQ(ScoreHand({V(11), V(6), V(10)}), 17);
}

TEST(HandScoreTest, MultipleAcesAreReducedAsNeeded) {
    EXPECT_EQ(ScoreHand({V(11), V(11)}), 12);
    EXPECT_EQ(ScoreHand({V(11), V(11), V(11)}), 13);
    EXPECT_EQ(ScoreHand({V(11), V(11), V(11), V(11), V(8)}), 12);
}

TEST(HandScoreTest, ScoreAbove21IsReportedAsIs) {
    EXPECT_EQ(ScoreHand({V(10), V(10), V(5)}), 25);
}

TEST(HandScoreTest, ThreeCardTwentyOne) {
    EXPECT_EQ(ScoreHand({V(7), V(7), V(7)}), 21);
}

TEST(HandScoreTest, BlackjackNeedsAceAndTenValueCard) {
    EXPECT_TRUE(IsBlackjack({V(11), Make(ERank::King)}));
    EXPECT_TRUE(IsBlackjack({Make(ERank::Queen), V(11)}));
    EXPECT_TRUE(IsBlackjack({V(11), V(10)}));
}

TEST(HandScoreTest, NonBlackjackHands) {
    EXPECT_FALSE(IsBlackjack({}));
    EXPECT_FALSE(IsBlackjack({V(11)}));
    EXPECT_FALSE(IsBlackjack({V(10), V(10)}));
    EXPECT_FALSE(IsBlackjack({V(7), V(7), V(7)}));
    EXPECT_FALSE(IsBlackjack({V(5), V(6), V(10)}));
}
