#pragma once

#include <vector>

#include <Backend/Card.h>
#include <Backend/game/player.h>

namespace Backend {
    // Dealerul e un jucator fara bani, cu a doua carte cu fata in jos pana o intoarce.
    // Cat e ascunsa, GetHand() si GetScore() o lasa deoparte.
    class Dealer : public Player {
       public:
        Dealer();

        std::vector<Card> GetHand() const override;
        int GetScore() const override;

        void AddHoleCard(Card card);
        bool HasHiddenCard() const;
        // Intoarce cartea ascunsa si o da inapoi.
        Card RevealHoleCard();

       private:
        bool m_holeCardHidden = false;
    };
}  // namespace Backend
