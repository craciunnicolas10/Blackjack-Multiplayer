#include <net/client_session.h>

#include <utility>
#include <vector>

#include <net/game_events.h>
#include <net/protocol.h>
#include <screens/table_screen.h>
#include <ui/utf8.h>

namespace Frontend::Net {
    ClientSession::ClientSession(std::unique_ptr<sf::TcpSocket> socket, std::size_t mySeat,
                                 std::size_t seatCount)
        : m_socket(std::move(socket)), m_mySeat(mySeat), m_seatCount(seatCount) {}

    void ClientSession::start(TableScreen& table) {
        m_table = &table;
        m_game = std::make_shared<RemoteGame>([this](sf::Packet& packet) { send(packet); });

        std::vector<bool> localSeats(m_seatCount, false);
        if (m_mySeat < m_seatCount) {
            localSeats[m_mySeat] = true;
        }
        m_controller = GameController::create(table, m_game, std::move(localSeats));
    }

    void ClientSession::update() {
        if (m_stopped) {
            return;
        }
        sf::Packet packet;
        sf::Socket::Status status = m_socket->receive(packet);
        while (status == sf::Socket::Status::Done && !m_stopped) {
            handleMessage(packet);
            packet.clear();
            status = m_socket->receive(packet);
        }
        if (m_stopped) {
            return;
        }
        if (m_connectionLost || status == sf::Socket::Status::Disconnected ||
            status == sf::Socket::Status::Error) {
            stop(utf8("Gazda a ieșit din joc."));
        }
    }

    void ClientSession::send(sf::Packet& packet) {
        if (!m_stopped && !sendAll(*m_socket, packet)) {
            m_connectionLost = true;
        }
    }

    void ClientSession::handleMessage(sf::Packet& packet) {
        const std::optional<MessageType> type = readMessageType(packet);
        if (!type) {
            return;
        }
        switch (*type) {
            case MessageType::GameEvent:
                if (std::optional<DecodedEvent> event = decodeEvent(packet)) {
                    m_game->applyEvent(std::move(*event));
                }
                break;
            case MessageType::GameOver:
                m_controller->showGameOver();
                break;
            case MessageType::PlayerLeft: {
                sf::String name;
                packet >> name;
                stop(name + utf8(" a ieșit din joc."));
                break;
            }
            default:
                break;  // restul sunt pentru lobby sau pentru gazdă
        }
    }

    void ClientSession::stop(const sf::String& message) {
        m_stopped = true;
        m_socket->disconnect();
        m_table->showConnectionLost(message);
    }
}  // namespace Frontend::Net
