#include <map>
#include <set>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#if __has_include(<Backend/game/deck.h>)
#include <Backend/game/deck.h>
#elif __has_include(<Backend/game/Deck.h>)
#include <Backend/game/Deck.h>
#elif __has_include(<Backend/deck.h>)
#include <Backend/deck.h>
#else
#error "Nu gasesc header-ul clasei Deck"
#endif

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;

namespace {
    using Key = std::pair<int, int>;

    Key KeyOf(const Card& card) {
        return {static_cast<int>(card.rank), static_cast<int>(card.suit)};
    }

    std::vector<Card> DrawMany(IDeck& deck, int count) {
        std::vector<Card> cards;
        for (int i = 0; i < count; ++i) {
            cards.push_back(deck.Draw());
        }
        return cards;
    }

    bool IsValid(const Card& card) {
        const int rank = static_cast<int>(card.rank);
        const int suit = static_cast<int>(card.suit);
        return rank >= 2 && rank <= 14 && suit >= 0 && suit <= 3;
    }
}  // namespace

TEST(DeckTest, ShuffledDeckHasFiftyTwoDistinctCards) {
    Deck deck;
    deck.Shuffle();

    std::set<Key> seen;
    for (const Card& card : DrawMany(deck, 52)) {
        EXPECT_TRUE(IsValid(card));
        seen.insert(KeyOf(card));
    }

    EXPECT_EQ(seen.size(), 52u);
}

TEST(DeckTest, RebuildsItselfWhenEmpty) {
    Deck deck;
    deck.Shuffle();

    std::map<Key, int> counts;
    for (const Card& card : DrawMany(deck, 104)) {
        EXPECT_TRUE(IsValid(card));
        ++counts[KeyOf(card)];
    }

    EXPECT_EQ(counts.size(), 52u);
    for (const auto& [key, count] : counts) {
        EXPECT_EQ(count, 2);
    }
}

TEST(DeckTest, ShuffleRestoresFullDeckMidway) {
    Deck deck;
    deck.Shuffle();
    DrawMany(deck, 20);

    deck.Shuffle();

    std::set<Key> seen;
    for (const Card& card : DrawMany(deck, 52)) {
        seen.insert(KeyOf(card));
    }
    EXPECT_EQ(seen.size(), 52u);
}

TEST(DeckTest, ShuffleChangesTheOrder) {
    Deck deck;

    deck.Shuffle();
    const std::vector<Card> first = DrawMany(deck, 52);
    deck.Shuffle();
    const std::vector<Card> second = DrawMany(deck, 52);

    EXPECT_NE(first, second);
}

TEST(DeckTest, WorksThroughInterface) {
    std::unique_ptr<IDeck> deck = std::make_unique<Deck>();
    deck->Shuffle();

    EXPECT_TRUE(IsValid(deck->Draw()));
}
