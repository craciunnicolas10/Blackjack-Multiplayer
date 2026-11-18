#include <Backend/INetworkClient.h>

#include <optional>
#include <utility>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/System/Time.hpp>

#include <Backend/net/network_client.h>
#include <Backend/net/protocol.h>

namespace Backend {
    namespace {
        // Cat asteptam gazda sa raspunda la conectare.
        constexpr float kConnectTimeoutSeconds = 3.f;
    }  // namespace

    std::shared_ptr<INetworkClient> INetworkClient::Produce(const std::string& address,
                                                            const std::string& name,
                                                            EConnectResult& result) {
        if (!IsValidName(name)) {
            result = EConnectResult::InvalidName;
            return nullptr;
        }
        const auto hostAndPort = SplitAddress(address);
        const std::optional<sf::IpAddress> ip =
            hostAndPort && !hostAndPort->first.empty() ? sf::IpAddress::resolve(hostAndPort->first)
                                                       : std::nullopt;
        if (!ip) {
            result = EConnectResult::InvalidAddress;
            return nullptr;
        }

        auto socket = std::make_unique<sf::TcpSocket>();
        if (socket->connect(*ip, hostAndPort->second, sf::seconds(kConnectTimeoutSeconds)) !=
            sf::Socket::Status::Done) {
            result = EConnectResult::Unreachable;
            return nullptr;
        }
        socket->setBlocking(false);

        sf::Packet join = MakeMessage(EMessageType::Join);
        join << kProtocolVersion << name;
        if (!SendAll(*socket, join)) {
            result = EConnectResult::Unreachable;
            return nullptr;
        }
        result = EConnectResult::Connected;
        return std::make_shared<NetworkClient>(std::move(socket));
    }
}  // namespace Backend
