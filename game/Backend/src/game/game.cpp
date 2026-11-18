#include <Backend/game/game.h>

#include <algorithm>
#include <utility>

#include <Backend/game/hand_score.h>

namespace Backend {
    namespace {
        // Blackjack plateste 3:2, castigul 1:1 (sumele includ pariul returnat).
        constexpr int kBlackjackPayoutNumerator = 5;
        constexpr int kBlackjackPayoutDenominator = 2;
        constexpr int kWinPayoutMultiplier = 2;
        constexpr int kInitialCardsPerHand = 2;
    }  // namespace

    Game::Game(std::shared_ptr<IDeck> deck) : m_deck(std::move(deck)) {}

    // --- Observer ---

    void Game::AddListener(std::weak_ptr<IGameListener> listener) {
        const auto added = listener.lock();
        if (!added) {
            return;
        }
        std::erase_if(m_listeners, [](const auto& existing) { return existing.expired(); });
        const bool alreadyAdded =
            std::ranges::any_of(m_listeners, [&](const auto& existing) {
                return existing.lock() == added;
            });
        if (!alreadyAdded) {
            m_listeners.push_back(std::move(listener));
        }
    }

    void Game::RemoveListener(IGameListener* listener) {
        std::erase_if(m_listeners, [&](const auto& existing) {
            const auto locked = existing.lock();
            return !locked || locked.get() == listener;
        });
    }

    void Game::SetState(EGameState state) {
        m_state = state;
        Notify([&](IGameListener& listener) { listener.OnGameStateChanged(state); });
    }

    // --- Actiuni ---

    bool Game::AddPlayer(const std::string& name) {
        if (m_state != EGameState::WaitingForPlayers || GetPlayerCount() >= kMaxPlayers ||
            name.empty() || HasNameTaken(name)) {
            return false;
        }
        m_players.push_back(std::make_unique<Player>(name, kStartingBalance));
        const int playerIndex = GetPlayerCount() - 1;
        Notify([&](IGameListener& listener) { listener.OnPlayerAdded(playerIndex); });
        return true;
    }

    bool Game::StartRound() {
        const bool canStart =
            (m_state == EGameState::WaitingForPlayers || m_state == EGameState::RoundOver) &&
            std::ranges::any_of(m_players, [](const auto& player) {
                return CanAffordMinBet(*player);
            });
        if (!canStart) {
            return false;
        }
        for (auto& player : m_players) {
            player->ResetRound();
        }
        m_dealer = Dealer();
        m_currentPlayer = kNoPlayer;
        SetState(EGameState::Betting);
        return true;
    }

    bool Game::PlaceBet(int playerIndex, int amount) {
        if (m_state != EGameState::Betting || !IsValidPlayerIndex(playerIndex)) {
            return false;
        }
        Player& player = *m_players[playerIndex];
        if (player.IsInRound() || amount < kMinBet || amount > kMaxBet ||
            amount > player.GetBalance()) {
            return false;
        }
        player.AddToBet(amount);
        Notify([&](IGameListener& listener) { listener.OnBetPlaced(playerIndex, amount); });
        if (AllBetsPlaced()) {
            DealInitialCards();
        }
        return true;
    }

    bool Game::Hit() {
        if (m_state != EGameState::PlayersTurn) {
            return false;
        }
        DealToPlayer(m_currentPlayer);
        if (CurrentPlayer().GetScore() >= kBlackjackScore) {
            GiveTurnFrom(m_currentPlayer + 1);
        }
        return true;
    }

    bool Game::Stand() {
        if (m_state != EGameState::PlayersTurn) {
            return false;
        }
        GiveTurnFrom(m_currentPlayer + 1);
        return true;
    }

    bool Game::DoubleDown() {
        if (m_state != EGameState::PlayersTurn) {
            return false;
        }
        Player& player = CurrentPlayer();
        const int extraBet = player.GetBet();
        if (player.GetFullHand().size() != kInitialCardsPerHand ||
            player.GetBalance() < extraBet) {
            return false;
        }
        player.AddToBet(extraBet);
        const int playerIndex = m_currentPlayer;
        const int totalBet = player.GetBet();
        Notify([&](IGameListener& listener) { listener.OnBetPlaced(playerIndex, totalBet); });
        DealToPlayer(playerIndex);
        GiveTurnFrom(playerIndex + 1);
        return true;
    }

    // --- Citire ---

    EGameState Game::GetState() const { return m_state; }

    int Game::GetPlayerCount() const { return static_cast<int>(m_players.size()); }

    int Game::GetCurrentPlayerIndex() const { return m_currentPlayer; }

    const IPlayer* Game::GetPlayer(int playerIndex) const {
        return IsValidPlayerIndex(playerIndex) ? m_players[playerIndex].get() : nullptr;
    }

    const IPlayer* Game::GetDealer() const { return &m_dealer; }

    // --- Ajutatoare ---

