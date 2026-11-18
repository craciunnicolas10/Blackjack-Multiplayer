#include <cstdint>
#include <optional>
#include <string>

#include <gtest/gtest.h>

#include <SFML/Network/Packet.hpp>

#include <Backend/net/protocol.h>

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;

TEST(ProtocolMessageTest, EveryMessageTypeRoundTrips) {
    for (std::uint8_t raw = 0; raw <= static_cast<std::uint8_t>(kLastMessageType); ++raw) {
        const auto type = static_cast<EMessageType>(raw);
        sf::Packet packet = MakeMessage(type);

        const auto read = ReadMessageType(packet);

        ASSERT_TRUE(read.has_value());
        EXPECT_EQ(*read, type);
        EXPECT_TRUE(packet.endOfPacket());
    }
}

TEST(ProtocolMessageTest, EmptyPacketHasNoType) {
    sf::Packet packet;

    EXPECT_FALSE(ReadMessageType(packet).has_value());
}

TEST(ProtocolMessageTest, UnknownTypeIsRejected) {
    sf::Packet packet;
    packet << static_cast<std::uint8_t>(static_cast<std::uint8_t>(kLastMessageType) + 1);

    EXPECT_FALSE(ReadMessageType(packet).has_value());
}

TEST(ProtocolMessageTest, ExtraPayloadFollowsTheType) {
    sf::Packet packet = MakeMessage(EMessageType::Rejected);
    packet << static_cast<std::uint8_t>(EJoinRejection::TableFull);

    ASSERT_EQ(ReadMessageType(packet), EMessageType::Rejected);
    EJoinRejection reason = EJoinRejection::DifferentVersion;
    ASSERT_TRUE(ReadEnum(packet, reason, kLastJoinRejection));
    EXPECT_EQ(reason, EJoinRejection::TableFull);
}

TEST(ProtocolEnumTest, AcceptsEveryValueUpToLast) {
    for (std::uint8_t raw = 0; raw <= static_cast<std::uint8_t>(kLastGameState); ++raw) {
        sf::Packet packet;
        packet << raw;
        EGameState state = EGameState::WaitingForPlayers;

        ASSERT_TRUE(ReadEnum(packet, state, kLastGameState));
        EXPECT_EQ(static_cast<std::uint8_t>(state), raw);
    }
}

TEST(ProtocolEnumTest, RejectsValuesAfterLast) {
    sf::Packet packet;
    packet << static_cast<std::uint8_t>(static_cast<std::uint8_t>(kLastRoundResult) + 1);
    ERoundResult result = ERoundResult::Win;

    EXPECT_FALSE(ReadEnum(packet, result, kLastRoundResult));
}

TEST(ProtocolEnumTest, RejectsEmptyPacket) {
    sf::Packet packet;
    EEventType type = EEventType::StateChanged;

    EXPECT_FALSE(ReadEnum(packet, type, kLastEventType));
}

TEST(ProtocolCardTest, EveryCardRoundTrips) {
    for (int suit = 0; suit <= static_cast<int>(ESuit::Spades); ++suit) {
        for (int rank = 2; rank <= static_cast<int>(ERank::Ace); ++rank) {
            const Card card{static_cast<ERank>(rank), static_cast<ESuit>(suit)};
            sf::Packet packet;
            WriteCard(packet, card);
            Card read{ERank::Two, ESuit::Clubs};

            ASSERT_TRUE(ReadCard(packet, read));
            EXPECT_EQ(read, card);
        }
    }
}

TEST(ProtocolCardTest, EmptyPacketIsRejected) {
    sf::Packet packet;
    Card card{ERank::Two, ESuit::Clubs};

    EXPECT_FALSE(ReadCard(packet, card));
}

TEST(ProtocolCardTest, TruncatedCardIsRejected) {
    sf::Packet full;
    WriteCard(full, V(10));
    ASSERT_GT(full.getDataSize(), 1u);
    sf::Packet cut = Truncate(full, full.getDataSize() - 1);
    Card card{ERank::Two, ESuit::Clubs};

    EXPECT_FALSE(ReadCard(cut, card));
}

TEST(ProtocolCardTest, InvalidValuesAreRejected) {
    const std::pair<int, int> invalid[] = {{200, 200}, {0, 0}, {1, 1}, {15, 0}};
    for (const auto& [first, second] : invalid) {
        sf::Packet packet;
        packet << static_cast<std::uint8_t>(first) << static_cast<std::uint8_t>(second);
        Card card{ERank::Two, ESuit::Clubs};

        EXPECT_FALSE(ReadCard(packet, card)) << first << "," << second;
    }
}

TEST(ProtocolNameTest, RejectsEmptyName) {
    EXPECT_FALSE(IsValidName(""));
}

TEST(ProtocolNameTest, AcceptsOrdinaryNames) {
    EXPECT_TRUE(IsValidName("Ana"));
    EXPECT_TRUE(IsValidName("Player 1"));
}

TEST(ProtocolNameTest, LimitIsMeasuredInBytes) {
    EXPECT_TRUE(IsValidName(std::string(kMaxNameBytes, 'a')));
    EXPECT_FALSE(IsValidName(std::string(kMaxNameBytes + 1, 'a')));

    std::string diacritics;
    for (int i = 0; i < 24; ++i) {
        diacritics += "\xC4\x83";
    }
    EXPECT_TRUE(IsValidName(diacritics));
    diacritics += "\xC4\x83";
    EXPECT_FALSE(IsValidName(diacritics));
}

TEST(ProtocolAddressTest, DefaultsToTheDefaultPort) {
    const auto parsed = SplitAddress("192.168.1.5");

    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->first, "192.168.1.5");
    EXPECT_EQ(parsed->second, 53000);
}

TEST(ProtocolAddressTest, ReadsExplicitPort) {
    const auto parsed = SplitAddress("192.168.1.5:53001");

    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->first, "192.168.1.5");
    EXPECT_EQ(parsed->second, 53001);
}

TEST(ProtocolAddressTest, AcceptsHighestPort) {
    const auto parsed = SplitAddress("10.0.0.1:65535");

    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->second, 65535);
}

TEST(ProtocolAddressTest, RejectsBadPorts) {
    EXPECT_FALSE(SplitAddress("10.0.0.1:abc").has_value());
    EXPECT_FALSE(SplitAddress("10.0.0.1:12x").has_value());
    EXPECT_FALSE(SplitAddress("10.0.0.1:70000").has_value());
    EXPECT_FALSE(SplitAddress("10.0.0.1:-1").has_value());
}
