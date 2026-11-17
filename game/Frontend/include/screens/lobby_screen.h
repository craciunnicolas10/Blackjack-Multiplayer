#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

#include <app/app_context.h>
#include <screens/screen.h>
#include <ui/button.h>
#include <ui/felt_background.h>
#include <ui/text_input.h>

namespace Frontend {
    // Ecranul în care se adaugă jucătorii (1-4) înainte de masă.
    class LobbyScreen : public Screen {
       public:
        static constexpr std::size_t kMaxPlayers = 4;
        static constexpr std::size_t kMaxNameLength = 12;

        explicit LobbyScreen(AppContext context);

        void handleEvent(const sf::Event& event) override;
        void update(float dt) override;
        void draw(sf::RenderTarget& target) override;
        bool isTypingText() const override { return m_nameInput.isFocused(); }

       private:
        // Butoanele doar marchează ce s-a cerut; modificăm lista după ce s-a terminat
        // parcurgerea butoanelor, ca să nu ștergem un buton în timp ce îi rulează callback-ul.
        void applyPendingActions();
        void tryAddPlayer();
        void removePlayer(std::size_t index);
        void rebuildPlayerRows();
        void showError(const sf::String& message);

        AppContext m_context;
        FeltBackground m_background;
        sf::Text m_title;
        sf::Text m_hint;
        sf::Text m_error;
        TextInput m_nameInput;
        Button m_addButton;
        Button m_backButton;
        Button m_startButton;

        std::vector<sf::String> m_players;
        std::vector<Button> m_removeButtons;

        bool m_addRequested = false;
        std::optional<std::size_t> m_removeRequested;
        bool m_backRequested = false;
        bool m_startRequested = false;
    };
}  // namespace Frontend
