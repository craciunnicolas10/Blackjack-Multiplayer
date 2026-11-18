#pragma once
#include <platform.h>

#include <memory>
#include <string>

#include <Backend/INetworkSession.h>

namespace Backend {
    // Gazda unui joc in retea: tine jocul real si asteapta sa intre ceilalti (2-4 jucatori).
    //   Produce -> (ceilalti intra; OnPlayersChanged) -> StartGame -> OnGameStarted
    //   -> apelantul isi pune ascultatorii pe GetGame() si porneste prima runda cu
    //      GetGame()->StartRound().
    // Gazda verifica cine intra (versiune, loc la masa, nume) si, in timpul jocului, ca un
    // jucator actioneaza doar cand e randul lui; restul regulilor le verifica jocul.
    // BLACKJACK_API pe clasa, ca Produce() sa poata fi apelata din Frontend.
    class BLACKJACK_API INetworkHost : public INetworkSession {
       public:
        // Primul port incercat; daca e ocupat se iau urmatoarele, pana la kDefaultPort + 9.
        static constexpr unsigned short kDefaultPort = 53000;

        // Creeaza jocul si incepe sa astepte jucatori. nullptr daca numele nu e bun sau nu e
        // niciun port liber.
        static std::shared_ptr<INetworkHost> Produce(const std::string& hostName);

        // Ce scriu ceilalti ca sa intre: "192.168.1.5", sau "192.168.1.5:53001" cand portul nu
        // e kDefaultPort. Gol daca adresa calculatorului nu poate fi aflata.
        virtual std::string GetAddress() const = 0;
        // A intrat cel putin inca un jucator.
        virtual bool CanStartGame() const = 0;
        // Porneste jocul cu cei intrati si anunta OnGameStarted. Dupa asta nu mai intra nimeni.
        virtual bool StartGame() = 0;
    };
}  // namespace Backend
