#include <game/fake_game.h>

#include <algorithm>
#include <optional>

#include <screens/table_screen.h>
#include <ui/card_textures.h>

namespace Frontend {
    namespace {
        // Sub atâtea cărți rămase în pachet luăm unul nou, amestecat.
        constexpr std::size_t kReshuffleBelow = 15;
    }  // namespace

    FakeGame::FakeGame(TableScreen& table)
        : m_table(table), m_rng(std::random_device{}()), m_players(table.getSeatCount()) {
        m_table.setOnHit([this] { hit(); });
        m_table.setOnStand([this] { stand(); });
        m_table.setOnDoubleDown([this] { doubleDown(); });
        m_table.setOnBet([this](int amount) { placeBet(amount); });
        for (std::size_t seat = 0; seat < m_players.size(); ++seat) {
            m_table.setPlayerMoney(seat, m_players[seat].money);
        }
    }

    void FakeGame::startRound() {
        m_table.clearRound();
        m_dealer.clear();
        m_dealerRevealed = false;
        m_roundOver = false;

        for (std::size_t seat = 0; seat < m_players.size(); ++seat) {
            Player& player = m_players[seat];
            player.hand.clear();
            if (player.money < kMinBet) {
                player.money = kStartingMoney;  // e doar demo: cine a rămas fără bani o ia de la capăt
            }
            player.bet = 0;
            m_table.setPlayerMoney(seat, player.money);
            m_table.setPlayerBet(seat, player.bet);
        }

        askBet(0);
    }

    void FakeGame::askBet(std::size_t seat) {
        m_bettingSeat = seat;
        const Player& player = m_players[seat];
        m_table.startBetting(seat, player.money, kMinBet, kMaxBet, player.lastBet);
    }

    void FakeGame::placeBet(int amount) {
        if (m_bettingSeat >= m_players.size()) {
            return;
        }
        Player& player = m_players[m_bettingSeat];
        if (amount < kMinBet || amount > kMaxBet || amount > player.money) {
            return;  // panoul nu lasă asta, dar jocul verifică oricum
        }
        player.bet = amount;
        player.lastBet = amount;
        player.money -= amount;
        m_table.setPlayerMoney(m_bettingSeat, player.money);
        m_table.setPlayerBet(m_bettingSeat, player.bet);

        if (m_bettingSeat + 1 < m_players.size()) {
            askBet(m_bettingSeat + 1);
        } else {
            m_bettingSeat = m_players.size();
            m_table.endBetting();
            dealInitialCards();
        }
    }

    void FakeGame::dealInitialCards() {
        // Ca la masă: câte o carte fiecăruia, apoi dealerului, de două ori.
        for (std::size_t seat = 0; seat < m_players.size(); ++seat) {
            dealToPlayer(seat);
        }
        dealToDealer(false);
        for (std::size_t seat = 0; seat < m_players.size(); ++seat) {
            dealToPlayer(seat);
        }
        dealToDealer(true);

        nextTurn(0);
    }

    void FakeGame::update(float dt) {
        if (!m_roundOver || m_table.isAnimating()) {
            return;
        }
        m_nextRoundTimer -= dt;
        if (m_nextRoundTimer <= 0.f) {
            startRound();
        }
    }

    int FakeGame::score(const std::vector<Card>& hand) {
        int total = 0;
        int aces = 0;
        for (const Card& card : hand) {
            if (card.rank == kMaxRank) {
                total += 11;
                ++aces;
            } else {
                total += std::min(card.rank, 10);
            }
        }
        // Asul valorează 1 în loc de 11 cât timp altfel s-ar depăși 21.
        while (total > 21 && aces > 0) {
            total -= 10;
            --aces;
        }
        return total;
    }

    bool FakeGame::isBlackjack(const std::vector<Card>& hand) {
        return hand.size() == 2 && score(hand) == 21;
    }

    FakeGame::Card FakeGame::drawCard() {
        if (m_deck.size() < kReshuffleBelow) {
            m_deck.clear();
            for (int rank = kMinRank; rank <= kMaxRank; ++rank) {
                for (int suit = 0; suit < kSuitCount; ++suit) {
                    m_deck.push_back({rank, suit});
                }
            }
            std::shuffle(m_deck.begin(), m_deck.end(), m_rng);
        }
        const Card card = m_deck.back();
        m_deck.pop_back();
        return card;
    }