    bool Game::IsValidPlayerIndex(int playerIndex) const {
        return playerIndex >= 0 && playerIndex < GetPlayerCount();
    }

    bool Game::HasNameTaken(const std::string& name) const {
        return std::ranges::any_of(
            m_players, [&](const auto& player) { return player->GetName() == name; });
    }

    bool Game::CanAffordMinBet(const Player& player) { return player.GetBalance() >= kMinBet; }

    bool Game::AllBetsPlaced() const {
        // Cine nu are bani de pariul minim e sarit in runda asta.
        return std::ranges::all_of(m_players, [](const auto& player) {
            return player->IsInRound() || !CanAffordMinBet(*player);
        });
    }

    Player& Game::CurrentPlayer() { return *m_players[m_currentPlayer]; }

    // --- Desfasurarea rundei ---

    void Game::DealInitialCards() {
        m_deck->Shuffle();
        for (int pass = 0; pass < kInitialCardsPerHand; ++pass) {
            for (int playerIndex = 0; playerIndex < GetPlayerCount(); ++playerIndex) {
                if (m_players[playerIndex]->IsInRound()) {
                    DealToPlayer(playerIndex);
                }
            }
            const bool isHoleCard = pass == kInitialCardsPerHand - 1;
            DealToDealer(isHoleCard);
        }
        SetState(EGameState::PlayersTurn);
        GiveTurnFrom(0);
    }

    void Game::DealToPlayer(int playerIndex) {
        const Card card = m_deck->Draw();
        m_players[playerIndex]->AddCard(card);
        Notify([&](IGameListener& listener) { listener.OnPlayerCardDealt(playerIndex, card); });
    }

    void Game::DealToDealer(bool faceDown) {
        const Card card = m_deck->Draw();
        if (faceDown) {
            m_dealer.AddHoleCard(card);
        } else {
            m_dealer.AddCard(card);
        }
        Notify([&](IGameListener& listener) { listener.OnDealerCardDealt(card, faceDown); });
    }

    void Game::GiveTurnFrom(int fromIndex) {
        for (int playerIndex = fromIndex; playerIndex < GetPlayerCount(); ++playerIndex) {
            const Player& player = *m_players[playerIndex];
            if (player.IsInRound() && player.GetScore() < kBlackjackScore) {
                m_currentPlayer = playerIndex;
                Notify([&](IGameListener& listener) { listener.OnTurnChanged(playerIndex); });
                return;
            }
        }
        m_currentPlayer = kNoPlayer;
        PlayDealer();
    }

    void Game::PlayDealer() {
        SetState(EGameState::DealerTurn);
        const Card holeCard = m_dealer.RevealHoleCard();
        Notify([&](IGameListener& listener) { listener.OnDealerCardRevealed(holeCard); });
        if (DealerNeedsToDraw()) {
            while (m_dealer.GetScore() < kDealerStandsOn) {
                DealToDealer(false);
            }
        }
        SettleRound();
    }

    bool Game::DealerNeedsToDraw() const {
        // Daca toti jucatorii au trecut de 21 sau au blackjack, rezultatul e deja hotarat.
        return std::ranges::any_of(m_players, [](const auto& player) {
            return player->IsInRound() && !player->IsBusted() && !player->HasBlackjack();
        });
    }

    void Game::SettleRound() {
        for (int playerIndex = 0; playerIndex < GetPlayerCount(); ++playerIndex) {
            Player& player = *m_players[playerIndex];
            if (!player.IsInRound()) {
                continue;
            }
            const ERoundResult result = ResultFor(player);
            player.Receive(PayoutFor(result, player.GetBet()));
            Notify([&](IGameListener& listener) { listener.OnRoundEnded(playerIndex, result); });
        }
        SetState(EGameState::RoundOver);
    }

    ERoundResult Game::ResultFor(const Player& player) const {
        if (player.IsBusted()) {
            return ERoundResult::Lose;
        }
        if (player.HasBlackjack()) {
            return m_dealer.HasBlackjack() ? ERoundResult::Push : ERoundResult::Blackjack;
        }
        if (m_dealer.HasBlackjack()) {
            return ERoundResult::Lose;
        }
        if (m_dealer.IsBusted()) {
            return ERoundResult::Win;
        }
        const int playerScore = player.GetScore();
        const int dealerScore = m_dealer.GetScore();
        if (playerScore > dealerScore) {
            return ERoundResult::Win;
        }
        return playerScore == dealerScore ? ERoundResult::Push : ERoundResult::Lose;
    }

    int Game::PayoutFor(ERoundResult result, int bet) {
        switch (result) {
            case ERoundResult::Blackjack:
                return bet * kBlackjackPayoutNumerator / kBlackjackPayoutDenominator;
            case ERoundResult::Win:
                return bet * kWinPayoutMultiplier;
            case ERoundResult::Push:
                return bet;
            case ERoundResult::Lose:
                return 0;
        }
        return 0;
    }
}  // namespace Backend
