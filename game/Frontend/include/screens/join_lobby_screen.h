#pragma once

#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <app/app_context.h>
#include <screens/screen.h>
#include <ui/button.h>
#include <ui/felt_background.h>
#include <ui/text_input.h>

namespace Frontend {
    // Jocul în rețea, la un jucător: își scrie numele și adresa gazdei, se conectează, apoi
    // vede cine mai e în joc până când gazda apasă Start.
    class JoinLobbyScreen : public Screen {
       public:
        explicit JoinLobbyScreen(AppContext context);

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void draw(sf::RenderTarget& target) override;
        bool isTypingText() const override;

       private:
        void applyPendingActions();
        void connect();
        void disconnect(const sf::String& error);
        void receiveMessages();
        // Gazda a pornit jocul: trecem la masă cu ClientSession.
        void startGame(sf::Packet& packet);
        void showError(const sf::String& message);
        void showHint(const sf::String& message);

        AppContext m_context;
        FeltBackground m_background;
        sf::Text m_title;
        sf::Text m_hint;
        sf::Text m_error;
        TextInput m_nameInput;
        TextInput m_addressInput;
        Button m_connectButton;
        Button m_backButton;

        std::unique_ptr<sf::TcpSocket> m_socket;  // non-blocking; nullptr = neconectat
        std::vector<sf::String> m_players;        // primiți de la gazdă

        bool m_connectRequested = false;
        bool m_backRequested = false;
    };
}  // namespace Frontend
