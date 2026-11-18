#include <net/host_session.h>

#include <cstdint>
#include <utility>

#include <net/protocol.h>
#include <screens/table_screen.h>
#include <ui/utf8.h>

namespace Frontend::Net {
    namespace {
        constexpr std::size_t kHostSeat = 0;
    }  // namespace

    HostSession::HostSession(std::vector<sf::String> playerNames,
                             std::vector<RemotePlayer> remotePlayers)
        : m_playerNames(std::move(playerNames)), m_remotePlayers(std::move(remotePlayers)) {}

    void HostSession::start(TableScreen& table) {
        m_table = &table;
        m_game = Backend::IGame::Produce();

        std::vector<bool> localSeats(m_playerNames.size(), false);
        localSeats[kHostSeat] = true;
        m_controller = GameController::create(table, m_game, std::move(localSeats));
        m_controller->setOnGameOver([this] {
            sf::Packet packet = makeMessage(MessageType::GameOver);
            broadcast(packet);
        });

        m_encoder = std::make_shared<EventEncoder>(
            *m_game, [this](sf::Packet& packet) { broadcast(packet); });
        m_game->AddListener(m_encoder);

        for (const sf::String& name : m_playerNames) {
            m_game->AddPlayer(toUtf8String(name));
        }
        m_game->StartRound();
    }

    void HostSession::update() {
        if (m_stopped) {
            return;
        }
        for (const RemotePlayer& player : m_remotePlayers) {
            if (m_lostPlayer) {
                break;
            }
            sf::Packet packet;
            sf::Socket::Status status = player.socket->receive(packet);
            while (status == sf::Socket::Status::Done) {
                handleMessage(player, packet);
                packet.clear();
                status = player.socket->receive(packet);
            }
            if (status == sf::Socket::Status::Disconnected ||
                status == sf::Socket::Status::Error) {
                m_lostPlayer = player.name;
            }
        }
        if (m_lostPlayer) {
            stop(*m_lostPlayer);
        }
    }

    void HostSession::broadcast(sf::Packet& packet) {
        for (const RemotePlayer& player : m_remotePlayers) {
            if (!sendAll(*player.socket, packet) && !m_lostPlayer) {
                m_lostPlayer = player.name;
            }
        }
    }

    void HostSession::handleMessage(const RemotePlayer& player, sf::Packet& packet) {
        const std::optional<MessageType> type = readMessageType(packet);
        if (!type) {
            return;
        }
        const int seat = static_cast<int>(player.seat);
        const bool myTurn = m_game->GetState() == Backend::EGameState::PlayersTurn &&
                            m_game->GetCurrentPlayerIndex() == seat;
        switch (*type) {
            case MessageType::PlaceBet: {
                std::int32_t amount = 0;
                if (packet >> amount && m_controller->getBettingSeat() == player.seat) {
                    m_game->PlaceBet(seat, amount);
                }
                break;
            }
            case MessageType::Hit:
                if (myTurn) {
                    m_game->Hit();
                }
                break;
            case MessageType::Stand:
                if (myTurn) {
                    m_game->Stand();
                }
                break;
            case MessageType::DoubleDown:
                if (myTurn) {
                    m_game->DoubleDown();
                }
                break;
            case MessageType::NextRound:
                if (m_game->GetState() == Backend::EGameState::RoundOver) {
                    m_controller->requestNextRound();
                }
                break;
            default:
                break;  // mesajele gazdei nu au ce căuta aici
        }
    }

    void HostSession::stop(const sf::String& leaverName) {
        m_stopped = true;
        sf::Packet packet = makeMessage(MessageType::PlayerLeft);
        packet << leaverName;
        for (const RemotePlayer& player : m_remotePlayers) {
            if (player.name != leaverName) {
                sendAll(*player.socket, packet);
            }
            player.socket->disconnect();
        }
        m_table->showConnectionLost(leaverName + utf8(" a ieșit din joc."));
    }
}  // namespace Frontend::Net
