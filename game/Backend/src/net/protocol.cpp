#include <Backend/net/protocol.h>

#include <charconv>
#include <system_error>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>

#include <Backend/INetworkHost.h>

namespace Backend {
    namespace {
        constexpr float kSendTimeoutSeconds = 3.f;
    }  // namespace

    sf::Packet MakeMessage(EMessageType type) {
        sf::Packet packet;
        packet << static_cast<std::uint8_t>(type);
        return packet;
    }

    std::optional<EMessageType> ReadMessageType(sf::Packet& packet) {
        EMessageType type{};
        if (!ReadEnum(packet, type, kLastMessageType)) {
            return std::nullopt;
        }
        return type;
    }

    bool SendAll(sf::TcpSocket& socket, sf::Packet& packet) {
        const sf::Clock clock;
        while (true) {
            const sf::Socket::Status status = socket.send(packet);
            if (status == sf::Socket::Status::Done) {
                return true;
            }
            const bool tryAgain = status == sf::Socket::Status::Partial ||
                                  status == sf::Socket::Status::NotReady;
            if (!tryAgain || clock.getElapsedTime() > sf::seconds(kSendTimeoutSeconds)) {
                return false;
            }
            sf::sleep(sf::milliseconds(1));
        }
    }

    void WriteCard(sf::Packet& packet, Card card) {
        packet << static_cast<std::uint8_t>(card.rank) << static_cast<std::uint8_t>(card.suit);
    }

    bool ReadCard(sf::Packet& packet, Card& card) {
        std::uint8_t rank = 0;
        std::uint8_t suit = 0;
        if (!(packet >> rank >> suit)) {
            return false;
        }
        const bool validRank = rank >= static_cast<std::uint8_t>(ERank::Two) &&
                               rank <= static_cast<std::uint8_t>(ERank::Ace);
        const bool validSuit = suit <= static_cast<std::uint8_t>(ESuit::Spades);
        if (!validRank || !validSuit) {
            return false;
        }
        card = {static_cast<ERank>(rank), static_cast<ESuit>(suit)};
        return true;
    }

    bool IsValidName(const std::string& name) {
        return !name.empty() && name.size() <= kMaxNameBytes;
    }

    std::optional<std::pair<std::string, unsigned short>> SplitAddress(const std::string& text) {
        const std::size_t colon = text.rfind(':');
        if (colon == std::string::npos) {
            return std::pair{text, INetworkHost::kDefaultPort};
        }
        unsigned short port = 0;
        const char* begin = text.data() + colon + 1;
        const char* end = text.data() + text.size();
        const auto [last, error] = std::from_chars(begin, end, port);
        if (error != std::errc{} || last != end || port == 0) {
            return std::nullopt;
        }
        return std::pair{text.substr(0, colon), port};
    }
}  // namespace Backend
