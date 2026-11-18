#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <Backend/IGame.h>

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace {
    const std::vector<Card> kDealer17 = {V(10), V(7)};
}

class GameTest : public ::testing::Test {
   protected:
    void Prepare(std::vector<Card> cards, int count = 1) {
        playerCount = count;
        deck = std::make_shared<ScriptedDeck>(std::move(cards));
        game = IGame::Produce(deck);
        ASSERT_NE(game, nullptr);
        listener = std::make_shared<NiceMock<MockGameListener>>();
        game->AddListener(listener);
        for (int i = 0; i < count; ++i) {
            ASSERT_TRUE(game->AddPlayer("P" + std::to_string(i)));
        }
    }

    void BetAll(int amount) {
        for (int i = 0; i < playerCount; ++i) {
            ASSERT_TRUE(game->PlaceBet(i, amount));
        }
    }

    void BeginRound(int amount) {
        ASSERT_TRUE(game->StartRound());
        ASSERT_NO_FATAL_FAILURE(BetAll(amount));
    }

    void StandAll() {
        while (game->GetState() == EGameState::PlayersTurn) {
            ASSERT_TRUE(game->Stand());
        }
    }

    int Balance(int index) const {
        return game->GetPlayer(index)->GetBalance();
    }

    int playerCount = 0;
    std::shared_ptr<ScriptedDeck> deck;
    std::shared_ptr<IGame> game;
    std::shared_ptr<NiceMock<MockGameListener>> listener;
};

TEST(GameProduceTest, NullDeckGivesNullGame) {
    EXPECT_EQ(IGame::Produce(std::shared_ptr<IDeck>{}), nullptr);
}

TEST(GameProduceTest, DefaultGameStartsEmpty) {
    const auto game = IGame::Produce();

    ASSERT_NE(game, nullptr);
    EXPECT_EQ(game->GetState(), EGameState::WaitingForPlayers);
    EXPECT_EQ(game->GetPlayerCount(), 0);
    EXPECT_EQ(game->GetCurrentPlayerIndex(), -1);
    EXPECT_EQ(game->GetPlayer(0), nullptr);
    EXPECT_NE(game->GetDealer(), nullptr);
    EXPECT_FALSE(game->CanPlaceBet(0));
    EXPECT_FALSE(game->CanDoubleDown());
}

TEST(GameProduceTest, DefaultGamePlaysAFullRoundWithRandomDeck) {
    const auto game = IGame::Produce();
    ASSERT_TRUE(game->AddPlayer("Ana"));
    ASSERT_TRUE(game->AddPlayer("Bob"));
    ASSERT_TRUE(game->StartRound());
    ASSERT_TRUE(game->PlaceBet(0, kMinBet));
    ASSERT_TRUE(game->PlaceBet(1, kMinBet));

    while (game->GetState() == EGameState::PlayersTurn) {
        ASSERT_TRUE(game->Stand());
    }

    EXPECT_EQ(game->GetState(), EGameState::RoundOver);
    EXPECT_EQ(game->GetCurrentPlayerIndex(), -1);
}

TEST_F(GameTest, AddPlayerRegistersPlayersWithStartingBalance) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 0));
    EXPECT_CALL(*listener, OnPlayerAdded(0)).Times(1);
    EXPECT_CALL(*listener, OnPlayerAdded(1)).Times(1);

    EXPECT_TRUE(game->AddPlayer("Ana"));
    EXPECT_TRUE(game->AddPlayer("Bob"));

    EXPECT_EQ(game->GetPlayerCount(), 2);
    ASSERT_NE(game->GetPlayer(0), nullptr);
    EXPECT_EQ(game->GetPlayer(0)->GetName(), "Ana");
    EXPECT_EQ(game->GetPlayer(1)->GetName(), "Bob");
    EXPECT_EQ(Balance(0), kStartingBalance);
    EXPECT_EQ(Balance(1), kStartingBalance);
}

TEST_F(GameTest, AddPlayerRejectsDuplicateNames) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 0));
    ASSERT_TRUE(game->AddPlayer("Ana"));

    EXPECT_FALSE(game->AddPlayer("Ana"));
    EXPECT_EQ(game->GetPlayerCount(), 1);
}

