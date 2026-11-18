#include <Backend/IGame.h>

#include <utility>

#include <Backend/game/deck.h>
#include <Backend/game/game.h>

namespace Backend {
    std::shared_ptr<IGame> IGame::Produce() { return Produce(std::make_shared<Deck>()); }

    std::shared_ptr<IGame> IGame::Produce(std::shared_ptr<IDeck> deck) {
        if (!deck) {
            return nullptr;
        }
        return std::make_shared<Game>(std::move(deck));
    }
}  // namespace Backend
