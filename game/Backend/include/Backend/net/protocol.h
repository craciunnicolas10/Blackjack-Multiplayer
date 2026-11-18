#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <Backend/Card.h>
#include <Backend/GameEnums.h>
#include <Backend/NetworkEnums.h>

// Ce isi trimit calculatoarele in jocul in retea. Fiecare mesaj e un sf::Packet care incepe cu
// EMessageType. Nu face parte din API: Frontend-ul vede doar INetworkHost / INetworkClient.
namespace Backend {
    // Numarul de porturi incercate de gazda, incepand cu INetworkHost::kDefaultPort.
    inline constexpr unsigned short kPortCount = 10;
    // Se schimba cand se schimba mesajele, ca doua versiuni diferite sa nu joace impreuna.
    inline constexpr std::uint32_t kProtocolVersion = 2;

    enum class EMessageType : std::uint8_t {
        // --- jucator -> gazda ---
        Join,  // versiune, nume
        PlaceBet,  // suma
        Hit,
        Stand,
        DoubleDown,
        NextRound,
        // --- gazda -> jucator ---
        Rejected,      // EJoinRejection
        PlayerList,    // numele din lobby, in ordinea locurilor
        GameStarting,  // locul jucatorului, numele tuturor
        GameEvent,     // un anunt al jocului (EEventType + date) si starea de dupa el
        GameOver,      // nimeni nu mai are bani de pariu
        PlayerLeft,    // numele celui care a iesit; jocul se opreste
    };

    // Anunturile din IGameListener, unul la unul.
    enum class EEventType : std::uint8_t {
        StateChanged,
        PlayerAdded,
        BetPlaced,
        PlayerCardDealt,
        DealerCardDealt,
        DealerCardRevealed,
        TurnChanged,
        RoundEnded,
    };

    // Ultima valoare din fiecare enum trimis prin retea, ca sa recunoastem valorile gresite.
    inline constexpr auto kLastMessageType = EMessageType::PlayerLeft;
    inline constexpr auto kLastEventType = EEventType::RoundEnded;
    inline constexpr auto kLastGameState = EGameState::RoundOver;
    inline constexpr auto kLastRoundResult = ERoundResult::Blackjack;
    inline constexpr auto kLastJoinRejection = EJoinRejection::NameTaken;

    // Limite pentru ce primim din retea, ca un mesaj stricat sa nu ceara memorie la nesfarsit.
    inline constexpr std::uint8_t kMaxHandSize = 22;
    // Numele, in octeti UTF-8 (12 litere cu diacritice incap).
    inline constexpr std::size_t kMaxNameBytes = 48;

    sf::Packet MakeMessage(EMessageType type);
    // nullopt daca pachetul e gol sau tipul e necunoscut.
    std::optional<EMessageType> ReadMessageType(sf::Packet& packet);

    // Pe socket-uri non-blocking un pachet poate pleca pe bucati; asta il trimite pe tot.
    // False daca legatura s-a rupt sau celalalt nu mai primeste nimic de cateva secunde.
    bool SendAll(sf::TcpSocket& socket, sf::Packet& packet);

    // Citeste un enum trimis ca un octet. False daca pachetul s-a terminat sau valoarea e
    // dupa `last`.
    template <typename Enum>
    bool ReadEnum(sf::Packet& packet, Enum& value, Enum last) {
        std::uint8_t raw = 0;
        if (!(packet >> raw) || raw > static_cast<std::uint8_t>(last)) {
            return false;
        }
        value = static_cast<Enum>(raw);
        return true;
    }

    void WriteCard(sf::Packet& packet, Card card);
    // False daca pachetul s-a terminat sau cartea e invalida (rang sau culoare gresite).
    bool ReadCard(sf::Packet& packet, Card& card);

    // Un nume de jucator primit din retea: nu e gol si nu e prea lung.
    bool IsValidName(const std::string& name);

    // "192.168.1.5" sau "192.168.1.5:53001" -> adresa si portul (implicit kDefaultPort).
    // nullopt daca portul scris nu e un numar bun.
    std::optional<std::pair<std::string, unsigned short>> SplitAddress(const std::string& text);
}  // namespace Backend