TEST_F(GameTest, AddPlayerRejectsMoreThanMaxPlayers) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, kMaxPlayers));

    EXPECT_FALSE(game->AddPlayer("Extra"));
    EXPECT_EQ(game->GetPlayerCount(), kMaxPlayers);
}

TEST_F(GameTest, AddPlayerRejectedOutsideWaitingState) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 1));
    ASSERT_TRUE(game->StartRound());

    EXPECT_FALSE(game->AddPlayer("Late"));
    EXPECT_EQ(game->GetPlayerCount(), 1);
}

TEST_F(GameTest, GetPlayerReturnsNullForInvalidIndex) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 2));

    EXPECT_EQ(game->GetPlayer(-1), nullptr);
    EXPECT_EQ(game->GetPlayer(2), nullptr);
    EXPECT_EQ(game->GetPlayer(100), nullptr);
}

TEST_F(GameTest, PlayerPointersStayValidWhenPlayersAreAdded) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 1));
    const IPlayer* first = game->GetPlayer(0);

    game->AddPlayer("Second");
    game->AddPlayer("Third");

    EXPECT_EQ(game->GetPlayer(0), first);
    EXPECT_EQ(first->GetName(), "P0");
}

TEST_F(GameTest, ActionsAreRefusedBeforeTheRoundStarts) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 1));

    EXPECT_FALSE(game->PlaceBet(0, 100));
    EXPECT_FALSE(game->Hit());
    EXPECT_FALSE(game->Stand());
    EXPECT_FALSE(game->DoubleDown());
    EXPECT_EQ(game->GetState(), EGameState::WaitingForPlayers);
}

TEST_F(GameTest, StartRoundMovesToBetting) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 1));
    EXPECT_CALL(*listener, OnGameStateChanged(EGameState::Betting)).Times(1);

    EXPECT_TRUE(game->StartRound());

    EXPECT_EQ(game->GetState(), EGameState::Betting);
    EXPECT_TRUE(game->CanPlaceBet(0));
}

TEST_F(GameTest, StartRoundRefusedWhileBetting) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 1));
    ASSERT_TRUE(game->StartRound());

    EXPECT_FALSE(game->StartRound());
    EXPECT_EQ(game->GetState(), EGameState::Betting);
}

TEST_F(GameTest, HitStandDoubleRefusedWhileBetting) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 1));
    ASSERT_TRUE(game->StartRound());

    EXPECT_FALSE(game->Hit());
    EXPECT_FALSE(game->Stand());
    EXPECT_FALSE(game->DoubleDown());
}

TEST_F(GameTest, PlaceBetRejectsInvalidAmounts) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 1));
    ASSERT_TRUE(game->StartRound());

    EXPECT_FALSE(game->PlaceBet(0, 0));
    EXPECT_FALSE(game->PlaceBet(0, -5));
    EXPECT_FALSE(game->PlaceBet(0, kMinBet - 1));
    EXPECT_FALSE(game->PlaceBet(0, kMaxBet + 1));

    EXPECT_EQ(Balance(0), kStartingBalance);
    EXPECT_EQ(game->GetPlayer(0)->GetBet(), 0);
    EXPECT_TRUE(game->CanPlaceBet(0));
}

TEST_F(GameTest, PlaceBetRejectsInvalidPlayerIndex) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 1));
    ASSERT_TRUE(game->StartRound());

    EXPECT_FALSE(game->PlaceBet(-1, 100));
    EXPECT_FALSE(game->PlaceBet(1, 100));
    EXPECT_FALSE(game->CanPlaceBet(-1));
    EXPECT_FALSE(game->CanPlaceBet(1));
}

TEST_F(GameTest, PlaceBetAcceptsMinAndMax) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(8)}, {V(10), V(8)}}, kDealer17), 2));
    ASSERT_TRUE(game->StartRound());

    EXPECT_TRUE(game->PlaceBet(0, kMinBet));
    EXPECT_TRUE(game->PlaceBet(1, kMaxBet));
    EXPECT_EQ(game->GetPlayer(0)->GetBet(), kMinBet);
    EXPECT_EQ(game->GetPlayer(1)->GetBet(), kMaxBet);
}

