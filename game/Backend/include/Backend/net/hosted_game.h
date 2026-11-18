#pragma once

#include <functional>
#include <memory>
#include <string>

#include <Backend/IGame.h>

namespace Backend {
    // La gazda: jocul real, cu un singur lucru in plus (Decorator peste IGame): cand
    // StartRound e refuzat, adica nimeni nu mai are bani de pariu, anunta jocul in retea.
    // Restul functiilor merg direct la jocul real.
    class HostedGame : public IGame {
       public:
        HostedGame(std::shared_ptr<IGame> game, std::function<void()> onStartRoundRefused);

        void AddListener(std::weak_ptr<IGameListener> listener) override;
        void RemoveListener(IGameListener* listener) override;

        bool AddPlayer(const std::string& name) override;
        bool PlaceBet(int playerIndex, int amount) override;
        bool StartRound() override;
        bool Hit() override;
        bool Stand() override;
        bool DoubleDown() override;

        EGameState GetState() const override;
        int GetPlayerCount() const override;
        int GetCurrentPlayerIndex() const override;
        const IPlayer* GetPlayer(int playerIndex) const override;
        const IPlayer* GetDealer() const override;

       private:
        std::shared_ptr<IGame> m_game;
        std::function<void()> m_onStartRoundRefused;
    };
}  // namespace Backend
