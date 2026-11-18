#include <gtest/gtest.h>

#include <Backend/game/dealer.h>

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;

TEST(DealerTest, StartsEmptyAndWithoutMoney) {
    Dealer dealer;

    EXPECT_EQ(dealer.GetBalance(), 0);
    EXPECT_TRUE(dealer.GetHand().empty());
    EXPECT_FALSE(dealer.HasHiddenCard());
    EXPECT_EQ(dealer.GetScore(), 0);
}

TEST(DealerTest, VisibleCardsAreShown) {
    Dealer dealer;

    dealer.AddCard(V(10));
    dealer.AddCard(V(8));

    EXPECT_EQ(dealer.GetHand().size(), 2u);
    EXPECT_EQ(dealer.GetScore(), 18);
}

TEST(DealerTest, HoleCardIsLeftOutOfHandAndScore) {
    Dealer dealer;
    dealer.AddCard(V(10));

    dealer.AddHoleCard(V(7));

    EXPECT_TRUE(dealer.HasHiddenCard());
    ASSERT_EQ(dealer.GetHand().size(), 1u);
    EXPECT_EQ(dealer.GetHand()[0], V(10));
    EXPECT_EQ(dealer.GetScore(), 10);
    EXPECT_FALSE(dealer.IsBusted());
}

TEST(DealerTest, FullHandIncludesHoleCard) {
    Dealer dealer;
    dealer.AddCard(V(10));
    dealer.AddHoleCard(V(7));

    ASSERT_EQ(dealer.GetFullHand().size(), 2u);
    EXPECT_EQ(dealer.GetFullHand()[1], V(7));
}

TEST(DealerTest, RevealReturnsHoleCardAndShowsIt) {
    Dealer dealer;
    dealer.AddCard(V(10));
    dealer.AddHoleCard(V(7));

    const Card revealed = dealer.RevealHoleCard();

    EXPECT_EQ(revealed, V(7));
    EXPECT_FALSE(dealer.HasHiddenCard());
    EXPECT_EQ(dealer.GetHand().size(), 2u);
    EXPECT_EQ(dealer.GetScore(), 17);
}

TEST(DealerTest, HiddenBlackjackIsStillDetectedByRules) {
    Dealer dealer;
    dealer.AddCard(V(11));
    dealer.AddHoleCard(Make(ERank::King));

    EXPECT_TRUE(dealer.HasBlackjack());
    EXPECT_EQ(dealer.GetScore(), 11);
}
