#pragma once

#include <memory>
#include <string>
#include <vector>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <Backend/IGame.h>
#include <Backend/INetworkClient.h>
#include <Backend/INetworkListener.h>
#include <Backend/net/listener_list.h>
#include <Backend/net/remote_game.h>

namespace Backend {
    // Un jucator conectat la gazda (vezi INetworkClient). Jocul real e la gazda; aici
    // RemoteGame ii tine locul si e tinut la zi din mesajele primite.
    class NetworkClient : public INetworkClient,
                          public std::enable_shared_from_this<NetworkClient> {
       public:
        // `socket` e deja conectat si a trimis Join.
        explicit NetworkClient(std::unique_ptr<sf::TcpSocket> socket);

        void AddListener(std::weak_ptr<INetworkListener> listener) override;
        void RemoveListener(INetworkListener* listener) override;
        void Update() override;
        bool IsHost() const override;
        std::vector<std::string> GetPlayerNames() const override;
        int GetLocalSeat() const override;
        std::shared_ptr<IGame> GetGame() const override;

       private:
        // False daca trebuie oprit cititul pentru acum (legatura inchisa sau jocul abia a
        // pornit: anunturile lui asteapta pana isi pune Frontend-ul ascultatorii).
        bool HandleMessage(sf::Packet& packet);
        bool StartGame(sf::Packet& packet);
        void Send(sf::Packet& packet);
        void Close();

        std::unique_ptr<sf::TcpSocket> m_socket;  // non-blocking
        ListenerList<INetworkListener> m_listeners;
        std::vector<std::string> m_names;
        int m_seat = -1;
        std::shared_ptr<RemoteGame> m_game;
        bool m_closed = false;
        bool m_sendFailed = false;
    };
}  // namespace Backend
