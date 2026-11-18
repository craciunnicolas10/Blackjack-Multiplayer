#include <Backend/net/network_client.h>

#include <cstdint>
#include <optional>
#include <utility>

#include <Backend/net/game_events.h>
#include <Backend/net/protocol.h>

namespace Backend {
    namespace {
        bool ReadNames(sf::Packet& packet, std::vector<std::string>& names) {
            std::uint8_t count = 0;
            if (!(packet >> count) || count > kMaxPlayers) {
                return false;
            }
            names.assign(count, {});
            for (std::string& name : names) {
                if (!(packet >> name) || !IsValidName(name)) {
                    return false;
                }
            }
            return true;
        }
    }  // namespace

    NetworkClient::NetworkClient(std::unique_ptr<sf::TcpSocket> socket)
        : m_socket(std::move(socket)) {}

    void NetworkClient::AddListener(std::weak_ptr<INetworkListener> listener) {
        m_listeners.Add(std::move(listener));
    }

    void NetworkClient::RemoveListener(INetworkListener* listener) {
        m_listeners.Remove(listener);
    }

    bool NetworkClient::IsHost() const { return false; }

    std::vector<std::string> NetworkClient::GetPlayerNames() const { return m_names; }

    int NetworkClient::GetLocalSeat() const { return m_seat; }

    std::shared_ptr<IGame> NetworkClient::GetGame() const { return m_game; }

    void NetworkClient::Update() {
        if (m_closed) {
            return;
        }
        sf::Packet packet;
        sf::Socket::Status status = m_socket->receive(packet);
        while (status == sf::Socket::Status::Done) {
            if (!HandleMessage(packet)) {
                return;
            }
            packet.clear();
            status = m_socket->receive(packet);
        }
        if (m_sendFailed || status == sf::Socket::Status::Disconnected ||
            status == sf::Socket::Status::Error) {
            Close();
            m_listeners.Notify([](INetworkListener& listener) { listener.OnHostLeft(); });
        }
    }

    bool NetworkClient::HandleMessage(sf::Packet& packet) {
        const std::optional<EMessageType> type = ReadMessageType(packet);
        if (!type) {
            return true;
        }
        switch (*type) {
            case EMessageType::PlayerList:
                if (!m_game && ReadNames(packet, m_names)) {
                    m_listeners.Notify(
                        [&](INetworkListener& listener) { listener.OnPlayersChanged(m_names); });
                }
                return true;
            case EMessageType::Rejected: {
                EJoinRejection reason{};
                if (!ReadEnum(packet, reason, kLastJoinRejection)) {
                    reason = EJoinRejection::InvalidName;
                }
                Close();
                m_listeners.Notify(
                    [reason](INetworkListener& listener) { listener.OnJoinRejected(reason); });
                return false;
            }
            case EMessageType::GameStarting:
                return StartGame(packet);
            case EMessageType::GameEvent:
                if (m_game) {
                    if (std::optional<DecodedEvent> event = DecodeEvent(packet)) {
                        m_game->ApplyEvent(std::move(*event));
                    }
                }
                return !m_closed;
            case EMessageType::GameOver:
                m_listeners.Notify([](INetworkListener& listener) { listener.OnGameOver(); });
                return true;
            case EMessageType::PlayerLeft: {
                std::string name;
                packet >> name;
                Close();
                m_listeners.Notify(
                    [&](INetworkListener& listener) { listener.OnPlayerLeft(name); });
                return false;
            }
            default:
                return true;  // mesajele jucatorilor nu au ce cauta aici
        }
    }

    bool NetworkClient::StartGame(sf::Packet& packet) {
        std::uint8_t seat = 0;
        std::vector<std::string> names;
        if (m_game || !(packet >> seat) || !ReadNames(packet, names) || seat >= names.size()) {
            return true;  // mesaj stricat sau venit de doua ori: il ignoram
        }
        m_seat = seat;
        m_names = std::move(names);
        // Prin weak_ptr: Frontend-ul poate tine jocul in viata si dupa ce s-a inchis legatura.
        m_game = std::make_shared<RemoteGame>([weak = weak_from_this()](sf::Packet& message) {
            if (const auto self = weak.lock()) {
                self->Send(message);
            }
        });
        m_listeners.Notify([](INetworkListener& listener) { listener.OnGameStarted(); });
        // Anunturile jocului care au venit deja asteapta in socket pana la urmatorul Update(),
        // dupa ce Frontend-ul si-a pus ascultatorii pe GetGame().
        return false;
    }

    void NetworkClient::Send(sf::Packet& packet) {
        if (!m_closed && !SendAll(*m_socket, packet)) {
            m_sendFailed = true;
        }
    }

    void NetworkClient::Close() {
        m_closed = true;
        m_socket->disconnect();
    }
}  // namespace Backend
