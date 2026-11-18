#pragma once

#include <string>
#include <vector>

#include <Backend/Card.h>

namespace Backend {
    // Vedere doar pentru citire asupra unui jucator (sau a dealerului).
    class IPlayer {
       public:
        virtual ~IPlayer() = default;

        virtual std::string GetName() const = 0;
        virtual std::vector<Card> GetHand() const = 0;
        virtual int GetScore() const = 0;
        virtual int GetBalance() const = 0;
        virtual int GetBet() const = 0;
    };
}  // namespace Backend
