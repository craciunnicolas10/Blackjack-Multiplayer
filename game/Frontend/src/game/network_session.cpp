#include <game/network_session.h>

#include <utility>
#include <vector>

#include <screens/table_screen.h>
#include <ui/utf8.h>

namespace Frontend {
    NetworkSession::NetworkSession(std::shared_ptr<Backend::INetworkSession> network)
        : m_network(std::move(network)), m_events(std::make_shared<NetworkEvents>()) {}

    void NetworkSession::start(TableScreen& table) {
        // Doar locul de pe calculatorul ăsta primește panoul de pariu și butoanele.
        std::vector<bool> localSeats(table.getSeatCount(), false);
        const int mySeat = m_network->GetLocalSeat();
        if (mySeat >= 0 && static_cast<std::size_t>(mySeat) < localSeats.size()) {
            localSeats[static_cast<std::size_t>(mySeat)] = true;
        }
        m_controller = GameController::create(table, m_network->GetGame(), std::move(localSeats));

        m_events->onGameOver = [this] { m_controller->showGameOver(); };
        m_events->onPlayerLeft = [&table](const std::string& name) {
            table.showConnectionLost(utf8(name) + utf8(" a ieșit din joc."));
        };
        m_events->onHostLeft = [&table] {
            table.showConnectionLost(utf8("Gazda a ieșit din joc."));
        };
        m_network->AddListener(m_events);

        // Prima rundă o pornește gazda, acum că masa ei ascultă jocul.
        if (m_network->IsHost()) {
            m_network->GetGame()->StartRound();
        }
    }

    void NetworkSession::update() { m_network->Update(); }
}  // namespace Frontend
