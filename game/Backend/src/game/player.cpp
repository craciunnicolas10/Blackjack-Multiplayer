#include <Backend/game/player.h>

#include <utility>

#include <Backend/game/hand_score.h>

namespace Backend {
    Player::Player(std::string name, int balance) : m_name(std::move(name)), m_balance(balance) {}

    std::string Player::GetName() const { return m_name; }

    std::vector<Card> Player::GetHand() const { return m_hand; }

    int Player::GetScore() const { return ScoreHand(m_hand); }

    int Player::GetBalance() const { return m_balance; }

    int Player::GetBet() const { return m_bet; }

    const std::vector<Card>& Player::GetFullHand() const { return m_hand; }

    bool Player::HasBlackjack() const { return IsBlackjack(m_hand); }

    bool Player::IsBusted() const { return ScoreHand(m_hand) > kBlackjackScore; }

    bool Player::IsInRound() const { return m_bet > 0; }

    void Player::AddCard(Card card) { m_hand.push_back(card); }

    void Player::AddToBet(int amount) {
        m_balance -= amount;
        m_bet += amount;
    }

    void Player::Receive(int amount) { m_balance += amount; }

    void Player::ResetRound() {
        m_hand.clear();
        m_bet = 0;
    }
}  // namespace Backend
