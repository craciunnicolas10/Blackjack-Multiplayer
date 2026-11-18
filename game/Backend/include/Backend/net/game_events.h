#pragma once

#include <functional>
#include <optional>

#include <SFML/Network/Packet.hpp>

#include <Backend/IGame.h>
#include <Backend/IGameListener.h>
#include <Backend/net/game_snapshot.h>
#include <Backend/net/protocol.h>

namespace Backend {
    // La gazda: asculta jocul real si face din fiecare anunt un mesaj GameEvent, cu starea
    // jocului de dupa anunt. `send` il trimite tuturor jucatorilor din retea.
    class EventEncoder : public IGameListener {
       public:
        EventEncoder(const IGame& game, std::function<void(sf::Packet&)> send);

        void OnGameStateChanged(EGameState newState) override;
        void OnPlayerAdded(int playerIndex) override;
        void OnBetPlaced(int playerIndex, int amount) override;
        void OnPlayerCardDealt(int playerIndex, Card card) override;
        void OnDealerCardDealt(Card card, bool isFaceDown) override;
        void OnDealerCardRevealed(Card card) override;
        void OnTurnChanged(int playerIndex) override;
        void OnRoundEnded(int playerIndex, ERoundResult result) override;

       private:
        static sf::Packet Begin(EEventType type);
        // Adauga starea jocului si trimite.
        void Finish(sf::Packet& packet);

        const IGame& m_game;
        std::function<void(sf::Packet&)> m_send;
    };

    // La jucator: un GameEvent citit din retea.
    struct DecodedEvent {
        // Anunta evenimentul unui ascultator, exact ca jocul real.
        std::function<void(IGameListener&)> notify;
        // Starea jocului de dupa eveniment.
        GameSnapshot snapshot;
    };

    // Citeste restul unui mesaj GameEvent (dupa EMessageType). nullopt daca e stricat.
    std::optional<DecodedEvent> DecodeEvent(sf::Packet& packet);
}  // namespace Backend
