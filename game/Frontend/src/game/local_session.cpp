#include <game/local_session.h>

#include <utility>

#include <ui/utf8.h>

namespace Frontend {
    LocalSession::LocalSession(std::vector<sf::String> playerNames)
        : m_playerNames(std::move(playerNames)) {}

    void LocalSession::start(TableScreen& table) {
        m_game = Backend::IGame::Produce();
        m_controller = GameController::create(table, m_game,
                                              std::vector<bool>(m_playerNames.size(), true));
        for (const sf::String& name : m_playerNames) {
            m_game->AddPlayer(toUtf8String(name));
        }
        m_game->StartRound();
    }
}  // namespace Frontend
