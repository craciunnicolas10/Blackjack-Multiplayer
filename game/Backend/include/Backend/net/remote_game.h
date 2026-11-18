#pragma once

#include <functional>
#include <memory>
#include <string>

#include <SFML/Network/Packet.hpp>

#include <Backend/IGame.h>
#include <Backend/IGameListener.h>
#include <Backend/net/game_events.h>
#include <Backend/net/game_snapshot.h>
#include <Backend/net/listener_list.h>
#include <Backend/net/protocol.h>

namespace Backend {
    // Jocul vazut de pe calculatorul unui jucator din retea (Proxy pentru jocul gazdei).
    // Implementeaza IGame, deci Frontend-ul lucreaza cu el exact ca jocul local:
    //  - actiunile (PlaceBet, Hit, ...) pleaca la gazda ca mesaje; jocul real decide;
    //  - anunturile gazdei ajung prin ApplyEvent() si sunt date mai departe ascultatorilor;
    //  - intrebarile (GetPlayer, GetState, ...) primesc raspuns din ultima stare primita.
    class RemoteGame : public IGame {
       public:
        explicit RemoteGame(std::function<void(sf::Packet&)> send);

        // Primit de la gazda: actualizeaza starea si anunta ascultatorii.
        void ApplyEvent(DecodedEvent event);

        void AddListener(std::weak_ptr<IGameListener> listener) override;
        void RemoveListener(IGameListener* listener) override;

        // Jucatorii sunt asezati de gazda, deci aici nu se poate adauga nimeni.
        bool AddPlayer(const std::string& name) override;
        // Actiunile intorc true cand au plecat la gazda; daca jocul le accepta, se vede din
        // anunturile care vin inapoi. Locul jucatorului il stie gazda din conexiune.
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
        bool SendAction(EMessageType type);

        std::function<void(sf::Packet&)> m_send;
        ListenerList<IGameListener> m_listeners;
        GameSnapshot m_snapshot;
    };
}  // namespace Backend
