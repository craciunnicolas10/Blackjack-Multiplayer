#pragma once

#include <memory>

#include <Backend/INetworkSession.h>
#include <game/game_controller.h>
#include <game/game_session.h>
#include <game/network_events.h>

namespace Frontend {
    // Masa unui joc în rețea, la gazdă sau la un jucător. Rețeaua e toată în Backend
    // (INetworkSession); aici doar legăm jocul ei de masă, ca în jocul local, și arătăm
    // pe masă ce anunță rețeaua (sfârșitul jocului, cine a ieșit).
    class NetworkSession : public GameSession {
       public:
        explicit NetworkSession(std::shared_ptr<Backend::INetworkSession> network);

        void start(TableScreen& table) override;
        // Citește ce a venit din rețea (anunțurile jocului ajung la masă prin GameController).
        void update() override;

       private:
        std::shared_ptr<Backend::INetworkSession> m_network;
        std::shared_ptr<NetworkEvents> m_events;
        std::shared_ptr<GameController> m_controller;
    };
}  // namespace Frontend
