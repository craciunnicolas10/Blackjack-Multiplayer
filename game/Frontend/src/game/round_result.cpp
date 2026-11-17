#include <game/round_result.h>

namespace Frontend {
    const char* roundResultLabel(RoundResult result) {
        switch (result) {
            case RoundResult::Win:
                return "Câștig";
            case RoundResult::Lose:
                return "Pierdere";
            case RoundResult::Push:
                return "Egal";
            case RoundResult::Blackjack:
                return "Blackjack!";
        }
        return "";
    }

    sf::Color roundResultColor(RoundResult result) {
        switch (result) {
            case RoundResult::Win:
                return sf::Color(40, 140, 60);
            case RoundResult::Lose:
                return sf::Color(170, 40, 40);
            case RoundResult::Push:
                return sf::Color(100, 100, 100);
            case RoundResult::Blackjack:
                return sf::Color(190, 140, 20);
        }
        return sf::Color::Black;
    }
}  // namespace Frontend
