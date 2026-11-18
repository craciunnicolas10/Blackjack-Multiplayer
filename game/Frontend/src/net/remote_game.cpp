#include <net/remote_game.h>

#include <algorithm>
#include <cstdint>
#include <utility>

#include <net/protocol.h>

namespace Frontend::Net {
    RemoteGame::RemoteGame(std::function<void(sf::Packet&)> send) : m_send(std::move(send)) {}

    void RemoteGame::applyEvent(DecodedEvent event) {
        // Întâi starea, apoi anunțul: ca la jocul real, ascultătorul care întreabă jocul în
        // timpul anunțului vede deja starea de după.
        m_snapshot = std::move(event.snapshot);
        // Pe o copie, ca un ascultător să se poată scoate singur în timpul anunțului.
        const auto listeners = m_listeners;
        for (const auto& weakListener : listeners) {
            if (const auto listener = weakListener.lock()) {
                event.notify(*listener);
            }
        }
    }

    void RemoteGame::AddListener(std::weak_ptr<Backend::IGameListener> listener) {
        if (!listener.expired()) {
            m_listeners.push_back(std::move(listener));
        }
    }

    void RemoteGame::RemoveListener(Backend::IGameListener* listener) {
        std::erase_if(m_listeners, [listener](const auto& existing) {
            const auto locked = existing.lock();
            return !locked || locked.get() == listener;
        });
    }

    bool RemoteGame::AddPlayer(const std::string& /*name*/) { return false; }

    bool RemoteGame::PlaceBet(int /*playerIndex*/, int amount) {
        sf::Packet packet = makeMessage(MessageType::PlaceBet);
        packet << static_cast<std::int32_t>(amount);
        m_send(packet);
        return true;
    }

    bool RemoteGame::StartRound() { return sendAction(MessageType::NextRound); }

    bool RemoteGame::Hit() { return sendAction(MessageType::Hit); }

    bool RemoteGame::Stand() { return sendAction(MessageType::Stand); }

    bool RemoteGame::DoubleDown() { return sendAction(MessageType::DoubleDown); }

    bool RemoteGame::sendAction(MessageType type) {
        sf::Packet packet = makeMessage(type);
        m_send(packet);
        return true;
    }

    Backend::EGameState RemoteGame::GetState() const { return m_snapshot.state; }

    int RemoteGame::GetPlayerCount() const { return static_cast<int>(m_snapshot.players.size()); }

    int RemoteGame::GetCurrentPlayerIndex() const { return m_snapshot.currentPlayer; }

    const Backend::IPlayer* RemoteGame::GetPlayer(int playerIndex) const {
        if (playerIndex < 0 || playerIndex >= GetPlayerCount()) {
            return nullptr;
        }
        return &m_snapshot.players[static_cast<std::size_t>(playerIndex)];
    }

    const Backend::IPlayer* RemoteGame::GetDealer() const { return &m_snapshot.dealer; }
}  // namespace Frontend::Net