TEST_F(GameTest, PlaceBetTwiceIsRefused) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(8)}, {V(10), V(8)}}, kDealer17), 2));
    ASSERT_TRUE(game->StartRound());
    EXPECT_CALL(*listener, OnBetPlaced(0, 100)).Times(1);

    ASSERT_TRUE(game->PlaceBet(0, 100));

    EXPECT_FALSE(game->PlaceBet(0, 100));
    EXPECT_FALSE(game->CanPlaceBet(0));
    EXPECT_TRUE(game->CanPlaceBet(1));
    EXPECT_EQ(Balance(0), kStartingBalance - 100);
    EXPECT_EQ(game->GetState(), EGameState::Betting);
}

TEST_F(GameTest, DealerHoleCardStaysHiddenDuringPlayersTurn) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(10)}}, kDealer17)));

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));

    ASSERT_EQ(game->GetState(), EGameState::PlayersTurn);
    ASSERT_EQ(game->GetDealer()->GetHand().size(), 1u);
    EXPECT_EQ(game->GetDealer()->GetHand()[0], V(10));
    EXPECT_EQ(game->GetDealer()->GetScore(), 10);
}

TEST_F(GameTest, PlayerWinsWhenHigherThanDealer) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(10)}}, kDealer17)));
    EXPECT_CALL(*listener, OnDealerCardRevealed(V(7))).Times(1);
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Win)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    EXPECT_EQ(game->GetState(), EGameState::RoundOver);
    EXPECT_EQ(Balance(0), kStartingBalance + 100);
}

TEST_F(GameTest, PlayerLosesWhenLowerThanDealer) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(8)}}, {V(10), V(9)})));
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Lose)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    EXPECT_EQ(game->GetState(), EGameState::RoundOver);
    EXPECT_EQ(Balance(0), kStartingBalance - 100);
}

TEST_F(GameTest, EqualScoresArePush) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(9)}}, {V(10), V(9)})));
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Push)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    EXPECT_EQ(Balance(0), kStartingBalance);
}

TEST_F(GameTest, DealerDrawsBelowSeventeen) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(10)}}, {V(10), V(6)}, {V(5)})));
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Lose)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    EXPECT_EQ(game->GetDealer()->GetHand().size(), 3u);
    EXPECT_EQ(game->GetDealer()->GetScore(), 21);
    EXPECT_EQ(deck->overdraws, 0);
}

TEST_F(GameTest, DealerStandsOnSeventeen) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(10)}}, kDealer17, {V(2)})));

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    EXPECT_EQ(game->GetDealer()->GetHand().size(), 2u);
    EXPECT_EQ(game->GetDealer()->GetScore(), 17);
}

TEST_F(GameTest, PlayerWinsWhenDealerBusts) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(8)}}, {V(10), V(6)}, {V(10)})));
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Win)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    EXPECT_TRUE(game->GetDealer()->IsBusted());
    EXPECT_EQ(Balance(0), kStartingBalance + 100);
}

TEST_F(GameTest, PlayerBustLosesTheRound) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(6)}}, kDealer17, {V(10)})));
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Lose)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_TRUE(game->Hit());

    EXPECT_TRUE(game->GetPlayer(0)->IsBusted());
    EXPECT_EQ(game->GetState(), EGameState::RoundOver);
    EXPECT_EQ(Balance(0), kStartingBalance - 100);
}

TEST_F(GameTest, SoftAceIsReducedInsteadOfBusting) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(11), V(6)}}, kDealer17, {V(10)})));
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Push)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_TRUE(game->Hit());

    EXPECT_FALSE(game->GetPlayer(0)->IsBusted());
    EXPECT_EQ(game->GetPlayer(0)->GetScore(), 17);
    ASSERT_TRUE(game->Stand());
    EXPECT_EQ(Balance(0), kStartingBalance);
}

TEST_F(GameTest, BlackjackPaysMoreThanARegularWin) {
    ASSERT_NO_FATAL_FAILURE(
        Prepare(ArrangeDeck({{V(11), Make(ERank::King)}}, kDealer17)));
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Blackjack)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    EXPECT_EQ(game->GetState(), EGameState::RoundOver);
    EXPECT_GT(Balance(0), kStartingBalance + 100);
}

