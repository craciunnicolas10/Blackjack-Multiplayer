#pragma once

namespace Backend {
    enum class EGameState {
        WaitingForPlayers,
        Betting,
        PlayersTurn,
        DealerTurn,
        RoundOver
    };

    enum class ERoundResult {
        Win,
        Lose,
        Push,
        Blackjack
    };
}  // namespace Backend
