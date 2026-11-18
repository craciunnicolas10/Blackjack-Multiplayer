#pragma once

#include <string>
#include <vector>

#include <Backend/Card.h>
#include <Backend/IPlayer.h>

namespace Backend {
    class Player : public IPlayer {
       public:
        Player(std::string name, int balance);

        std::string GetName() const override;
        std::vector<Card> GetHand() const override;
        int GetScore() const override;
        int GetBalance() const override;
        int GetBet() const override;
        bool IsBusted() const override;

        // Toate cartile, inclusiv cele ascunse (pentru reguli, nu pentru afisare).
        const std::vector<Card>& GetFullHand() const;
        bool HasBlackjack() const;
        // A pariat in runda curenta, deci joaca.
        bool IsInRound() const;

        void AddCard(Card card);
        // Muta suma din bani in pariu (se adauga la pariul existent, pentru Double).
        void AddToBet(int amount);
        void Receive(int amount);
        // Strange cartile si pariul, pentru o runda noua.
        void ResetRound();

       private:
        std::string m_name;
        std::vector<Card> m_hand;
        int m_balance;
        int m_bet = 0;
    };
}  // namespace Backend
