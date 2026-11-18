#include <game/game_controller.h>

#include <utility>

#include <screens/table_screen.h>
#include <ui/card_textures.h>

namespace Frontend {
    namespace {
        // Masa primește cărțile ca numere; numerotarea e aceeași ca în Backend, deci e de
        // ajuns un static_cast. Dacă cineva schimbă ordinea în Backend, compilarea pică aici.
        static_assert(static_cast<int>(Backend::ERank::Two) == kMinRank);
        static_assert(static_cast<int>(Backend::ERank::Ace) == kMaxRank);
        static_assert(static_cast<int>(Backend::ESuit::Clubs) == kSuitClubs);
        static_assert(static_cast<int>(Backend::ESuit::Diamonds) == kSuitDiamonds);
        static_assert(static_cast<int>(Backend::ESuit::Hearts) == kSuitHearts);
        static_assert(static_cast<int>(Backend::ESuit::Spades) == kSuitSpades);

        int rankOf(Backend::Card card) { return static_cast<int>(card.rank); }
        int suitOf(Backend::Card card) { return static_cast<int>(card.suit); }
    }  // namespace

    std::shared_ptr<GameController> GameController::create(TableScreen& table,
                                                           std::shared_ptr<Backend::IGame> game,
                                                           std::vector<bool> localSeats) {
        // Constructorul e privat, deci nu se poate folosi std::make_shared.
        std::shared_ptr<GameController> controller(
            new GameController(table, std::move(game), std::move(localSeats)));
        controller->m_game->AddListener(controller);
        controller->connectTable();
        return controller;
    }

    GameController::GameController(TableScreen& table, std::shared_ptr<Backend::IGame> game,
                                   std::vector<bool> localSeats)
        : m_table(table),
          m_game(std::move(game)),
          m_localSeats(std::move(localSeats)),
          m_lastBets(table.getSeatCount(), kSuggestedFirstBet),
          m_balancesAtRoundStart(table.getSeatCount(), 0) {}

    void GameController::showGameOver() { m_table.showGameOver(); }

    bool GameController::isLocal(std::size_t seat) const {
        return seat < m_localSeats.size() && m_localSeats[seat];
    }

    void GameController::connectTable() {
        // Masa păstrează funcțiile astea, iar masa e cea care ține GameController-ul în viață,
        // deci `this` e valid cât timp pot fi apelate.
        m_table.setOnBet([this](int amount) { placeBet(amount); });
        m_table.setOnHit([this] { m_game->Hit(); });
        m_table.setOnStand([this] { m_game->Stand(); });
        m_table.setOnDoubleDown([this] { m_game->DoubleDown(); });
        m_table.setOnNextRound([this] { requestNextRound(); });
    }

    // --- Ce face jucătorul pe masă → jocul ---

    void GameController::placeBet(int amount) {
        if (!m_bettingSeat) {
            return;
        }
        const std::size_t seat = *m_bettingSeat;
        // Dacă jocul acceptă pariul, anunță prin OnBetPlaced și de acolo trecem mai departe.
        if (m_game->PlaceBet(static_cast<int>(seat), amount)) {
            m_lastBets[seat] = amount;
        }
    }

    void GameController::requestNextRound() {
        // Jocul refuză runda nouă când nimeni nu mai are bani nici de pariul minim.
        if (!m_game->StartRound()) {
            m_table.showGameOver();
        }
    }

    // --- Ce anunță jocul → masa ---

    void GameController::OnGameStateChanged(Backend::EGameState newState) {
        switch (newState) {
            case Backend::EGameState::Betting:
                onBettingStarted();
                break;
            case Backend::EGameState::DealerTurn:
                m_table.setCurrentSeat(std::nullopt);
                break;
            case Backend::EGameState::RoundOver:
                onRoundOver();
                break;
            case Backend::EGameState::WaitingForPlayers:
            case Backend::EGameState::PlayersTurn:
                break;
        }
    }

    void GameController::OnPlayerAdded(int playerIndex) {
        const auto seat = static_cast<std::size_t>(playerIndex);
        if (seat < seatCount()) {
            m_table.setPlayerMoney(seat, balanceOf(seat));
        }
    }

