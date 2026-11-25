#include <Backend/net/network_host.h>

#include <algorithm>
#include <cstdint>
#include <utility>

#include <SFML/Network/IpAddress.hpp>

namespace Backend {
    namespace {
        constexpr int kHostSeat = 0;
        // Cate calculatoare pot astepta in acelasi timp sa-si trimita numele.
        constexpr std::size_t kMaxConnections = 8;
    }  // namespace

    NetworkHost::NetworkHost(std::string hostName) : m_hostName(std::move(hostName)) {}

    bool NetworkHost::Listen() {
        for (unsigned short i = 0; i < kPortCount; ++i) {
            const auto port = static_cast<unsigned short>(kDefaultPort + i);
            if (m_listener.ListenReusable(port)) {
                m_listener.setBlocking(false);
                m_port = port;
                return true;
            }
        }
        return false;
    }

    void NetworkHost::AddListener(std::weak_ptr<INetworkListener> listener) {
        m_listeners.Add(std::move(listener));
    }

    void NetworkHost::RemoveListener(INetworkListener* listener) { m_listeners.Remove(listener); }

    bool NetworkHost::IsHost() const { return true; }

    int NetworkHost::GetLocalSeat() const { return kHostSeat; }

    std::shared_ptr<IGame> NetworkHost::GetGame() const { return m_game; }

    std::vector<std::string> NetworkHost::GetPlayerNames() const {
        std::vector<std::string> names{m_hostName};
        for (const Connection& connection : m_connections) {
            if (connection.joined && !connection.closed) {
                names.push_back(connection.name);
            }
        }
        return names;
    }

    int NetworkHost::JoinedCount() const {
        return static_cast<int>(GetPlayerNames().size()) - 1;
    }

    std::string NetworkHost::GetAddress() const {
        const std::optional<sf::IpAddress> address = sf::IpAddress::getLocalAddress();
        if (!address) {
            return {};
        }
        // Portul se scrie doar cand nu e cel obisnuit; atunci trebuie scris si la conectare.
        const std::string port = m_port == kDefaultPort ? "" : ":" + std::to_string(m_port);
        return address->toString() + port;
    }

    bool NetworkHost::CanStartGame() const { return !m_started && JoinedCount() > 0; }

    // --- Lobby ---

    void NetworkHost::Update() {
        if (m_stopped) {
            return;
        }
        if (!m_started) {
            AcceptConnections();
        }
        ReceiveMessages();
        if (m_lostPlayer) {
            StopGame(*m_lostPlayer);
        }
    }

    void NetworkHost::AcceptConnections() {
        while (m_connections.size() < kMaxConnections) {
            auto socket = std::make_unique<sf::TcpSocket>();
            if (m_listener.accept(*socket) != sf::Socket::Status::Done) {
                return;
            }
            socket->setBlocking(false);
            Connection connection;
            connection.socket = std::move(socket);
            m_connections.push_back(std::move(connection));
        }
    }

    void NetworkHost::ReceiveMessages() {
        bool listChanged = false;
        for (Connection& connection : m_connections) {
            sf::Packet packet;
            sf::Socket::Status status = connection.socket->receive(packet);
            while (status == sf::Socket::Status::Done && !connection.closed && !m_stopped) {
                const std::optional<EMessageType> type = ReadMessageType(packet);
                if (type == EMessageType::Join && !connection.joined && !m_started) {
                    listChanged = HandleJoin(connection, packet) || listChanged;
                } else if (type && connection.joined && m_started) {
                    HandleAction(connection, *type, packet);
                }
                packet.clear();
                status = connection.socket->receive(packet);
            }
            if (status == sf::Socket::Status::Disconnected ||
                status == sf::Socket::Status::Error) {
                connection.closed = true;
                if (connection.joined && m_started && !m_lostPlayer) {
                    m_lostPlayer = connection.name;
                }
                listChanged = listChanged || connection.joined;
            }
        }
        if (m_started) {
            return;  // in joc locurile nu se mai schimba; cine iese opreste jocul
        }
        std::erase_if(m_connections,
                      [](const Connection& connection) { return connection.closed; });
        if (listChanged) {
            BroadcastPlayerList();
        }
    }

    bool NetworkHost::HandleJoin(Connection& connection, sf::Packet& packet) {
        std::uint32_t version = 0;
        std::string name;
        if (!(packet >> version >> name)) {
            connection.closed = true;
            return false;
        }
        if (version != kProtocolVersion) {
            Reject(connection, EJoinRejection::DifferentVersion);
            return false;
        }
        if (JoinedCount() + 1 >= kMaxPlayers) {
            Reject(connection, EJoinRejection::TableFull);
            return false;
        }
        if (!IsValidName(name)) {
            Reject(connection, EJoinRejection::InvalidName);
            return false;
        }
        // Aceeasi regula ca IGame::AddPlayer: numele trebuie sa fie diferite.
        const std::vector<std::string> names = GetPlayerNames();
        if (std::ranges::find(names, name) != names.end()) {
            Reject(connection, EJoinRejection::NameTaken);
            return false;
        }
        connection.name = std::move(name);
        connection.joined = true;
        return true;
    }