TEST_F(GameTest, DealerBlackjackBeatsPlayerTwenty) {
    ASSERT_NO_FATAL_FAILURE(
        Prepare(ArrangeDeck({{V(10), V(10)}}, {V(11), Make(ERank::King)})));
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Lose)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    EXPECT_EQ(Balance(0), kStartingBalance - 100);
}

TEST_F(GameTest, BothBlackjackIsPush) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(11), Make(ERank::King)}},
                                                {V(11), Make(ERank::Queen)})));
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Push)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    EXPECT_EQ(Balance(0), kStartingBalance);
}

TEST_F(GameTest, DoubleDownDoublesTheBetAndDrawsOneCard) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(5), V(6)}}, kDealer17, {V(10)})));
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Win)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_TRUE(game->CanDoubleDown());
    ASSERT_TRUE(game->DoubleDown());

    EXPECT_EQ(game->GetPlayer(0)->GetHand().size(), 3u);
    EXPECT_EQ(game->GetState(), EGameState::RoundOver);
    EXPECT_EQ(Balance(0), kStartingBalance + 200);
}

TEST_F(GameTest, DoubleDownLosesDoubleToo) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(5), V(6)}}, kDealer17, {V(2)})));

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_TRUE(game->DoubleDown());

    EXPECT_EQ(Balance(0), kStartingBalance - 200);
}

TEST_F(GameTest, DoubleDownNotAllowedAfterHit) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(2), V(3)}}, kDealer17, {V(2)})));

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_TRUE(game->Hit());

    EXPECT_FALSE(game->CanDoubleDown());
    EXPECT_FALSE(game->DoubleDown());
    EXPECT_EQ(game->GetState(), EGameState::PlayersTurn);
}

TEST_F(GameTest, DoubleDownNeedsEnoughMoney) {
    const auto losingRound = ArrangeDeck({{V(10), V(8)}}, {V(10), V(9)});
    const auto secondRound = ArrangeDeck({{V(5), V(6)}}, kDealer17, {V(10)});
    ASSERT_NO_FATAL_FAILURE(Prepare(Concat(losingRound, secondRound)));

    ASSERT_NO_FATAL_FAILURE(BeginRound(kMaxBet));
    ASSERT_NO_FATAL_FAILURE(StandAll());
    ASSERT_NO_FATAL_FAILURE(BeginRound(kMaxBet));

    ASSERT_EQ(game->GetState(), EGameState::PlayersTurn);
    EXPECT_EQ(Balance(0), 0);
    EXPECT_FALSE(game->CanDoubleDown());
    EXPECT_FALSE(game->DoubleDown());
}

TEST_F(GameTest, TwoPlayersPlayInTurnAndAreSettledSeparately) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(10)}, {V(10), V(6)}}, kDealer17), 2));
    EXPECT_CALL(*listener, OnTurnChanged(0)).Times(1);
    EXPECT_CALL(*listener, OnTurnChanged(1)).Times(1);
    EXPECT_CALL(*listener, OnDealerCardRevealed(V(7))).Times(1);
    EXPECT_CALL(*listener, OnRoundEnded(0, ERoundResult::Win)).Times(1);
    EXPECT_CALL(*listener, OnRoundEnded(1, ERoundResult::Lose)).Times(1);

    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_EQ(game->GetCurrentPlayerIndex(), 0);

    ASSERT_TRUE(game->Stand());
    EXPECT_EQ(game->GetCurrentPlayerIndex(), 1);
    EXPECT_EQ(game->GetState(), EGameState::PlayersTurn);

    ASSERT_TRUE(game->Stand());
    EXPECT_EQ(game->GetState(), EGameState::RoundOver);
    EXPECT_EQ(game->GetCurrentPlayerIndex(), -1);
    EXPECT_EQ(Balance(0), kStartingBalance + 100);
    EXPECT_EQ(Balance(1), kStartingBalance - 100);
}

