#pragma once

#include <functional>
#include <optional>

#include <SFML/Network/Packet.hpp>

#include <Backend/IGame.h>
#include <Backend/IGameListener.h>
#include <net/game_snapshot.h>
#include <net/protocol.h>

namespace Frontend::Net {
    // La gazdă: ascultă jocul real și face din fiecare anunț un mesaj GameEvent, cu starea
    // jocului de după anunț. `send` îl trimite tuturor jucătorilor din rețea.
    class EventEncoder : public Backend::IGameListener {
       public:
        EventEncoder(const Backend::IGame& game, std::function<void(sf::Packet&)> send);

        void OnGameStateChanged(Backend::EGameState newState) override;
        void OnPlayerAdded(int playerIndex) override;
        void OnBetPlaced(int playerIndex, int amount) override;
        void OnPlayerCardDealt(int playerIndex, Backend::Card card) override;
        void OnDealerCardDealt(Backend::Card card, bool isFaceDown) override;
        void OnDealerCardRevealed(Backend::Card card) override;
        void OnTurnChanged(int playerIndex) override;
        void OnRoundEnded(int playerIndex, Backend::ERoundResult result) override;

       private:
        static sf::Packet begin(EventType type);
        // Adaugă starea jocului și trimite.
        void finish(sf::Packet& packet);

        const Backend::IGame& m_game;
        std::function<void(sf::Packet&)> m_send;
    };

    // La jucător: un GameEvent citit din rețea.
    struct DecodedEvent {
        // Anunță evenimentul unui ascultător, exact ca jocul real.
        std::function<void(Backend::IGameListener&)> notify;
        // Starea jocului de după eveniment.
        GameSnapshot snapshot;
    };

    // Citește restul unui mesaj GameEvent (după MessageType). nullopt dacă e stricat.
    std::optional<DecodedEvent> decodeEvent(sf::Packet& packet);
}  // namespace Frontend::Net
