#pragma once

#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>

#include <Backend/INetworkHost.h>
#include <app/app_context.h>
#include <game/network_events.h>
#include <screens/screen.h>
#include <ui/button.h>
#include <ui/felt_background.h>
#include <ui/text_input.h>

namespace Frontend {
    // Jocul în rețea, la gazdă: își scrie numele și creează jocul, apoi așteaptă să intre
    // ceilalți (2-4 jucători în total, câte unul pe calculator) și apasă Start.
    // Conexiunile și regulile de intrare sunt în Backend (INetworkHost); ecranul doar le arată.
    class HostLobbyScreen : public Screen {
       public:
        explicit HostLobbyScreen(AppContext context);

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void draw(sf::RenderTarget& target) override;
        bool isTypingText() const override { return !m_host && m_nameInput.isFocused(); }

       private:
        void applyPendingActions();
        void createGame();
        void openTable();
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

        std::shared_ptr<Backend::INetworkHost> m_host;  // nullptr până se creează jocul
        std::shared_ptr<NetworkEvents> m_events;
        std::vector<sf::String> m_players;

        bool m_createRequested = false;
        bool m_backRequested = false;
        bool m_startRequested = false;
        bool m_gameStarted = false;
    };
}  // namespace Frontend
