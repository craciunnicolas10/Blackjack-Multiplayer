#pragma once

#include <cstdint>
#include <optional>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <Backend/Card.h>
#include <Backend/GameEnums.h>

// Ce își trimit calculatoarele în jocul în rețea. Gazda ține jocul real (Backend); ceilalți
// îi trimit doar ce a ales jucătorul lor și primesc înapoi tot ce s-a întâmplat la masă.
// Fiecare mesaj e un sf::Packet care începe cu MessageType.
namespace Frontend::Net {
    // Gazda încearcă întâi kDefaultPort; dacă e ocupat (ex. de jocul de adineauri, pe care
    // sistemul îl mai ține câteva zeci de secunde), îl ia pe următorul liber din kPortCount.
    inline constexpr unsigned short kDefaultPort = 53000;
    inline constexpr unsigned short kPortCount = 10;
    // Se schimbă când se schimbă mesajele, ca două versiuni diferite să nu joace împreună.
    inline constexpr std::uint32_t kProtocolVersion = 1;

    enum class MessageType : std::uint8_t {
        // --- jucător → gazdă ---
        Join,        // versiune, nume
        PlaceBet,    // suma
        Hit,
        Stand,
        DoubleDown,
        NextRound,
        // --- gazdă → jucător ---
        Rejected,      // motiv (nume luat, masă plină, versiune diferită)
        PlayerList,    // numele din lobby, în ordinea locurilor
        GameStarting,  // locul tău, numele tuturor
        GameEvent,     // un anunț al jocului (EventType + date) și starea de după el
        GameOver,      // nimeni nu mai are bani de pariu
        PlayerLeft,    // numele celui care a ieșit; jocul se oprește
    };

    // Anunțurile din Backend::IGameListener, unul la unul.
    enum class EventType : std::uint8_t {
        StateChanged,
        PlayerAdded,
        BetPlaced,
        PlayerCardDealt,
        DealerCardDealt,
        DealerCardRevealed,
        TurnChanged,
        RoundEnded,
    };

    // Limite pentru ce primim din rețea, ca un mesaj stricat să nu ceară memorie la nesfârșit.
    inline constexpr std::uint8_t kMaxHandSize = 22;
    inline constexpr std::uint8_t kMaxNameLength = 24;

    sf::Packet makeMessage(MessageType type);
    // nullopt dacă pachetul e gol sau tipul e necunoscut.
    std::optional<MessageType> readMessageType(sf::Packet& packet);

    // Pe socket-uri non-blocking un pachet poate pleca pe bucăți; asta îl trimite pe tot.
    // False dacă legătura s-a rupt sau celălalt nu mai primește nimic de câteva secunde.
    bool sendAll(sf::TcpSocket& socket, sf::Packet& packet);

    // Ultima valoare din fiecare enum trimis prin rețea, ca să recunoaștem valorile greșite.
    inline constexpr auto kLastMessageType = MessageType::PlayerLeft;
    inline constexpr auto kLastEventType = EventType::RoundEnded;
    inline constexpr auto kLastGameState = Backend::EGameState::RoundOver;
    inline constexpr auto kLastRoundResult = Backend::ERoundResult::Blackjack;

    // Citește un enum trimis ca un octet. False dacă pachetul s-a terminat sau valoarea e
    // după `last`.
    template <typename Enum>
    bool readEnum(sf::Packet& packet, Enum& value, Enum last) {
        std::uint8_t raw = 0;
        if (!(packet >> raw) || raw > static_cast<std::uint8_t>(last)) {
            return false;
        }
        value = static_cast<Enum>(raw);
        return true;
    }

    void writeCard(sf::Packet& packet, Backend::Card card);
    // False dacă pachetul s-a terminat sau cartea e invalidă (rang sau culoare greșite).
    bool readCard(sf::Packet& packet, Backend::Card& card);
}  // namespace Frontend::Net
