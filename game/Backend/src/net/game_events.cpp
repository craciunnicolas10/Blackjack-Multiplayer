#include <Backend/net/game_events.h>

#include <cstdint>
#include <utility>

namespace Backend {
    namespace {
        // Cartea cu fata in jos nu pleaca din calculatorul gazdei, ca sa nu poata fi vazuta;
        // in locul ei trimitem una oarecare, pe care masa oricum n-o arata.
        constexpr Card kHiddenCard{ERank::Two, ESuit::Clubs};

        bool ReadIndex(sf::Packet& packet, int& index) {
            std::uint8_t value = 0;
            if (!(packet >> value) || value >= kMaxPlayers) {
                return false;
            }
            index = value;
            return true;
        }

        // Citeste datele evenimentului si intoarce functia care il anunta.
        std::optional<std::function<void(IGameListener&)>> ReadNotify(sf::Packet& packet,
                                                                      EEventType type) {
            using Listener = IGameListener;
            int index = 0;
            Card card{};
            switch (type) {
                case EEventType::StateChanged: {
                    EGameState state{};
                    if (!ReadEnum(packet, state, kLastGameState)) {
                        return std::nullopt;
                    }
                    return [state](Listener& l) { l.OnGameStateChanged(state); };
                }
                case EEventType::PlayerAdded:
                    if (!ReadIndex(packet, index)) {
                        return std::nullopt;
                    }
                    return [index](Listener& l) { l.OnPlayerAdded(index); };
                case EEventType::BetPlaced: {
                    std::int32_t amount = 0;
                    if (!ReadIndex(packet, index) || !(packet >> amount)) {
                        return std::nullopt;
                    }
                    return [index, amount](Listener& l) { l.OnBetPlaced(index, amount); };
                }
                case EEventType::PlayerCardDealt:
                    if (!ReadIndex(packet, index) || !ReadCard(packet, card)) {
                        return std::nullopt;
                    }
                    return [index, card](Listener& l) { l.OnPlayerCardDealt(index, card); };
                case EEventType::DealerCardDealt: {
                    bool faceDown = false;
                    if (!ReadCard(packet, card) || !(packet >> faceDown)) {
                        return std::nullopt;
                    }
                    return [card, faceDown](Listener& l) { l.OnDealerCardDealt(card, faceDown); };
                }
                case EEventType::DealerCardRevealed:
                    if (!ReadCard(packet, card)) {
                        return std::nullopt;
                    }
                    return [card](Listener& l) { l.OnDealerCardRevealed(card); };
                case EEventType::TurnChanged:
                    if (!ReadIndex(packet, index)) {
                        return std::nullopt;
                    }
                    return [index](Listener& l) { l.OnTurnChanged(index); };
                case EEventType::RoundEnded: {
                    ERoundResult result{};
                    if (!ReadIndex(packet, index) || !ReadEnum(packet, result, kLastRoundResult)) {
                        return std::nullopt;
                    }
                    return [index, result](Listener& l) { l.OnRoundEnded(index, result); };
                }
            }
            return std::nullopt;
        }
    }  // namespace

    EventEncoder::EventEncoder(const IGame& game, std::function<void(sf::Packet&)> send)
        : m_game(game), m_send(std::move(send)) {}

    sf::Packet EventEncoder::Begin(EEventType type) {
        sf::Packet packet = MakeMessage(EMessageType::GameEvent);
        packet << static_cast<std::uint8_t>(type);
        return packet;
    }

    void EventEncoder::Finish(sf::Packet& packet) {
        WriteSnapshot(packet, TakeSnapshot(m_game));
        m_send(packet);
    }

    void EventEncoder::OnGameStateChanged(EGameState newState) {
        sf::Packet packet = Begin(EEventType::StateChanged);
        packet << static_cast<std::uint8_t>(newState);
        Finish(packet);
    }

    void EventEncoder::OnPlayerAdded(int playerIndex) {
        sf::Packet packet = Begin(EEventType::PlayerAdded);
        packet << static_cast<std::uint8_t>(playerIndex);
        Finish(packet);
    }

    void EventEncoder::OnBetPlaced(int playerIndex, int amount) {
        sf::Packet packet = Begin(EEventType::BetPlaced);
        packet << static_cast<std::uint8_t>(playerIndex) << static_cast<std::int32_t>(amount);
        Finish(packet);
    }

    void EventEncoder::OnPlayerCardDealt(int playerIndex, Card card) {
        sf::Packet packet = Begin(EEventType::PlayerCardDealt);
        packet << static_cast<std::uint8_t>(playerIndex);
        WriteCard(packet, card);
        Finish(packet);
    }

    void EventEncoder::OnDealerCardDealt(Card card, bool isFaceDown) {
        sf::Packet packet = Begin(EEventType::DealerCardDealt);
        WriteCard(packet, isFaceDown ? kHiddenCard : card);
        packet << isFaceDown;
        Finish(packet);
    }

    void EventEncoder::OnDealerCardRevealed(Card card) {
        sf::Packet packet = Begin(EEventType::DealerCardRevealed);
        WriteCard(packet, card);
        Finish(packet);
    }

    void EventEncoder::OnTurnChanged(int playerIndex) {
        sf::Packet packet = Begin(EEventType::TurnChanged);
        packet << static_cast<std::uint8_t>(playerIndex);
        Finish(packet);
    }

    void EventEncoder::OnRoundEnded(int playerIndex, ERoundResult result) {
        sf::Packet packet = Begin(EEventType::RoundEnded);
        packet << static_cast<std::uint8_t>(playerIndex) << static_cast<std::uint8_t>(result);
        Finish(packet);
    }

    std::optional<DecodedEvent> DecodeEvent(sf::Packet& packet) {
        EEventType type{};
        if (!ReadEnum(packet, type, kLastEventType)) {
            return std::nullopt;
        }
        auto notify = ReadNotify(packet, type);
        if (!notify) {
            return std::nullopt;
        }
        DecodedEvent event{std::move(*notify), {}};
        if (!ReadSnapshot(packet, event.snapshot)) {
            return std::nullopt;
        }
        return event;
    }
}  // namespace Backend
