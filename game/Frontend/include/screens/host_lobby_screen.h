#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <app/app_context.h>
#include <screens/screen.h>
#include <ui/button.h>
#include <ui/felt_background.h>
#include <ui/text_input.h>

namespace Frontend {
    // Jocul în rețea, la gazdă: își scrie numele și creează jocul, apoi așteaptă să intre
    // ceilalți (2-4 jucători în total, câte unul pe calculator) și apasă Start.
    // Ceilalți se conectează cu adresa afișată aici (vezi JoinLobbyScreen).
    class HostLobbyScreen : public Screen {
       public:
        explicit HostLobbyScreen(AppContext context);

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void draw(sf::RenderTarget& target) override;
        bool isTypingText() const override { return !m_hosting && m_nameInput.isFocused(); }

       private:
        // Un calculator conectat. Până trimite Join nu are nume și nu e în listă.
        struct Connection {
            std::unique_ptr<sf::TcpSocket> socket;  // non-blocking
            std::optional<sf::String> name;
            bool closed = false;
        };

        void applyPendingActions();
        void startHosting();
        void acceptConnections();
        void receiveMessages();
        void handleJoin(Connection& connection, sf::Packet& packet);
        void reject(Connection& connection, const sf::String& reason);
        void broadcastPlayerList();
        // Gazda prima, apoi ceilalți în ordinea în care au intrat: ăsta e și locul la masă.
        std::vector<sf::String> playerNames() const;
        std::size_t joinedCount() const;
        void startGame();
        void showError(const sf::String& message);
        void refreshButtons();

        AppContext m_context;
        FeltBackground m_background;
        sf::Text m_title;
        sf::Text m_hint;
        sf::Text m_address;
        sf::Text m_error;
        TextInput m_nameInput;
        Button m_createButton;
        Button m_backButton;
        Button m_startButton;

        sf::TcpListener m_listener;
        bool m_hosting = false;
        sf::String m_hostName;
        std::vector<Connection> m_connections;

        bool m_createRequested = false;
        bool m_backRequested = false;
        bool m_startRequested = false;
    };
}  // namespace Frontend
