#include <net/game_events.h>

#include <cstdint>
#include <utility>

namespace Frontend::Net {
    namespace {
        // Cartea cu fața în jos nu pleacă din calculatorul gazdei, ca să nu poată fi văzută;
        // în locul ei trimitem una oarecare, pe care masa oricum n-o arată.
        constexpr Backend::Card kHiddenCard{Backend::ERank::Two, Backend::ESuit::Clubs};

        bool readIndex(sf::Packet& packet, int& index) {
            std::uint8_t value = 0;
            if (!(packet >> value) || value >= Backend::kMaxPlayers) {
                return false;
            }
            index = value;
            return true;
        }

        // Citește datele evenimentului și întoarce funcția care îl anunță.
        std::optional<std::function<void(Backend::IGameListener&)>> readNotify(
            sf::Packet& packet, EventType type) {
            using Listener = Backend::IGameListener;
            int index = 0;
            Backend::Card card{};
            switch (type) {
                case EventType::StateChanged: {
                    Backend::EGameState state{};
                    if (!readEnum(packet, state, kLastGameState)) {
                        return std::nullopt;
                    }
                    return [state](Listener& l) { l.OnGameStateChanged(state); };
                }
                case EventType::PlayerAdded:
                    if (!readIndex(packet, index)) {
                        return std::nullopt;
                    }
                    return [index](Listener& l) { l.OnPlayerAdded(index); };
                case EventType::BetPlaced: {
                    std::int32_t amount = 0;
                    if (!readIndex(packet, index) || !(packet >> amount)) {
                        return std::nullopt;
                    }
                    return [index, amount](Listener& l) { l.OnBetPlaced(index, amount); };
                }
                case EventType::PlayerCardDealt:
                    if (!readIndex(packet, index) || !readCard(packet, card)) {
                        return std::nullopt;
                    }
                    return [index, card](Listener& l) { l.OnPlayerCardDealt(index, card); };
                case EventType::DealerCardDealt: {
                    bool faceDown = false;
                    if (!readCard(packet, card) || !(packet >> faceDown)) {
                        return std::nullopt;
                    }
                    return [card, faceDown](Listener& l) { l.OnDealerCardDealt(card, faceDown); };
                }
                case EventType::DealerCardRevealed:
                    if (!readCard(packet, card)) {
                        return std::nullopt;
                    }
                    return [card](Listener& l) { l.OnDealerCardRevealed(card); };
                case EventType::TurnChanged:
                    if (!readIndex(packet, index)) {
                        return std::nullopt;
                    }
                    return [index](Listener& l) { l.OnTurnChanged(index); };
                case EventType::RoundEnded: {
                    Backend::ERoundResult result{};
                    if (!readIndex(packet, index) || !readEnum(packet, result, kLastRoundResult)) {
                        return std::nullopt;
                    }
                    return [index, result](Listener& l) { l.OnRoundEnded(index, result); };
                }
            }
            return std::nullopt;
        }
    }  // namespace

    EventEncoder::EventEncoder(const Backend::IGame& game, std::function<void(sf::Packet&)> send)
        : m_game(game), m_send(std::move(send)) {}

    sf::Packet EventEncoder::begin(EventType type) {
        sf::Packet packet = makeMessage(MessageType::GameEvent);
        packet << static_cast<std::uint8_t>(type);
        return packet;
    }

    void EventEncoder::finish(sf::Packet& packet) {
        writeSnapshot(packet, takeSnapshot(m_game));
        m_send(packet);
    }

    void EventEncoder::OnGameStateChanged(Backend::EGameState newState) {
        sf::Packet packet = begin(EventType::StateChanged);
        packet << static_cast<std::uint8_t>(newState);
        finish(packet);
    }

    void EventEncoder::OnPlayerAdded(int playerIndex) {
        sf::Packet packet = begin(EventType::PlayerAdded);
        packet << static_cast<std::uint8_t>(playerIndex);
        finish(packet);
    }

    void EventEncoder::OnBetPlaced(int playerIndex, int amount) {
        sf::Packet packet = begin(EventType::BetPlaced);
        packet << static_cast<std::uint8_t>(playerIndex) << static_cast<std::int32_t>(amount);
        finish(packet);
    }

    void EventEncoder::OnPlayerCardDealt(int playerIndex, Backend::Card card) {
        sf::Packet packet = begin(EventType::PlayerCardDealt);
        packet << static_cast<std::uint8_t>(playerIndex);
        writeCard(packet, card);
        finish(packet);
    }

    void EventEncoder::OnDealerCardDealt(Backend::Card card, bool isFaceDown) {
        sf::Packet packet = begin(EventType::DealerCardDealt);
        writeCard(packet, isFaceDown ? kHiddenCard : card);
        packet << isFaceDown;
        finish(packet);
    }

    void EventEncoder::OnDealerCardRevealed(Backend::Card card) {
        sf::Packet packet = begin(EventType::DealerCardRevealed);
        writeCard(packet, card);
        finish(packet);
    }

    void EventEncoder::OnTurnChanged(int playerIndex) {
        sf::Packet packet = begin(EventType::TurnChanged);
        packet << static_cast<std::uint8_t>(playerIndex);
        finish(packet);
    }

    void EventEncoder::OnRoundEnded(int playerIndex, Backend::ERoundResult result) {
        sf::Packet packet = begin(EventType::RoundEnded);
        packet << static_cast<std::uint8_t>(playerIndex) << static_cast<std::uint8_t>(result);
        finish(packet);
    }

    std::optional<DecodedEvent> decodeEvent(sf::Packet& packet) {
        EventType type{};
        if (!readEnum(packet, type, kLastEventType)) {
            return std::nullopt;
        }
        auto notify = readNotify(packet, type);
        if (!notify) {
            return std::nullopt;
        }
        DecodedEvent event{std::move(*notify), {}};
        if (!readSnapshot(packet, event.snapshot)) {
            return std::nullopt;
        }
        return event;
    }
}  // namespace Frontend::Net
