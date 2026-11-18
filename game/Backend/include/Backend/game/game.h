#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Backend/IDeck.h>
#include <Backend/IGame.h>
#include <Backend/IGameListener.h>
#include <Backend/game/dealer.h>
#include <Backend/game/player.h>

namespace Backend {
    inline constexpr int kDealerStandsOn = 17;

    class Game : public IGame {
       public:
        explicit Game(std::shared_ptr<IDeck> deck);

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
        bool CanPlaceBet(int playerIndex) const override;
        bool CanDoubleDown() const override;

       private:
        static constexpr int kNoPlayer = -1;

        // Observer: anunta toti ascultatorii care mai exista. Lucreaza pe o copie a listei, ca
        // un ascultator sa se poata scoate singur in timpul anuntului.
        template <typename Callback>
        void Notify(Callback&& callback) {
            const std::vector<std::weak_ptr<IGameListener>> listeners = m_listeners;
            for (const auto& weakListener : listeners) {
                if (const auto listener = weakListener.lock()) {
                    callback(*listener);
                }
            }
        }

        void SetState(EGameState state);

        bool IsValidPlayerIndex(int playerIndex) const;
        bool HasNameTaken(const std::string& name) const;
        static bool CanAffordMinBet(const Player& player);
        bool AllBetsPlaced() const;
        Player& CurrentPlayer();
        // Indexul vine ca int din API; aici se face o singura data conversia.
        Player& PlayerAt(int playerIndex);
        const Player& PlayerAt(int playerIndex) const;

        void DealInitialCards();
        void DealToPlayer(int playerIndex);
        void DealToDealer(bool faceDown);

        // Da randul primului jucator de la `fromIndex` incolo care mai poate cere carti;
        // daca nu mai e niciunul, joaca dealerul.
        void GiveTurnFrom(int fromIndex);
        void PlayDealer();
        bool DealerNeedsToDraw() const;
        void SettleRound();
        ERoundResult ResultFor(const Player& player) const;
        static int PayoutFor(ERoundResult result, int bet);

        std::shared_ptr<IDeck> m_deck;
        std::vector<std::weak_ptr<IGameListener>> m_listeners;
        // unique_ptr ca pointerii dati prin GetPlayer sa ramana valizi cand se adauga jucatori.
        std::vector<std::unique_ptr<Player>> m_players;
        Dealer m_dealer;
        EGameState m_state = EGameState::WaitingForPlayers;
        int m_currentPlayer = kNoPlayer;
    };
}  // namespace Backend
