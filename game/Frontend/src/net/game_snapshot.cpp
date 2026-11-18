#include <net/game_snapshot.h>

#include <cstdint>

#include <net/protocol.h>

namespace Frontend::Net {
    namespace {
        PlayerSnapshot snapshotOf(const Backend::IPlayer& player) {
            PlayerSnapshot snapshot;
            snapshot.name = player.GetName();
            snapshot.hand = player.GetHand();
            snapshot.score = player.GetScore();
            snapshot.balance = player.GetBalance();
            snapshot.bet = player.GetBet();
            return snapshot;
        }

        void writePlayer(sf::Packet& packet, const PlayerSnapshot& player) {
            packet << player.name << static_cast<std::uint8_t>(player.hand.size());
            for (const Backend::Card card : player.hand) {
                writeCard(packet, card);
            }
            packet << static_cast<std::int32_t>(player.score)
                   << static_cast<std::int32_t>(player.balance)
                   << static_cast<std::int32_t>(player.bet);
        }

        bool readPlayer(sf::Packet& packet, PlayerSnapshot& player) {
            std::uint8_t handSize = 0;
            if (!(packet >> player.name >> handSize) || handSize > kMaxHandSize) {
                return false;
            }
            player.hand.assign(handSize, {});
            for (Backend::Card& card : player.hand) {
                if (!readCard(packet, card)) {
                    return false;
                }
            }
            std::int32_t score = 0;
            std::int32_t balance = 0;
            std::int32_t bet = 0;
            if (!(packet >> score >> balance >> bet)) {
                return false;
            }
            player.score = score;
            player.balance = balance;
            player.bet = bet;
            return true;
        }
    }  // namespace

    std::string PlayerSnapshot::GetName() const { return name; }

    std::vector<Backend::Card> PlayerSnapshot::GetHand() const { return hand; }

    int PlayerSnapshot::GetScore() const { return score; }

    int PlayerSnapshot::GetBalance() const { return balance; }

    int PlayerSnapshot::GetBet() const { return bet; }

    GameSnapshot takeSnapshot(const Backend::IGame& game) {
        GameSnapshot snapshot;
        snapshot.state = game.GetState();
        snapshot.currentPlayer = game.GetCurrentPlayerIndex();
        for (int i = 0; i < game.GetPlayerCount(); ++i) {
            if (const Backend::IPlayer* player = game.GetPlayer(i)) {
                snapshot.players.push_back(snapshotOf(*player));
            }
        }
        if (const Backend::IPlayer* dealer = game.GetDealer()) {
            snapshot.dealer = snapshotOf(*dealer);
        }
        return snapshot;
    }

    void writeSnapshot(sf::Packet& packet, const GameSnapshot& snapshot) {
        packet << static_cast<std::uint8_t>(snapshot.state)
               << static_cast<std::int32_t>(snapshot.currentPlayer)
               << static_cast<std::uint8_t>(snapshot.players.size());
        for (const PlayerSnapshot& player : snapshot.players) {
            writePlayer(packet, player);
        }
        writePlayer(packet, snapshot.dealer);
    }

    bool readSnapshot(sf::Packet& packet, GameSnapshot& snapshot) {
        std::int32_t currentPlayer = 0;
        std::uint8_t playerCount = 0;
        if (!readEnum(packet, snapshot.state, kLastGameState) ||
            !(packet >> currentPlayer >> playerCount) || playerCount > Backend::kMaxPlayers) {
            return false;
        }
        snapshot.currentPlayer = currentPlayer;
        snapshot.players.assign(playerCount, {});
        for (PlayerSnapshot& player : snapshot.players) {
            if (!readPlayer(packet, player)) {
                return false;
            }
        }
        return readPlayer(packet, snapshot.dealer);
    }
}  // namespace Frontend::Net
