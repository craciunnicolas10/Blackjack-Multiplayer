#pragma once

#include <functional>
#include <string>
#include <vector>

#include <Backend/INetworkListener.h>
#include <Backend/NetworkEnums.h>

namespace Frontend {
    // Ascultă o sesiune de rețea din Backend și dă anunțurile mai departe unor funcții.
    // Backend-ul ține ascultătorii ca weak_ptr, iar ecranele nu sunt în shared_ptr, deci
    // ecranul ține un NetworkEvents (shared_ptr) și îi completează funcțiile de care are nevoie.
    class NetworkEvents : public Backend::INetworkListener {
       public:
        std::function<void(const std::vector<std::string>&)> onPlayersChanged;
        std::function<void(Backend::EJoinRejection)> onJoinRejected;
        std::function<void()> onGameStarted;
        std::function<void()> onGameOver;
        std::function<void(const std::string&)> onPlayerLeft;
        std::function<void()> onHostLeft;

        void OnPlayersChanged(const std::vector<std::string>& names) override;
        void OnJoinRejected(Backend::EJoinRejection reason) override;
        void OnGameStarted() override;
        void OnGameOver() override;
        void OnPlayerLeft(const std::string& name) override;
        void OnHostLeft() override;
    };
}  // namespace Frontend
