#pragma once
#include <platform.h>

#include <memory>
#include <string>

#include <Backend/INetworkSession.h>
#include <Backend/NetworkEnums.h>

namespace Backend {
    // Un jucator conectat la gazda: primeste lista din lobby (OnPlayersChanged), apoi
    // OnGameStarted, dupa care joaca prin GetGame() ca in jocul local.
    // BLACKJACK_API pe clasa, ca Produce() sa poata fi apelata din Frontend.
    class BLACKJACK_API INetworkClient : public INetworkSession {
       public:
        // Se conecteaza la gazda si cere sa intre cu `name`. `address` e cum o arata gazda
        // ("192.168.1.5" sau "192.168.1.5:53001"). Asteapta cel mult cateva secunde.
        // nullptr daca nu s-a putut; motivul e in `result`.
        static std::shared_ptr<INetworkClient> Produce(const std::string& address,
                                                       const std::string& name,
                                                       EConnectResult& result);
    };
}  // namespace Backend
