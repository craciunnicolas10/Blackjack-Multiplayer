#include <game/network_events.h>

namespace Frontend {
    void NetworkEvents::OnPlayersChanged(const std::vector<std::string>& names) {
        if (onPlayersChanged) {
            onPlayersChanged(names);
        }
    }

    void NetworkEvents::OnJoinRejected(Backend::EJoinRejection reason) {
        if (onJoinRejected) {
            onJoinRejected(reason);
        }
    }

    void NetworkEvents::OnGameStarted() {
        if (onGameStarted) {
            onGameStarted();
        }
    }

    void NetworkEvents::OnGameOver() {
        if (onGameOver) {
            onGameOver();
        }
    }

    void NetworkEvents::OnPlayerLeft(const std::string& name) {
        if (onPlayerLeft) {
            onPlayerLeft(name);
        }
    }

    void NetworkEvents::OnHostLeft() {
        if (onHostLeft) {
            onHostLeft();
        }
    }
}  // namespace Frontend