    void FakeGame::dealToPlayer(std::size_t seat) {
        const Card card = drawCard();
        m_players[seat].hand.push_back(card);
        m_table.dealPlayerCard(seat, card.rank, card.suit);
        m_table.setPlayerScore(seat, score(m_players[seat].hand));
    }

    void FakeGame::dealToDealer(bool faceDown) {
        const Card card = drawCard();
        m_dealer.push_back(card);
        m_table.dealDealerCard(card.rank, card.suit, faceDown);
        // Cât timp a doua carte e ascunsă, scorul arătat e doar al primei.
        m_table.setDealerScore(m_dealerRevealed ? score(m_dealer) : score({m_dealer.front()}));
    }

    void FakeGame::hit() {
        const std::optional<std::size_t> seat = m_table.getCurrentSeat();
        if (!seat) {
            return;
        }
        dealToPlayer(*seat);
        if (score(m_players[*seat].hand) >= 21) {
            nextTurn(*seat + 1);
        }
    }

    void FakeGame::stand() {
        if (const std::optional<std::size_t> seat = m_table.getCurrentSeat()) {
            nextTurn(*seat + 1);
        }
    }

    void FakeGame::doubleDown() {
        const std::optional<std::size_t> seat = m_table.getCurrentSeat();
        if (!seat) {
            return;
        }
        Player& player = m_players[*seat];
        if (player.money < player.bet) {
            return;  // nu are destui bani să dubleze
        }
        player.money -= player.bet;
        player.bet *= 2;
        m_table.setPlayerMoney(*seat, player.money);
        m_table.setPlayerBet(*seat, player.bet);

        // După Double primește o singură carte și rândul trece mai departe.
        dealToPlayer(*seat);
        nextTurn(*seat + 1);
    }

    void FakeGame::nextTurn(std::size_t seat) {
        for (; seat < m_players.size(); ++seat) {
            if (score(m_players[seat].hand) < 21) {
                m_table.setCurrentSeat(seat);
                return;
            }
        }
        playDealer();
    }

    void FakeGame::playDealer() {
        m_table.setCurrentSeat(std::nullopt);

        const Card hidden = m_dealer.at(1);
        m_dealerRevealed = true;
        m_table.revealDealerCard(hidden.rank, hidden.suit);
        m_table.setDealerScore(score(m_dealer));

        // Dealerul trage doar dacă mai e cineva în joc (nu toți au depășit 21).
        const bool anyoneLeft = std::any_of(m_players.begin(), m_players.end(),
                                            [](const Player& p) { return score(p.hand) <= 21; });
        while (anyoneLeft && score(m_dealer) < kDealerStandsOn) {
            dealToDealer(false);
        }

        settle();
    }

    void FakeGame::settle() {
        const int dealerScore = score(m_dealer);
        const bool dealerBlackjack = isBlackjack(m_dealer);

        for (std::size_t seat = 0; seat < m_players.size(); ++seat) {
            Player& player = m_players[seat];
            const int playerScore = score(player.hand);
            const bool playerBlackjack = isBlackjack(player.hand);

            RoundResult result = RoundResult::Lose;
            if (playerScore > 21) {
                result = RoundResult::Lose;
            } else if (playerBlackjack && dealerBlackjack) {
                result = RoundResult::Push;
            } else if (playerBlackjack) {
                result = RoundResult::Blackjack;
            } else if (dealerBlackjack) {
                result = RoundResult::Lose;
            } else if (dealerScore > 21 || playerScore > dealerScore) {
                result = RoundResult::Win;
            } else if (playerScore == dealerScore) {
                result = RoundResult::Push;
            }

            // Pariul a fost deja scos din bani; aici se dă înapoi cu câștigul.
            switch (result) {
                case RoundResult::Blackjack:
                    player.money += player.bet + player.bet * 3 / 2;
                    break;
                case RoundResult::Win:
                    player.money += 2 * player.bet;
                    break;
                case RoundResult::Push:
                    player.money += player.bet;
                    break;
                case RoundResult::Lose:
                    break;
            }
            m_table.setPlayerMoney(seat, player.money);
            m_table.showResult(seat, result);
        }

        m_roundOver = true;
        m_nextRoundTimer = kNextRoundDelay;
    }
}  // namespace Frontend
