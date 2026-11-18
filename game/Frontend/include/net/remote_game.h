#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <SFML/Network/Packet.hpp>

#include <Backend/IGame.h>
#include <Backend/IGameListener.h>
#include <net/game_events.h>
#include <net/game_snapshot.h>

namespace Frontend::Net {
    // Jocul văzut de pe calculatorul unui jucător din rețea (Proxy pentru jocul gazdei).
    // Implementează IGame, deci GameController și masa lucrează cu el exact ca jocul local:
    //  - acțiunile (PlaceBet, Hit, ...) pleacă la gazdă ca mesaje; jocul real decide;
    //  - anunțurile gazdei ajung prin applyEvent() și sunt date mai departe ascultătorilor;
    //  - întrebările (GetPlayer, GetState, ...) primesc răspuns din ultima stare primită.
    class RemoteGame : public Backend::IGame {
       public:
        explicit RemoteGame(std::function<void(sf::Packet&)> send);

        // Primit de la gazdă: actualizează starea și anunță ascultătorii.
        void applyEvent(DecodedEvent event);

        void AddListener(std::weak_ptr<Backend::IGameListener> listener) override;
        void RemoveListener(Backend::IGameListener* listener) override;

        // Jucătorii sunt așezați de gazdă, deci aici nu se poate adăuga nimeni.
        bool AddPlayer(const std::string& name) override;
        // Acțiunile întorc true când au plecat la gazdă; dacă jocul le acceptă, se vede din
        // anunțurile care vin înapoi. Locul jucătorului îl știe gazda din conexiune.
        bool PlaceBet(int playerIndex, int amount) override;
        bool StartRound() override;
        bool Hit() override;
        bool Stand() override;
        bool DoubleDown() override;

        Backend::EGameState GetState() const override;
        int GetPlayerCount() const override;
        int GetCurrentPlayerIndex() const override;
        const Backend::IPlayer* GetPlayer(int playerIndex) const override;
        const Backend::IPlayer* GetDealer() const override;

       private:
        bool sendAction(MessageType type);

        std::function<void(sf::Packet&)> m_send;
        std::vector<std::weak_ptr<Backend::IGameListener>> m_listeners;
        GameSnapshot m_snapshot;
    };
}  // namespace Frontend::Net
