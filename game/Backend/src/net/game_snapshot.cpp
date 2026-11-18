#include <Backend/net/game_snapshot.h>

#include <cstdint>

#include <Backend/net/protocol.h>

namespace Backend {
    namespace {
        PlayerSnapshot SnapshotOf(const IPlayer& player) {
            PlayerSnapshot snapshot;
            snapshot.name = player.GetName();
            snapshot.hand = player.GetHand();
            snapshot.score = player.GetScore();
            snapshot.balance = player.GetBalance();
            snapshot.bet = player.GetBet();
            snapshot.busted = player.IsBusted();
            return snapshot;
        }

        void WritePlayer(sf::Packet& packet, const PlayerSnapshot& player) {
            packet << player.name << static_cast<std::uint8_t>(player.hand.size());
            for (const Card card : player.hand) {
                WriteCard(packet, card);
            }
            packet << static_cast<std::int32_t>(player.score)
                   << static_cast<std::int32_t>(player.balance)
                   << static_cast<std::int32_t>(player.bet) << player.busted << player.canPlaceBet;
        }

        bool ReadPlayer(sf::Packet& packet, PlayerSnapshot& player) {
            std::uint8_t handSize = 0;
            if (!(packet >> player.name >> handSize) || handSize > kMaxHandSize) {
                return false;
            }
            player.hand.assign(handSize, {});
            for (Card& card : player.hand) {
                if (!ReadCard(packet, card)) {
                    return false;
                }
            }
            std::int32_t score = 0;
            std::int32_t balance = 0;
            std::int32_t bet = 0;
            if (!(packet >> score >> balance >> bet >> player.busted >> player.canPlaceBet)) {
                return false;
            }
            player.score = score;
            player.balance = balance;
            player.bet = bet;
            return true;
        }
    }  // namespace

    std::string PlayerSnapshot::GetName() const { return name; }

    std::vector<Card> PlayerSnapshot::GetHand() const { return hand; }

    int PlayerSnapshot::GetScore() const { return score; }

    int PlayerSnapshot::GetBalance() const { return balance; }

    int PlayerSnapshot::GetBet() const { return bet; }

    bool PlayerSnapshot::IsBusted() const { return busted; }

    GameSnapshot TakeSnapshot(const IGame& game) {
        GameSnapshot snapshot;
        snapshot.state = game.GetState();
        snapshot.currentPlayer = game.GetCurrentPlayerIndex();
        snapshot.canDoubleDown = game.CanDoubleDown();
        for (int i = 0; i < game.GetPlayerCount(); ++i) {
            if (const IPlayer* player = game.GetPlayer(i)) {
                PlayerSnapshot& added = snapshot.players.emplace_back(SnapshotOf(*player));
                added.canPlaceBet = game.CanPlaceBet(i);
            }
        }
        if (const IPlayer* dealer = game.GetDealer()) {
            snapshot.dealer = SnapshotOf(*dealer);
        }
        return snapshot;
    }

    void WriteSnapshot(sf::Packet& packet, const GameSnapshot& snapshot) {
        packet << static_cast<std::uint8_t>(snapshot.state)
               << static_cast<std::int32_t>(snapshot.currentPlayer) << snapshot.canDoubleDown
               << static_cast<std::uint8_t>(snapshot.players.size());
        for (const PlayerSnapshot& player : snapshot.players) {
            WritePlayer(packet, player);
        }
        WritePlayer(packet, snapshot.dealer);
    }

    bool ReadSnapshot(sf::Packet& packet, GameSnapshot& snapshot) {
        std::int32_t currentPlayer = 0;
        std::uint8_t playerCount = 0;
        if (!ReadEnum(packet, snapshot.state, kLastGameState) ||
            !(packet >> currentPlayer >> snapshot.canDoubleDown >> playerCount) ||
            playerCount > kMaxPlayers) {
            return false;
        }
        snapshot.currentPlayer = currentPlayer;
        snapshot.players.assign(playerCount, {});
        for (PlayerSnapshot& player : snapshot.players) {
            if (!ReadPlayer(packet, player)) {
                return false;
            }
        }
        return ReadPlayer(packet, snapshot.dealer);
    }
}  // namespace Backend
