#include <net/protocol.h>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>

namespace Frontend::Net {
    namespace {
        constexpr sf::Time kSendTimeout = sf::seconds(3.f);
    }  // namespace

    sf::Packet makeMessage(MessageType type) {
        sf::Packet packet;
        packet << static_cast<std::uint8_t>(type);
        return packet;
    }

    std::optional<MessageType> readMessageType(sf::Packet& packet) {
        MessageType type{};
        if (!readEnum(packet, type, kLastMessageType)) {
            return std::nullopt;
        }
        return type;
    }

    bool sendAll(sf::TcpSocket& socket, sf::Packet& packet) {
        const sf::Clock clock;
        while (true) {
            const sf::Socket::Status status = socket.send(packet);
            if (status == sf::Socket::Status::Done) {
                return true;
            }
            const bool tryAgain = status == sf::Socket::Status::Partial ||
                                  status == sf::Socket::Status::NotReady;
            if (!tryAgain || clock.getElapsedTime() > kSendTimeout) {
                return false;
            }
            sf::sleep(sf::milliseconds(1));
        }
    }

    void writeCard(sf::Packet& packet, Backend::Card card) {
        packet << static_cast<std::uint8_t>(card.rank) << static_cast<std::uint8_t>(card.suit);
    }

    bool readCard(sf::Packet& packet, Backend::Card& card) {
        std::uint8_t rank = 0;
        std::uint8_t suit = 0;
        if (!(packet >> rank >> suit)) {
            return false;
        }
        const bool validRank = rank >= static_cast<std::uint8_t>(Backend::ERank::Two) &&
                               rank <= static_cast<std::uint8_t>(Backend::ERank::Ace);
        const bool validSuit = suit <= static_cast<std::uint8_t>(Backend::ESuit::Spades);
        if (!validRank || !validSuit) {
            return false;
        }
        card = {static_cast<Backend::ERank>(rank), static_cast<Backend::ESuit>(suit)};
        return true;
    }
}  // namespace Frontend::Net