    void NetworkHost::Reject(Connection& connection, EJoinRejection reason) {
        sf::Packet packet = MakeMessage(EMessageType::Rejected);
        packet << static_cast<std::uint8_t>(reason);
        SendAll(*connection.socket, packet);
        connection.socket->disconnect();
        connection.closed = true;
    }

    void NetworkHost::BroadcastPlayerList() {
        const std::vector<std::string> names = GetPlayerNames();
        sf::Packet packet = MakeMessage(EMessageType::PlayerList);
        packet << static_cast<std::uint8_t>(names.size());
        for (const std::string& name : names) {
            packet << name;
        }
        for (Connection& connection : m_connections) {
            if (connection.joined && !SendAll(*connection.socket, packet)) {
                connection.closed = true;
            }
        }
        m_listeners.Notify([&](INetworkListener& listener) { listener.OnPlayersChanged(names); });
    }

    // --- Jocul ---

    bool NetworkHost::StartGame() {
        if (!CanStartGame()) {
            return false;
        }
        m_started = true;
        m_listener.close();
        std::erase_if(m_connections, [](const Connection& connection) {
            return !connection.joined || connection.closed;
        });

        const std::vector<std::string> names = GetPlayerNames();
        for (std::size_t i = 0; i < m_connections.size(); ++i) {
            Connection& connection = m_connections[i];
            connection.seat = static_cast<int>(i) + 1;  // gazda e la locul 0
            sf::Packet packet = MakeMessage(EMessageType::GameStarting);
            packet << static_cast<std::uint8_t>(connection.seat)
                   << static_cast<std::uint8_t>(names.size());
            for (const std::string& name : names) {
                packet << name;
            }
            if (!SendAll(*connection.socket, packet) && !m_lostPlayer) {
                m_lostPlayer = connection.name;
            }
        }

        m_realGame = IGame::Produce();
        m_encoder = std::make_shared<EventEncoder>(
            *m_realGame, [this](sf::Packet& packet) { Broadcast(packet); });
        m_realGame->AddListener(m_encoder);
        for (const std::string& name : names) {
            m_realGame->AddPlayer(name);
        }
        // Prin weak_ptr: jocul poate fi tinut in viata de Frontend si dupa ce gazda s-a inchis.
        m_game = std::make_shared<HostedGame>(m_realGame, [weak = weak_from_this()] {
            if (const auto self = weak.lock()) {
                self->OnStartRoundRefused();
            }
        });

        m_listeners.Notify([](INetworkListener& listener) { listener.OnGameStarted(); });
        return true;
    }

    void NetworkHost::HandleAction(const Connection& connection, EMessageType type,
                                   sf::Packet& packet) {
        const bool myTurn = m_game->GetState() == EGameState::PlayersTurn &&
                            m_game->GetCurrentPlayerIndex() == connection.seat;
        switch (type) {
            case EMessageType::PlaceBet: {
                std::int32_t amount = 0;
                if (packet >> amount) {
                    m_game->PlaceBet(connection.seat, amount);
                }
                break;
            }
            case EMessageType::Hit:
                if (myTurn) {
                    m_game->Hit();
                }
                break;
            case EMessageType::Stand:
                if (myTurn) {
                    m_game->Stand();
                }
                break;
            case EMessageType::DoubleDown:
                if (myTurn) {
                    m_game->DoubleDown();
                }
                break;
            case EMessageType::NextRound:
                if (m_game->GetState() == EGameState::RoundOver) {
                    m_game->StartRound();
                }
                break;
            default:
                break;  // mesajele gazdei nu au ce cauta aici
        }
    }

    void NetworkHost::Broadcast(sf::Packet& packet) {
        for (const Connection& connection : m_connections) {
            if (!SendAll(*connection.socket, packet) && !m_lostPlayer) {
                m_lostPlayer = connection.name;
            }
        }
    }

    void NetworkHost::OnStartRoundRefused() {
        sf::Packet packet = MakeMessage(EMessageType::GameOver);
        Broadcast(packet);
        m_listeners.Notify([](INetworkListener& listener) { listener.OnGameOver(); });
    }

    void NetworkHost::StopGame(const std::string& leaverName) {
        m_stopped = true;
        sf::Packet packet = MakeMessage(EMessageType::PlayerLeft);
        packet << leaverName;
        for (const Connection& connection : m_connections) {
            if (connection.name != leaverName) {
                SendAll(*connection.socket, packet);
            }
            connection.socket->disconnect();
        }
        m_listeners.Notify(
            [&](INetworkListener& listener) { listener.OnPlayerLeft(leaverName); });
    }
}  // namespace Backend
