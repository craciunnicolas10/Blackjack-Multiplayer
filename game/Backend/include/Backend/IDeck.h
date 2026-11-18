#pragma once

#include <Backend/Card.h>

namespace Backend {
    // Pachetul din care jocul trage cartile. Jocul real foloseste un pachet amestecat aleator;
    // testele pot da un pachet cu cartile intr-o ordine cunoscuta (vezi IGame::Produce).
    class IDeck {
       public:
        virtual ~IDeck() = default;

        // Apelata la inceputul fiecarei runde, inainte de impartire.
        virtual void Shuffle() = 0;
        virtual Card Draw() = 0;
    };
}  // namespace Backend
