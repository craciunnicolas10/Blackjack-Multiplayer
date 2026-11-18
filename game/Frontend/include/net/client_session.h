#pragma once

#include <cstddef>
#include <memory>
#include <optional>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/System/String.hpp>

#include <game/game_controller.h>
#include <game/game_session.h>
#include <net/remote_game.h>

namespace Frontend::Net {
    // Masa unui jucător conectat la gazdă. Jocul real e la gazdă; aici RemoteGame îi ține
    // locul, deci GameController și masa merg ca în jocul local. Doar locul nostru primește
    // panoul de pariu și butoanele.
    class ClientSession : public GameSession {
       public:
        ClientSession(std::unique_ptr<sf::TcpSocket> socket, std::size_t mySeat,
                      std::size_t seatCount);

        void start(TableScreen& table) override;
        void update() override;

       private:
        void send(sf::Packet& packet);
        void handleMessage(sf::Packet& packet);
        void stop(const sf::String& message);

        TableScreen* m_table = nullptr;
        std::unique_ptr<sf::TcpSocket> m_socket;  // non-blocking
        std::size_t m_mySeat;
        std::size_t m_seatCount;
        std::shared_ptr<RemoteGame> m_game;
        std::shared_ptr<GameController> m_controller;
        bool m_connectionLost = false;  // o trimitere a eșuat; oprim jocul la update()
        bool m_stopped = false;
    };
}  // namespace Frontend::Net
