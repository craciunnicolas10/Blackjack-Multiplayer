#pragma once

#include <string>
#include <vector>

#include <SFML/Network/Packet.hpp>

#include <Backend/Card.h>
#include <Backend/GameEnums.h>
#include <Backend/IGame.h>
#include <Backend/IPlayer.h>

namespace Frontend::Net {
    // Un jucător (sau dealerul) așa cum îl vede masa: doar cărțile vizibile, scorul lor,
    // banii și pariul. Implementează IPlayer, ca RemoteGame să-l poată da prin GetPlayer().
    class PlayerSnapshot : public Backend::IPlayer {
       public:
        std::string GetName() const override;
        std::vector<Backend::Card> GetHand() const override;
        int GetScore() const override;
        int GetBalance() const override;
        int GetBet() const override;

        std::string name;
        std::vector<Backend::Card> hand;
        int score = 0;
        int balance = 0;
        int bet = 0;
    };

    // Starea jocului la un moment dat: ce trimite gazda după fiecare anunț.
    struct GameSnapshot {
        Backend::EGameState state = Backend::EGameState::WaitingForPlayers;
        int currentPlayer = -1;
        std::vector<PlayerSnapshot> players;
        PlayerSnapshot dealer;
    };

    // Doar ce se vede la masă: cartea ascunsă a dealerului nu intră (GetDealer() n-o arată).
    GameSnapshot takeSnapshot(const Backend::IGame& game);

    void writeSnapshot(sf::Packet& packet, const GameSnapshot& snapshot);
    // False dacă pachetul e stricat sau depășește limitele (vezi protocol.h).
    bool readSnapshot(sf::Packet& packet, GameSnapshot& snapshot);
}  // namespace Frontend::Net