TEST_F(GameTest, NewRoundClearsTheTable) {
    const auto round = ArrangeDeck({{V(10), V(9)}}, {V(10), V(9)});
    ASSERT_NO_FATAL_FAILURE(Prepare(Concat(round, round)));
    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());
    ASSERT_EQ(game->GetState(), EGameState::RoundOver);

    ASSERT_TRUE(game->StartRound());

    EXPECT_EQ(game->GetState(), EGameState::Betting);
    EXPECT_TRUE(game->GetPlayer(0)->GetHand().empty());
    EXPECT_EQ(game->GetPlayer(0)->GetBet(), 0);
    EXPECT_TRUE(game->GetDealer()->GetHand().empty());
    EXPECT_EQ(Balance(0), kStartingBalance);
    EXPECT_TRUE(game->CanPlaceBet(0));
}

TEST_F(GameTest, ActionsRefusedOnceTheRoundIsOver) {
    ASSERT_NO_FATAL_FAILURE(Prepare(ArrangeDeck({{V(10), V(9)}}, {V(10), V(9)})));
    ASSERT_NO_FATAL_FAILURE(BeginRound(100));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    EXPECT_FALSE(game->Hit());
    EXPECT_FALSE(game->Stand());
    EXPECT_FALSE(game->DoubleDown());
    EXPECT_FALSE(game->PlaceBet(0, 100));
}

TEST_F(GameTest, BrokePlayerCannotStartAnotherRound) {
    const auto round = ArrangeDeck({{V(10), V(8)}}, {V(10), V(9)});
    ASSERT_NO_FATAL_FAILURE(Prepare(Concat(round, round)));

    ASSERT_NO_FATAL_FAILURE(BeginRound(kMaxBet));
    ASSERT_NO_FATAL_FAILURE(StandAll());
    ASSERT_NO_FATAL_FAILURE(BeginRound(kMaxBet));
    ASSERT_NO_FATAL_FAILURE(StandAll());

    ASSERT_EQ(game->GetState(), EGameState::RoundOver);
    EXPECT_EQ(Balance(0), 0);
    EXPECT_FALSE(game->CanPlaceBet(0));
    EXPECT_FALSE(game->StartRound());
    EXPECT_EQ(game->GetState(), EGameState::RoundOver);
}

TEST_F(GameTest, DeckIsShuffledOncePerRound) {
    auto mockDeck = std::make_shared<NiceMock<MockDeck>>();
    ON_CALL(*mockDeck, Draw()).WillByDefault(Return(V(10)));
    EXPECT_CALL(*mockDeck, Shuffle()).Times(2);
    const auto mockedGame = IGame::Produce(mockDeck);
    ASSERT_NE(mockedGame, nullptr);
    mockedGame->AddPlayer("Ana");

    for (int round = 0; round < 2; ++round) {
        ASSERT_TRUE(mockedGame->StartRound());
        ASSERT_TRUE(mockedGame->PlaceBet(0, 100));
        while (mockedGame->GetState() == EGameState::PlayersTurn) {
            ASSERT_TRUE(mockedGame->Stand());
        }
    }
}

TEST_F(GameTest, RemovedListenerIsNotNotified) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 0));
    game->RemoveListener(listener.get());
    EXPECT_CALL(*listener, OnPlayerAdded(_)).Times(0);

    game->AddPlayer("Ana");
}

TEST_F(GameTest, ExpiredListenerIsSkipped) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 0));
    listener.reset();

    EXPECT_TRUE(game->AddPlayer("Ana"));
}

TEST_F(GameTest, ListenerMayRemoveItselfWhileBeingNotified) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 0));
    EXPECT_CALL(*listener, OnPlayerAdded(_)).WillOnce([this](int) {
        game->RemoveListener(listener.get());
    });

    game->AddPlayer("Ana");
    game->AddPlayer("Bob");
}

TEST_F(GameTest, AllListenersAreNotified) {
    ASSERT_NO_FATAL_FAILURE(Prepare({}, 0));
    auto second = std::make_shared<NiceMock<MockGameListener>>();
    game->AddListener(second);
    EXPECT_CALL(*listener, OnPlayerAdded(0)).Times(1);
    EXPECT_CALL(*second, OnPlayerAdded(0)).Times(1);

    game->AddPlayer("Ana");
}
