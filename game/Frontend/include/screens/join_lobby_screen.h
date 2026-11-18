#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

#include <Backend/INetworkClient.h>
#include <app/app_context.h>
#include <game/network_events.h>
#include <screens/screen.h>
#include <ui/button.h>
#include <ui/felt_background.h>
#include <ui/text_input.h>

namespace Frontend {
    // Jocul în rețea, la un jucător: își scrie numele și adresa gazdei, se conectează, apoi
    // vede cine mai e în joc până când gazda apasă Start.
    // Conectarea și mesajele sunt în Backend (INetworkClient); ecranul doar le arată.
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
        // Înapoi la câmpurile de conectare, cu un mesaj de eroare.
        void disconnect(const sf::String& error);
        void openTable();
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

        std::shared_ptr<Backend::INetworkClient> m_client;  // nullptr = neconectat
        std::shared_ptr<NetworkEvents> m_events;
        std::vector<sf::String> m_players;  // primiți de la gazdă

        bool m_connectRequested = false;
        bool m_backRequested = false;
        bool m_gameStarted = false;
        // Legătura s-a închis în timpul unui anunț; o tratăm după ce se termină Update().
        std::optional<sf::String> m_disconnectError;
    };
}  // namespace Frontend
