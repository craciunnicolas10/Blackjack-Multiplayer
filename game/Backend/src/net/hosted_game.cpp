#include <Backend/net/hosted_game.h>

#include <utility>

namespace Backend {
    HostedGame::HostedGame(std::shared_ptr<IGame> game, std::function<void()> onStartRoundRefused)
        : m_game(std::move(game)), m_onStartRoundRefused(std::move(onStartRoundRefused)) {}

    void HostedGame::AddListener(std::weak_ptr<IGameListener> listener) {
        m_game->AddListener(std::move(listener));
    }

    void HostedGame::RemoveListener(IGameListener* listener) { m_game->RemoveListener(listener); }

    bool HostedGame::AddPlayer(const std::string& name) { return m_game->AddPlayer(name); }

    bool HostedGame::PlaceBet(int playerIndex, int amount) {
        return m_game->PlaceBet(playerIndex, amount);
    }

    bool HostedGame::StartRound() {
        const bool started = m_game->StartRound();
        // Refuzat dupa o runda = nimeni nu mai are bani; inainte de prima runda nu se poate.
        if (!started && m_game->GetState() == EGameState::RoundOver && m_onStartRoundRefused) {
            m_onStartRoundRefused();
        }
        return started;
    }

    bool HostedGame::Hit() { return m_game->Hit(); }

    bool HostedGame::Stand() { return m_game->Stand(); }

    bool HostedGame::DoubleDown() { return m_game->DoubleDown(); }

    EGameState HostedGame::GetState() const { return m_game->GetState(); }

    int HostedGame::GetPlayerCount() const { return m_game->GetPlayerCount(); }

    int HostedGame::GetCurrentPlayerIndex() const { return m_game->GetCurrentPlayerIndex(); }

    const IPlayer* HostedGame::GetPlayer(int playerIndex) const {
        return m_game->GetPlayer(playerIndex);
    }

    const IPlayer* HostedGame::GetDealer() const { return m_game->GetDealer(); }
}  // namespace Backend
