#pragma once

#include <string>
#include <vector>

#include <SFML/Network/Packet.hpp>

#include <Backend/Card.h>
#include <Backend/GameEnums.h>
#include <Backend/IGame.h>
#include <Backend/IPlayer.h>

namespace Backend {
    // Un jucator (sau dealerul) asa cum se vede la masa: doar cartile vizibile, scorul lor,
    // banii si pariul. Implementeaza IPlayer, ca RemoteGame sa-l poata da prin GetPlayer().
    class PlayerSnapshot : public IPlayer {
       public:
        std::string GetName() const override;
        std::vector<Card> GetHand() const override;
        int GetScore() const override;
        int GetBalance() const override;
        int GetBet() const override;
        bool IsBusted() const override;

        std::string name;
        std::vector<Card> hand;
        int score = 0;
        int balance = 0;
        int bet = 0;
        bool busted = false;
        bool canPlaceBet = false;  // IGame::CanPlaceBet pentru jucatorul asta
    };

    // Starea jocului la un moment dat: ce trimite gazda dupa fiecare anunt.
    struct GameSnapshot {
        EGameState state = EGameState::WaitingForPlayers;
        int currentPlayer = -1;
        bool canDoubleDown = false;
        std::vector<PlayerSnapshot> players;
        PlayerSnapshot dealer;
    };

    // Doar ce se vede la masa: cartea ascunsa a dealerului nu intra (GetDealer() n-o arata).
    GameSnapshot TakeSnapshot(const IGame& game);

    void WriteSnapshot(sf::Packet& packet, const GameSnapshot& snapshot);
    // False daca pachetul e stricat sau depaseste limitele (vezi protocol.h).
    bool ReadSnapshot(sf::Packet& packet, GameSnapshot& snapshot);
}  // namespace Backend
