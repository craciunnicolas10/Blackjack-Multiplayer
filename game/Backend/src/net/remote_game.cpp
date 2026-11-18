#include <Backend/net/remote_game.h>

#include <cstddef>
#include <cstdint>
#include <utility>

namespace Backend {
    RemoteGame::RemoteGame(std::function<void(sf::Packet&)> send) : m_send(std::move(send)) {}

    void RemoteGame::ApplyEvent(DecodedEvent event) {
        // Intai starea, apoi anuntul: ca la jocul real, ascultatorul care intreaba jocul in
        // timpul anuntului vede deja starea de dupa.
        m_snapshot = std::move(event.snapshot);
        m_listeners.Notify([&](IGameListener& listener) { event.notify(listener); });
    }

    void RemoteGame::AddListener(std::weak_ptr<IGameListener> listener) {
        m_listeners.Add(std::move(listener));
    }

    void RemoteGame::RemoveListener(IGameListener* listener) { m_listeners.Remove(listener); }

    bool RemoteGame::AddPlayer(const std::string& /*name*/) { return false; }

    bool RemoteGame::PlaceBet(int /*playerIndex*/, int amount) {
        sf::Packet packet = MakeMessage(EMessageType::PlaceBet);
        packet << static_cast<std::int32_t>(amount);
        m_send(packet);
        return true;
    }

    bool RemoteGame::StartRound() { return SendAction(EMessageType::NextRound); }

    bool RemoteGame::Hit() { return SendAction(EMessageType::Hit); }

    bool RemoteGame::Stand() { return SendAction(EMessageType::Stand); }

    bool RemoteGame::DoubleDown() { return SendAction(EMessageType::DoubleDown); }

    bool RemoteGame::SendAction(EMessageType type) {
        sf::Packet packet = MakeMessage(type);
        m_send(packet);
        return true;
    }

    EGameState RemoteGame::GetState() const { return m_snapshot.state; }

    int RemoteGame::GetPlayerCount() const { return static_cast<int>(m_snapshot.players.size()); }

    int RemoteGame::GetCurrentPlayerIndex() const { return m_snapshot.currentPlayer; }

    const IPlayer* RemoteGame::GetPlayer(int playerIndex) const {
        if (playerIndex < 0 || playerIndex >= GetPlayerCount()) {
            return nullptr;
        }
        return &m_snapshot.players[static_cast<std::size_t>(playerIndex)];
    }

    const IPlayer* RemoteGame::GetDealer() const { return &m_snapshot.dealer; }

    bool RemoteGame::CanPlaceBet(int playerIndex) const {
        if (playerIndex < 0 || playerIndex >= GetPlayerCount()) {
            return false;
        }
        return m_snapshot.players[static_cast<std::size_t>(playerIndex)].canPlaceBet;
    }

    bool RemoteGame::CanDoubleDown() const { return m_snapshot.canDoubleDown; }
}  // namespace Backend
