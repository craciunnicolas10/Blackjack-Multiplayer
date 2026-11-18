#pragma once

#include <memory>
#include <vector>

#include <SFML/System/String.hpp>

#include <Backend/IGame.h>
#include <game/game_controller.h>
#include <game/game_session.h>

namespace Frontend {
    // Toți jucătorii la același calculator, pe rând: jocul (Backend) rulează aici.
    class LocalSession : public GameSession {
       public:
        explicit LocalSession(std::vector<sf::String> playerNames);

        void start(TableScreen& table) override;

       private:
        std::vector<sf::String> m_playerNames;
        std::shared_ptr<Backend::IGame> m_game;
        std::shared_ptr<GameController> m_controller;
    };
}  // namespace Frontend
