#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/System/String.hpp>

#include <Backend/IGame.h>
#include <game/game_controller.h>
#include <game/game_session.h>
#include <net/game_events.h>

namespace Frontend::Net {
    // Un jucător conectat la gazdă din rețea.
    struct RemotePlayer {
        std::unique_ptr<sf::TcpSocket> socket;  // non-blocking
        std::size_t seat = 0;
        sf::String name;
    };

    // Masa gazdei: jocul real (Backend) rulează aici. Gazda e la locul 0 și joacă de la
    // tastatură ca în jocul local; ceilalți trimit acțiunile prin rețea, iar fiecare anunț
    // al jocului pleacă la toți (EventEncoder).
    class HostSession : public GameSession {
       public:
        HostSession(std::vector<sf::String> playerNames, std::vector<RemotePlayer> remotePlayers);

        void start(TableScreen& table) override;
        void update() override;

       private:
        void broadcast(sf::Packet& packet);
        // Ce a trimis un jucător. Acțiunile trec doar dacă e rândul lui; jocul verifică restul.
        void handleMessage(const RemotePlayer& player, sf::Packet& packet);
        // Cineva a ieșit: jocul se oprește pentru toți.
        void stop(const sf::String& leaverName);

        TableScreen* m_table = nullptr;
        std::vector<sf::String> m_playerNames;
        std::vector<RemotePlayer> m_remotePlayers;
        std::shared_ptr<Backend::IGame> m_game;
        std::shared_ptr<GameController> m_controller;
        std::shared_ptr<EventEncoder> m_encoder;
        // Cine a picat în timp ce îi trimiteam ceva; oprim jocul la următorul update().
        std::optional<sf::String> m_lostPlayer;
        bool m_stopped = false;
    };
}  // namespace Frontend::Net