    void GameController::OnBetPlaced(int playerIndex, int amount) {
        const auto seat = static_cast<std::size_t>(playerIndex);
        m_table.setPlayerBet(seat, amount);
        m_table.setPlayerMoney(seat, balanceOf(seat));
        // Tot prin OnBetPlaced vine și pariul dublat (Double); atunci nu se mai pariază.
        if (m_game->GetState() == Backend::EGameState::Betting) {
            askNextBet();
        }
    }

    void GameController::OnPlayerCardDealt(int playerIndex, Backend::Card card) {
        const auto seat = static_cast<std::size_t>(playerIndex);
        m_table.dealPlayerCard(seat, rankOf(card), suitOf(card));
        m_table.setPlayerScore(seat, m_game->GetPlayer(playerIndex)->GetScore());
    }

    void GameController::OnDealerCardDealt(Backend::Card card, bool isFaceDown) {
        m_table.dealDealerCard(rankOf(card), suitOf(card), isFaceDown);
        // Cât timp o carte e cu fața în jos, jocul dă doar scorul cărților vizibile.
        m_table.setDealerScore(m_game->GetDealer()->GetScore());
    }

    void GameController::OnDealerCardRevealed(Backend::Card card) {
        m_table.revealDealerCard(rankOf(card), suitOf(card));
        m_table.setDealerScore(m_game->GetDealer()->GetScore());
    }

    void GameController::OnTurnChanged(int playerIndex) {
        const auto seat = static_cast<std::size_t>(playerIndex);
        m_table.setCurrentSeat(seat, isLocal(seat));
    }

    void GameController::OnRoundEnded(int playerIndex, Backend::ERoundResult result) {
        const auto seat = static_cast<std::size_t>(playerIndex);
        m_table.showResult(seat, result);
        m_table.setPlayerMoney(seat, balanceOf(seat));
    }

    void GameController::onBettingStarted() {
        m_table.clearRound();
        for (std::size_t seat = 0; seat < seatCount(); ++seat) {
            m_balancesAtRoundStart[seat] = balanceOf(seat);
            m_table.setPlayerBet(seat, 0);
            m_table.setPlayerMoney(seat, balanceOf(seat));
        }
        askNextBet();
    }

    void GameController::askNextBet() {
        for (std::size_t seat = 0; seat < seatCount(); ++seat) {
            const int balance = balanceOf(seat);
            const bool alreadyBet = m_game->GetPlayer(static_cast<int>(seat))->GetBet() > 0;
            // Jocul sare peste cine nu are nici pariul minim (vezi IGame.h), deci nici masa
            // nu îl întreabă.
            if (!alreadyBet && balance >= Backend::kMinBet) {
                m_bettingSeat = seat;
                if (isLocal(seat)) {
                    m_table.startBetting(seat, balance, Backend::kMinBet, Backend::kMaxBet,
                                         m_lastBets[seat]);
                } else {
                    m_table.waitForBet(seat);
                }
                return;
            }
        }
        m_bettingSeat.reset();
        m_table.endBetting();
    }

    void GameController::onRoundOver() {
        std::vector<int> netWinnings(seatCount(), 0);
        for (std::size_t seat = 0; seat < seatCount(); ++seat) {
            netWinnings[seat] = balanceOf(seat) - m_balancesAtRoundStart[seat];
        }
        m_table.showRoundSummary(std::move(netWinnings));
    }

    // --- Ajutătoare ---

    int GameController::balanceOf(std::size_t seat) const {
        const Backend::IPlayer* player = m_game->GetPlayer(static_cast<int>(seat));
        return player ? player->GetBalance() : 0;
    }

    std::size_t GameController::seatCount() const {
        // Doar locurile care au și un jucător în joc (normal sunt aceleași).
        const auto players = static_cast<std::size_t>(m_game->GetPlayerCount());
        return players < m_table.getSeatCount() ? players : m_table.getSeatCount();
    }
}  // namespace Frontend
