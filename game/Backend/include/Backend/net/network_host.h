#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <Backend/IGame.h>
#include <Backend/INetworkHost.h>
#include <Backend/INetworkListener.h>
#include <Backend/net/game_events.h>
#include <Backend/net/hosted_game.h>
#include <Backend/net/listener_list.h>
#include <Backend/net/protocol.h>
#include <Backend/net/reusable_listener.h>

namespace Backend {
    // Gazda jocului in retea (vezi INetworkHost). In lobby primeste conexiuni si verifica cine
    // intra; dupa StartGame tine jocul real, primeste actiunile jucatorilor (doar cand e randul
    // lor) si trimite fiecare anunt al jocului tuturor (EventEncoder).
    class NetworkHost : public INetworkHost, public std::enable_shared_from_this<NetworkHost> {
       public:
        explicit NetworkHost(std::string hostName);

        // Incepe sa astepte jucatori pe primul port liber. False daca nu e niciunul.
        bool Listen();

        void AddListener(std::weak_ptr<INetworkListener> listener) override;
        void RemoveListener(INetworkListener* listener) override;
        void Update() override;
        bool IsHost() const override;
        std::vector<std::string> GetPlayerNames() const override;
        int GetLocalSeat() const override;
        std::shared_ptr<IGame> GetGame() const override;

        std::string GetAddress() const override;
        bool CanStartGame() const override;
        bool StartGame() override;

       private:
        // Un calculator conectat. Pana trimite Join nu are nume si nu e in lista.
        struct Connection {
            std::unique_ptr<sf::TcpSocket> socket;  // non-blocking
            std::string name;
            int seat = -1;
            bool joined = false;
            bool closed = false;
        };

        void AcceptConnections();
        void ReceiveMessages();
        // True daca s-a schimbat lista din lobby (a intrat cineva).
        bool HandleJoin(Connection& connection, sf::Packet& packet);
        void Reject(Connection& connection, EJoinRejection reason);
        // Actiunile trec doar daca e randul jucatorului; restul il verifica jocul.
        void HandleAction(const Connection& connection, EMessageType type, sf::Packet& packet);
        void BroadcastPlayerList();
        void Broadcast(sf::Packet& packet);
        void OnStartRoundRefused();
        // Cineva a iesit in timpul jocului: jocul se opreste pentru toti.
        void StopGame(const std::string& leaverName);
        int JoinedCount() const;

        std::string m_hostName;
        ReusableListener m_listener;
        unsigned short m_port = 0;
        std::vector<Connection> m_connections;
        ListenerList<INetworkListener> m_listeners;

        std::shared_ptr<IGame> m_realGame;
        std::shared_ptr<HostedGame> m_game;
        std::shared_ptr<EventEncoder> m_encoder;
        bool m_started = false;
        bool m_stopped = false;
        // Cine a picat in timp ce ii trimiteam ceva; oprim jocul la urmatorul Update().
        std::optional<std::string> m_lostPlayer;
    };
}  // namespace Backend
