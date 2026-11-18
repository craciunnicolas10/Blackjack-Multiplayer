#include <game/round_result.h>

namespace Frontend {
    const char* roundResultLabel(Backend::ERoundResult result) {
        switch (result) {
            case Backend::ERoundResult::Win:
                return "Câștig";
            case Backend::ERoundResult::Lose:
                return "Pierdere";
            case Backend::ERoundResult::Push:
                return "Egal";
            case Backend::ERoundResult::Blackjack:
                return "Blackjack!";
        }
        return "";
    }

    sf::Color roundResultColor(Backend::ERoundResult result) {
        switch (result) {
            case Backend::ERoundResult::Win:
                return sf::Color(40, 140, 60);
            case Backend::ERoundResult::Lose:
                return sf::Color(170, 40, 40);
            case Backend::ERoundResult::Push:
                return sf::Color(100, 100, 100);
            case Backend::ERoundResult::Blackjack:
                return sf::Color(190, 140, 20);
        }
        return sf::Color::Black;
    }
}  // namespace Frontend
